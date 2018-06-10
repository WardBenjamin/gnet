/* Copyright © 2018, Mykos Hudson-Crisp <micklionheart@gmail.com>
 * All rights reserved. */

#include "UDP.h"
#include "Stopwatch.h"
#include "NumericTypes.h"
#include "MathPacker.h"
#include "Hash.h"
#include "Cipher.h"
#include "X25519.h"
#include <string>
#include <vector>
#include <algorithm>
#include <set>
#include <atomic>
#include <mutex>
#include <thread>
#include <map>
#include <sstream>
#include <stdlib.h>
#include <memory.h>
#ifndef _WIN32
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <linux/in6.h>
#include <netdb.h>
#endif

using namespace std;

const u64 UDPHandshakeChecksumDomain = 0x1EA0840BD5BC145Eull;
const u64 UDPHandshakeKDFDomain = 0xF9466961D19DF3EDull;
const u64 UDPHandshakeAuthDomain = 0x7A8A40694E96AFCBull;
const u64 UDPPacketKDFDomain = 0xF9466961D19DF3EDull;
const u64 UDPPacketAuthDomain = 0x7A8A40694E96AFCBull;

int mtuMinIndex = 4;
	bool ListenEnabled = true;
atomic<bool> done;
thread* udpThread = 0;

int MTUTable[] = {68, 296, 508, 1006, 1492, 2002, 4352, 8166, 17914, 32000, 65535};
int MTUOverhead = 28;

int MTUTableSize = sizeof(MTUTable) / sizeof(MTUTable[0]);
int Stencil[] = {
	1, 5, 13, 26, 42, 63, 88, 117, 150, 187, 228, 273, 323, 377, 434, 496, 562, 632,
	707, 785, 868, 954, 1045, 1140, 1239, 1342, 1450, 1561, 1677, 1796, 1920, 2047
};
int StencilSize = 32;
map<int, int> Sockets;
int Port = ServerPort;
u64 rand64() {return (u64(rand())<<32)^rand();}
set<UDPAddress> connectQueue;
vector<Ref<UDPNode>> peers;
set<Ref<UDPNode>> deleteQueue;
mutex peersMutex, connectQueueMutex;

string GetHostname(struct sockaddr* addr) {
	string str;
	switch (addr->sa_family) {
		case AF_INET: {
			struct sockaddr_in *addr4 = (struct sockaddr_in*)addr;
			str.resize(INET_ADDRSTRLEN);
			inet_ntop(AF_INET, &(addr4->sin_addr), &str[0], INET_ADDRSTRLEN);
			break;
		}
		case AF_INET6: {
			struct sockaddr_in6 *addr6 = (struct sockaddr_in6*)addr;
			str.resize(INET6_ADDRSTRLEN);
			inet_ntop(AF_INET6, &(addr6->sin6_addr), &str[0], INET6_ADDRSTRLEN);
			break;
		}
		default:
			break;
	}
	return str;
}

bool GetSockaddr(sockaddr_in &addr, const string address, unsigned short port) {
	// FIXME: use gethostinfo()
	
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);

	//*
	hostent *host = 0;
	if (!host) host = gethostbyname(address.c_str());
	if (!host) return false;
	addr.sin_addr.s_addr = *((unsigned long *) host->h_addr_list[0]);
	return true;
	/*/
	struct addrinfo hints, *servinfo, *p;
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	char stupid[32];
	memset(stupid, 0, sizeof(stupid));
	snprintf(stupid, sizeof(stupid) - 1, "%d", port);
	int rv = getaddrinfo(address.c_str(), stupid, &hints, &servinfo);
	if (rv) {
		wchar_t* reallyWindows = gai_strerror(rv);
		char wtf[512];
		memset(wtf, 0, sizeof(wtf));
		wcstombs(wtf, reallyWindows, sizeof(wtf)-1);
		fprintf(stderr, "getaddrinfo: %s\n", wtf);
		return false;
	}
	// ...
	return true;
	//*/
}
UDPAddress::UDPAddress(int socket) {
	Ok = false;
	Socket = socket;
	MTU = MTUTable[mtuMinIndex] - MTUOverhead;
	memset(&Address, 0, sizeof(Address));
	AddressLength = sizeof(Address);
}
UDPAddress::UDPAddress(string host, int defaultPort, int socket) {
	Ok = false;
	Socket = socket;
	MTU = MTUTable[mtuMinIndex] - MTUOverhead;
	int port = defaultPort;
	auto portIdx = host.find(':');
	if (portIdx != string::npos) {
		port = atoi(host.substr(portIdx+1).c_str());
		if (port) {
			host = host.substr(0, portIdx);
		} else {
			port = ServerPort;
		}
	}
	if (GetSockaddr(*(sockaddr_in*)&Address, host, port)) {
		AddressLength = sizeof(sockaddr_in);
		Ok = true;
	} else {
		memset(&Address, 0, sizeof(Address));
		AddressLength = sizeof(Address);
	}
}
size_t UDPAddress::Send(const vector<char>& buf) {	
	return Send(buf.data(), buf.size());
}
size_t UDPAddress::Send(const void* buf, size_t len) {	
	return sendto(Socket, (char*)buf, len, 0, (struct sockaddr*)&Address, AddressLength);
}
bool UDPAddress::operator!() const {
	return !Ok;
}
bool UDPAddress::operator!=(const UDPAddress& other) const {
	return !(*this == other);
}
bool UDPAddress::operator==(const UDPAddress& other) const {
	if (Socket != other.Socket) return false;
	if (AddressLength != other.AddressLength) return false;
	return memcmp(&Address, &other.Address, AddressLength) == 0;
}
bool UDPAddress::operator<(const UDPAddress& other) const {
	if (Socket != other.Socket) return Socket < other.Socket;
	if (AddressLength != other.AddressLength) return AddressLength < other.AddressLength;
	return memcmp(&Address, &other.Address, AddressLength) > 0;
}
string UDPAddress::Hostname() const {
	return GetHostname((sockaddr*)&Address);
}

UDPPacket::UDPPacket(unsigned mtu_) {
	mtu = mtu_;
	acked = false;
}

UDPPacket::UDPPacket() {
	mtu = 0;
	acked = false;
}

unsigned UDPPacket::Size() {
	return (unsigned)((bin.Bits() + 7) / 8);
}

void UDPPacket::Ack(bool ack) {
	//for(auto& c: chunks) c->Ack(user, ack);
}

UDPNode::UDPNode() {
	key.Random();
	seqOut = 0;
	inTimer = 0;
	latency = 1;
	mtuIndex = mtuMinIndex;
	outTimer = 0;
	outPeriod = 1;
	pingCounter = 0;
	historyIndex = 0;
	handshakeTag = 0;
	historyIndex = 0;
	historyOffset = 0;
	historyIndexIn = 0;
	timeoutCounter = 0;
	historyOffsetIn = 0;
	handshakeDone = false;
	priority = 0;
	autoConnect = false;
	trusted = false;
	memset(historyIn, 0, sizeof(historyIn));
	lastSeen = Stopwatch::GlobalTime();
}
UDPNode::UDPNode(const UDPNode& x) {
	seqOut = x.seqOut;
	historyIndex = x.historyIndex;
	outTimer = x.outTimer;
	inTimer = x.inTimer;
	outPeriod = x.outPeriod;
	timeoutCounter = x.timeoutCounter;
	pingCounter = x.pingCounter;
	key = x.key;
	handshakeTag = x.handshakeTag;
	handshakeDone = x.handshakeDone;
	historyIndex = x.historyIndex;
	historyOffset = x.historyOffset;
	historyIndexIn = x.historyIndexIn;
	historyOffsetIn = x.historyOffsetIn;
	memcpy(historyIn, x.historyIn, sizeof(historyIn));
	lastSeen = x.lastSeen;
	latency = x.latency;
	ip = x.ip;
	mtuIndex = x.mtuIndex;
	priority = x.priority;
	autoConnect = x.autoConnect;
	trusted = x.trusted;
	for(size_t i=0;i<HistorySize;i++) {
		history[i] = x.history[i];
	}
}
UDPNode::~UDPNode() {
	memset(&key, 0, sizeof(key));
}
UDPNode::UDPNode(string host) {
	key.Random();
	seqOut = 0;
	inTimer = 0;
	latency = 1;
	mtuIndex = mtuMinIndex;
	outTimer = 0;
	outPeriod = 1;
	pingCounter = 0;
	historyIndex = 0;
	handshakeTag = 0;
	historyIndex = 0;
	historyOffset = 0;
	historyIndexIn = 0;
	timeoutCounter = 0;
	historyOffsetIn = 0;
	handshakeDone = false;
	priority = 0;
	autoConnect = false;
	trusted = false;
	memset(historyIn, 0, sizeof(historyIn));
	lastSeen = Stopwatch::GlobalTime();
	AddHost(0, host);
}
void UDPNode::Seen() {
	double t = Stopwatch::GlobalTime();
	latency = (latency * 0.1f + t - lastSeen) / 1.1f;
	lastSeen = t;
}
double UDPNode::Latency() {
	double t = Stopwatch::GlobalTime();
	t -= lastSeen;
	if (t < latency) {
		t = latency;
	}
	return t;
}
bool UDPNode::AddHost(int socket, string host, int defaultPort) {
	UDPAddress addy(host, defaultPort, socket);
	if (!addy) return false;
	ip = addy;
	return true;
}
void UDPNode::SendHandshake() {
	X25519 ecdhe;
	ecdhe *= key;
	vector<char> handshake;
	handshake.resize(40);
	memcpy(&handshake[0], &ecdhe, 32);
	Hash(&handshake[32], 8, &handshake[0], 32, 0, 0, UDPHandshakeChecksumDomain);
	memcpy(&handshakeTag, &handshake[32], 4);
	ip.Send(handshake);
}
void UDPNode::SendHandshakeResponse(const void* buffer) {
	X25519 ecdhe, kdf;
	ecdhe *= key;
	memcpy(&kdf, buffer, 32);
	kdf *= key;
	Hash(key, 32, &kdf, 32, 0, 0, UDPHandshakeKDFDomain);
	handshakeDone = true;
	timeoutCounter = 0;
	pingCounter = 0;

	char reply[36];
	memcpy(reply, buffer, 36);
	memcpy(reply, &ecdhe, 32);
	ip.Send(reply, 36);

	//printf("ECDHE > %s\n", string(key).c_str());
}
void UDPNode::RecvHandshakeResponse(const void* buffer) {
	X25519 kdf;
	memcpy(&kdf, buffer, 32);
	kdf *= key;
	Hash(key, 32, &kdf, 32, 0, 0, UDPHandshakeKDFDomain);
	handshakeDone = true;
	timeoutCounter = 0;
	pingCounter = 0;

	//printf("ECDHE < %s\n", string(key).c_str());
}

#pragma pack(push, 1)
struct PacketHeader {
	u8 msgId;
	u8 ackId;
	u32 ackFlags;
	u8 mac[10];
};
struct FullPacketHeader {
	u64 msgId;
	u64 ackId;
	u32 ackFlags;
};
#pragma pack(pop)

size_t UDPNode::SealPacket(u8* raw, size_t len) {
	FullPacketHeader fullHead;
	memset(&fullHead, 0, sizeof(fullHead));
	fullHead.msgId = seqOut++;
	fullHead.ackId = historyIndexIn;
	for(int i=0;i<32;i++) {
		u64 idx = historyIndexIn - Stencil[i];
		if (idx >= historyOffsetIn + HistorySize) continue;
		if (idx < historyOffsetIn) continue;
		u32 bit = historyIn[idx - historyOffsetIn];
		fullHead.ackFlags |= bit << i;
	}
	
	//printf("out msgId=%llu, ackId=%llu\n", fullHead.msgId, fullHead.ackId);
	PacketHeader head;
	memset(&head, 0, sizeof(head));
	head.msgId = (u8)fullHead.msgId;
	head.ackId = (u8)fullHead.ackId;
	head.ackFlags = fullHead.ackFlags;

	Binary256 k, c;
	Hash(k, sizeof(k), &fullHead, sizeof(fullHead), key, sizeof(key), UDPPacketKDFDomain);
	StreamCipher(raw, len, k, sizeof(k));
	Hash(c, sizeof(c), raw, len, key, sizeof(key), UDPPacketAuthDomain);
	memcpy(head.mac, k, sizeof(head.mac));
	BlockCipher(&head, sizeof(PacketHeader), c, sizeof(c));
	memcpy(raw+len, &head, sizeof(PacketHeader));
	return len + sizeof(PacketHeader);
}

size_t UDPNode::UnsealPacket(u8* raw, size_t len) {
	if (len <= sizeof(PacketHeader)) return 0;
		
	Binary256 k, c;
	PacketHeader head;
	memcpy(&head, raw + len - sizeof(PacketHeader), sizeof(PacketHeader));
	Hash(c, sizeof(c), raw, len - sizeof(PacketHeader), key, sizeof(key), UDPPacketAuthDomain);
	BlockDecipher(&head, sizeof(PacketHeader), c, sizeof(c));

	FullPacketHeader fullHead;
	memset(&fullHead, 0, sizeof(fullHead));
	fullHead.msgId = seqDec.Decode(head.msgId);
	fullHead.ackId = ackDec.Decode(head.ackId);
	fullHead.ackFlags = head.ackFlags;
	//printf("in  msgId=%llu, ackId=%llu\n", fullHead.msgId, fullHead.ackId);

	Hash(k, sizeof(k), &fullHead, sizeof(fullHead), key, sizeof(key), UDPPacketKDFDomain);
	u8 problem = 0;
	for(size_t i=0;i<sizeof(head.mac);i++) problem |= head.mac[i] ^ k.Byte[i];
	if (problem) return 0;
	
	memcpy(raw + len - sizeof(PacketHeader), &head, sizeof(PacketHeader));
	StreamCipher(raw, len, k, sizeof(k));
	seqDec.Confirm(fullHead.msgId);
	ackDec.Confirm(fullHead.ackId);
	history[fullHead.ackId].Ack(true);
	for(int i=0;i<32;i++) {
		bool ack = fullHead.ackFlags & 1;
		fullHead.ackFlags >>= 1;
		unsigned idx = Stencil[i];
		if (idx > fullHead.ackId) break;
		history[fullHead.ackId-idx].Ack(ack);
	}
		
	while (mtuIndex+1 < MTUTableSize) {
		unsigned mtu = MTUTable[mtuIndex+1] - MTUOverhead;
		if (mtu > len) break;
		printf("MTU Upgrade: %d\n", mtu);
		ip.MTU = mtu;
	}

	return len - sizeof(PacketHeader);
}

void UDPNode::SendFrames(float dt) {
	u8 raw[65536];
	outTimer += dt;
	timeoutCounter += dt;
	pingCounter += dt;
	UDPPacket buf(ip.MTU);
	if (pingCounter >= pingPeriod) {
		// FIXME: send keep alive
		pingCounter = 0;
	}
	if (timeoutCounter >= timeoutPeriod) {
		deleteQueue.insert(this);
		timeoutCounter = 0;
		return;
	}
	if (outTimer >= outPeriod) {
		outTimer -= outPeriod;
		if (true) {
			lock_guard<mutex> L(outboxMutex);
			// FIXME: send ip.MTU bytes from outbox
		}
		int bytes = buf.Size();
		if (bytes) {
			buf.bin.Export(raw, bytes);
			bytes = SealPacket(raw, bytes);
			if (bytes) {
				lock_guard<mutex> L(historyMutex);
				history[historyIndex++] = buf;
				historyIndex %= HistorySize;
				ip.Send(raw, bytes);
			}
		}
	}
	return;
}

void ProcessPacket(u8* buffer, size_t length, UDPAddress hint) {
	if (length == 36) {
		u32 tag;
		memcpy(&tag, buffer + 32, 4);
		for(auto p: peers) {
			if (tag == p->handshakeTag) {
				p->RecvHandshakeResponse(buffer);
				p->Seen();
				
				printf("Connected => %s\n", hint.Hostname().c_str());
				return;
			}
		}
	} else if (length == 40) {
		u64 tag, checksum;
		memcpy(&tag, buffer + 32, 8);
		Hash(&checksum, 8, buffer, 32, 0, 0, UDPHandshakeChecksumDomain);
		if (tag == checksum) {
			Ref<UDPNode> node = new UDPNode();
			node->ip = hint;
			node->ip.Socket = Sockets[((sockaddr*)&hint.Address)->sa_family];
			node->SendHandshakeResponse(buffer);
			node->Seen();
			
			peersMutex.lock();
			peers.push_back(node);
			peersMutex.unlock();
			
			printf("Connected <= %s\n", hint.Hostname().c_str());
			return;
		}
	}
	size_t len = 0;
	Ref<UDPNode> peer;
	for(auto p: peers) {
		if (p->ip != hint) continue;
		len = p->UnsealPacket(buffer, length);
		if (len) {
			peer = p;
			break;
		}
	}
	if (!len) {
		for(auto p: peers) {
			len = p->UnsealPacket(buffer, length);
			if (len) {
				peer = p;
				break;
			}
		}
	}
	if (len) {
		peer->lastSeen = Stopwatch::GlobalTime();
		// FIXME parse packed message
		//MathUnpacker unpacker;
		//unpacker.Import(buffer, len * 8);
	} else {
		printf("Corrupt or invalid packet (%lu bytes)\n", length);
	}
}

void UDPWorker() {
	struct addrinfo hint;
	memset(&hint, 0, sizeof(hint));
	hint.ai_family = AF_UNSPEC;
	hint.ai_socktype = SOCK_DGRAM;
	hint.ai_flags = AI_PASSIVE;
	struct addrinfo* list = 0;
	stringstream portStr;
	portStr << Port;
	if (getaddrinfo(0, portStr.str().c_str(), &hint, &list)) {
		perror("getaddrinfo");
		return;
	}
	bool ok = false;
	while (list) {
		struct addrinfo* i = list;
		list = list->ai_next;
		int sock = socket(i->ai_family, i->ai_socktype, i->ai_protocol);
		if (sock < 0) {
			perror("socket");
			continue;
		}
#ifndef _WIN32
		int reuse = 1, v6only = 1, dontfrag = 1, pmtud = IP_PMTUDISC_DO;
		setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));
		setsockopt(sock, IPPROTO_IP, IP_MTU_DISCOVER, &pmtud, sizeof(pmtud));
		if (i->ai_family == AF_INET6) {
			setsockopt(sock, IPPROTO_IPV6, IPV6_V6ONLY, &v6only, sizeof(v6only));
			setsockopt(sock, IPPROTO_IPV6, IPV6_DONTFRAG, &dontfrag, sizeof(dontfrag));
		}
#endif
		if (::bind(sock, i->ai_addr, i->ai_addrlen) < 0) {
			perror("bind");
			closesocket(sock);
			continue;
		}
		Sockets[i->ai_family] = sock;
		switch (i->ai_family) {
			case AF_INET:
				printf("[UDP] IPv4 Enabled\n");
				break;
			case AF_INET6:
				printf("[UDP] IPv6 Enabled\n");
				break;
			default:
				printf("[UDP] Listen Enabled (%s)\n", GetHostname(i->ai_addr).c_str());
				break;
		}
		ok = true;
	}
	if (!ok) {
		perror("unknown error");
		return;
	}
	Stopwatch sendTimer;
	vector<unsigned char> buf;
	buf.resize(65536);

	while (!done && ListenEnabled) {
		peersMutex.lock();
		for(size_t c=peers.size();c--;) {
			if (deleteQueue.count(peers[c])) {
				peers.erase(peers.begin() + c);
			}
		}
		deleteQueue.clear();
		peersMutex.unlock();

		if (true) {
			connectQueueMutex.lock();
			if (connectQueue.size()) {
				vector<Ref<UDPNode>> newPeers;
				for(auto& addy: connectQueue) {
					printf("Connecting: %s...\n", addy.Hostname().c_str());
					Ref<UDPNode> node = new UDPNode();
					node->ip = addy;
					node->ip.Socket = Sockets[((sockaddr*)&node->ip.Address)->sa_family];
					node->SendHandshake();
					newPeers.push_back(node);
				}
				peersMutex.lock();
				for(auto p: newPeers) {
					peers.push_back(p);
				}
				peersMutex.unlock();
				connectQueue.clear();
			}
			connectQueueMutex.unlock();
		}

		fd_set selectList;
		FD_ZERO(&selectList);
		int selectMax = 0;
		for(auto& s: Sockets) {
			if (s.second > selectMax) selectMax = s.second;
			FD_SET(s.second, &selectList);
		}
		selectMax++;
		struct timeval timeout;
		memset(&timeout, 0, sizeof(timeout));
		timeout.tv_usec = 8500;
		int selectStatus = select(selectMax, &selectList, 0, 0, &timeout);
		if (selectStatus == -1) {
			perror("select");
			break;
		}
		if (selectStatus) {
			for(auto& s: Sockets) {
				if (FD_ISSET(s.second, &selectList)) {
					UDPAddress hint(s.second);
					memset(buf.data(), 0, buf.size());
					int bytes = recvfrom(s.second,
						(char*)buf.data(), buf.size()-1, 0,
						(sockaddr*)&hint.Address, &hint.AddressLength);
					if (bytes < 0) {
						perror("recvfrom");
						continue;
					}
					ProcessPacket(buf.data(), bytes, hint);
				}
			}
		}
		float dt = (float)sendTimer.Lap();
		lock_guard<mutex> peersLock(peersMutex);
		for(auto& p: peers) p->SendFrames(dt);
	}
	for(auto& s: Sockets) closesocket(s.second);
	Sockets.clear();
	return;
}

void UDP::Start() {
#ifdef _WIN32
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(1, 1), &wsaData) != 0) return;
#endif
	done = true;
	if (udpThread) {
		udpThread->join();
		delete udpThread;
	}
	done = false;
	udpThread = new thread(UDPWorker);
}
	
void UDP::Stop() {
	done = true;
	if (udpThread) {
		udpThread->join();
		delete udpThread;
		udpThread = 0;
	}
}

bool UDP::Connect(string host) {
	UDPAddress addy(host, ServerPort);
	if (!addy) return false;
	connectQueueMutex.lock();
	connectQueue.insert(addy);
	connectQueueMutex.unlock();
	return true;
}

int UDP::NumPeers() {
	lock_guard<mutex> L(peersMutex);
	return peers.size();
}

vector<Ref<UDPNode>> UDP::GetPeers() {
	lock_guard<mutex> L(peersMutex);
	return peers;
}
