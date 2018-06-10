/* Copyright © 2018, Mykos Hudson-Crisp <micklionheart@gmail.com>
 * All rights reserved. */

#include <stdio.h>
#include "Hash.h"
extern "C" {
	#include "blake2b.h"
};

void Hash(void* out, u64 outlen, const void* data, u64 len, const void* key, u64 keylen, u64 domain, u64 counter) {
	u64 seed[4];
	u64 i=0;
	u64 pad[8];
	u64* buf = (u64*)out;
	u64 ctr[] = {counter, domain};
	blake2b_ctx ctx;
	if (blake2b_init(&ctx, (size_t)sizeof(seed), key, (size_t)keylen)) printf("*** FECQ *** blake2b_init: rut roe\n");
	blake2b_update(&ctx, &domain, sizeof(domain));
	blake2b_update(&ctx, data, (size_t)len);
	blake2b_final(&ctx, seed);
	for(;!ctr[0] || outlen>=8;ctr[0]++) {
		blake2b(pad, 64, ctr, sizeof(ctr), seed, sizeof(seed));
		u64 j = outlen/8;
		if (j > 8) j = 8;
		for(i=0;i<j;i++) *buf++ = pad[i];
		outlen -= 8 * j;
	}
	u8* buf8 = (u8*)buf;
	u8* pad8 = (u8*)&pad[i];
	for(i=0;i<outlen;i++) *buf8++ = *pad8++;
}

