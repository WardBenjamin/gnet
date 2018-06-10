/* Copyright © 2018, Mykos Hudson-Crisp <micklionheart@gmail.com>
 * All rights reserved. */
#pragma once

#ifdef _WIN32
#define _WINSOCK_DEPCRECATED
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "Ws2_32.lib")
char *inet_ntop(int af, const void *src, char *dst, socklen_t size);
#endif

#include <string>
#include <vector>
#include <mutex>
#include <set>
#include "Resource.h"
#include "SequenceDecoder.h"
#include "MathPacker.h"

#ifndef _WIN32
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <linux/in6.h>
#include <netdb.h>
#endif

const int ServerPort = 28400;
extern int ClientPort;
const double timeoutPeriod = 120.0;
const double pingPeriod = 15.0;

struct UDPAddress {
	struct sockaddr_storage Address;
	socklen_t AddressLength;
	int Socket;
	bool Ok;
	size_t MTU;

	UDPAddress(int socket = 0);
	UDPAddress(std::string host, int defaultPort = 0, int socket = 0);
	size_t Send(const std::vector<char>& buf);
	size_t Send(const void* buf, size_t len);
	bool operator!() const;
	bool operator==(const UDPAddress& other) const;
	bool operator!=(const UDPAddress& other) const;
	bool operator<(const UDPAddress& other) const;
	std::string Hostname() const;
};

struct UDPPacket {
	bool acked;
	MathPacker bin;
	unsigned mtu;

	UDPPacket();
	UDPPacket(unsigned mtu);
	unsigned Size();
	void Ack(bool ack = true);
};

struct UDPNode : public RefCount {
	const static int HistorySize = 256;

	UDPAddress ip;
	double outTimer, inTimer, outPeriod, timeoutCounter, pingCounter;
	u64 historyIndex, historyOffset;
	UDPPacket history[HistorySize];
	u64 historyIndexIn, historyOffsetIn;
	bool historyIn[HistorySize];
	std::mutex outboxMutex, historyMutex;
	Binary256 key;
	bool handshakeDone;
	u32 handshakeTag;
	double lastSeen, latency;
	SequenceDecoder<8> seqDec, ackDec;
	u64 seqOut;
	int mtuIndex;
	int priority;
	bool autoConnect;
	bool trusted;

	UDPNode();
	UDPNode(const UDPNode& x);
	~UDPNode();
	UDPNode(std::string host);
	void Seen();
	double Latency();
	bool AddHost(int socket, std::string host, int defaultPort = ServerPort);
	void SendHandshake();
	void SendHandshakeResponse(const void*);
	void RecvHandshakeResponse(const void*);
	void SendFrames(float dt);
	size_t SealPacket(u8* buf, size_t len);
	size_t UnsealPacket(u8* buf, size_t len);
};

struct UDP {
	static void Start();
	static void Stop();
	static bool Connect(std::string);
	static std::vector<Ref<UDPNode>> GetPeers();
	static int NumPeers();
};
