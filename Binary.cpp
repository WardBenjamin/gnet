/* Copyright © 2018, Mykos Hudson-Crisp <micklionheart@gmail.com>
 * All rights reserved. */

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <wincrypt.h>
#endif

#include "Binary.h"
#include <sstream>
#include <time.h>

using namespace std;


void RandomBytes(void* out, size_t length) {
#ifdef _WIN32
	HCRYPTPROV l_prov;
	if (CryptAcquireContext(&l_prov, NULL, NULL, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT)) {
		bool ok = CryptGenRandom(l_prov, length, (BYTE*)out);
		CryptReleaseContext(l_prov, 0);
		if (ok) return;
	}
#else
	FILE* fp = fopen("/dev/urandom", "rb");
	if (fp) {
		size_t r = fread(out, 1, length, fp);
		fclose(fp);
		if (r >= length) return;
	}
#endif
	printf("Warning: Using rand()\n");
	srand(rand() ^ time(0));
	for(size_t i=0;i<length;i++) {
		((char*)out)[i] ^= rand();
	}
}

bool Str2Bool(string x) {
	if (!x.size()) return false;
	if (x == "false") return false;
	if (x == "0") return false;
	return true;
}

string Int2Str(signed long long value) {
	stringstream ss;
	ss << value;
	return ss.str();
}

string Bin2Hex(const void* data, size_t len) {
	string out;
	for(size_t i=0;i<len;i++) {
		char buf[3];
		memset(buf, 0, sizeof(buf));
		snprintf(buf, 3, "%02X", ((unsigned char*)data)[i]);
		out += buf;
	}
	return out;
}

std::vector<char> Hex2Bin(std::string hex) {
	std::vector<char> out;
	const char* nibs = hex.c_str();
	while (*nibs) {
		if (!(((nibs[0] >= '0') && (nibs[0] <= '9')) || ((nibs[0] >= 'A') && (nibs[0] <= 'F')))) break;
		if (!(((nibs[1] >= '0') && (nibs[1] <= '9')) || ((nibs[1] >= 'A') && (nibs[1] <= 'F')))) break;
		int nib0 = nibs[0];
		nib0 -= (nib0 > '9') ? ('A' - 10) : '0';
		int nib1 = nibs[1];
		nib1 -= (nib1 > '9') ? ('A' - 10) : '0';
		out.push_back(nib0*16 + nib1);
		nibs += 2;
	}
	return out;
}

