/* Copyright © 2018, Mykos Hudson-Crisp <micklionheart@gmail.com>
* All rights reserved. */

#include "Cipher.h"
extern "C" {
	#include "blake2b.h"
};

void StreamCipher(void* data, u64 length, const void* key, u64 keylength) {
	unsigned i=0;
	u64 pad[8];
	u64* buf = (u64*)data;
	for(u64 ctr=0;!ctr || length>=8;ctr++) {
		blake2b(pad, 64, &ctr, 8, key, keylength);
		u64 j = length / 8;
		if (j > 8) j = 8;
		for(i=0;i<j;i++) *buf++ ^= pad[i];
		length -= 8 * j;
	}
	u8* buf8 = (u8*)buf;
	u8* pad8 = (u8*)&pad[i];
	for(i=0;i<length;i++) *buf8++ ^= *pad8++;
}

void BlockCipher(void* data_, u64 length, const void* key, u64 keylength) {
	u8* data = (u8*)data_;
	u64 rounds = length*4 + 1;
	for(u64 i=0;i<rounds;i++) {
		u64 index = i % length;
		u8 tmp = data[index];
		data[index] = i;
		blake2b(&data[index], 1, key, keylength, data, length);
		data[index] ^= tmp;
	}
}

void BlockDecipher(void* data_, u64 length, const void* key, u64 keylength) {
	u8* data = (u8*)data_;
	u64 rounds = length*4 + 1;
	for(u64 i=rounds;i--;) {
		int index = (int)(i % length);
		u8 tmp = data[index];
		data[index] = i;
		blake2b(&data[index], 1, key, keylength, data, length);
		data[index] ^= tmp;
	}
}

