/* Copyright © 2018, Mykos Hudson-Crisp <micklionheart@gmail.com>
* All rights reserved. */
#pragma once

#include "NumericTypes.h"
#include <memory.h>

extern "C" {
	int curve25519_donna(u8 *mypublic, const u8 *secret, const u8 *basepoint);
};

#pragma pack(push, 1)
struct X25519 : public Binary256 {
	X25519() {
		Clear();
	}
	X25519(const void* other) {
		memcpy(Byte, other, sizeof(Byte));
	}
	void Clear() {
		memset(this, 0, sizeof(Binary256));
		Byte[0] = 9;
	}
	void operator=(const void* other) {
		memcpy(Byte, other, sizeof(Byte));
	}
	void operator*=(const Binary256& other) {
		curve25519_donna(Byte, other.Byte, Byte);
	}
};
#pragma pack(pop)
