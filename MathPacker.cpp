/* Copyright © 2018, Mykos Hudson-Crisp <micklionheart@gmail.com>
* All rights reserved. */

#include "MathPacker.h"
#include <memory.h>
#include <ctype.h>

using namespace std;

#include <intrin.h>
int __inline __builtin_popcntll(u64 X) {
	u32 word1 = (u32)X;
	u32 word2 = (u32)(X >> 32ull);
	return __popcnt(word1) + __popcnt(word2);
}

int __inline __builtin_clzll(u64 X) {
	X |= X >> 1ull;
	X |= X >> 2ull;
	X |= X >> 4ull;
	X |= X >> 8ull;
	X |= X >> 16ull;
	X |= X >> 32ull;
	return __builtin_popcntll(~X);
}

MathPacker::MathPacker() {
	Scale = 1;
}
MathUnpacker::MathUnpacker() {
	Scale = 1;
}
MathUnpacker::MathUnpacker(const std::vector<u8>& buffer, u64 bits) {
	Scale = 1;
	if (!bits) bits = buffer.size() * 8;
	Import(&buffer[0], bits);
}
MathPacker::MathPacker(const MathPacker& other) {
	Buffer = other.Buffer;
	Scale = other.Scale;
}
void MathPacker::Write(s64 value, s64 min, s64 max) {
	Write(value - min, max - min + 1);
}
s64 MathUnpacker::Read(s64 min, s64 max) {
	return Read(max - min + 1) + min;
}
void MathPacker::Export(u8* buffer, size_t length) {
	size_t bits = (size_t)Bits();
	if (!bits) return;
	size_t bytes = (bits + 7) / 8;
	u8* src = (u8*)&Buffer[0];
	if (!length) {
		memcpy(buffer, src, bytes-1);
		buffer[bytes-1] &= 255 << (8 - (bits % 8));
		buffer[bytes-1] ^= src[bytes-1];
	} else {
		if (length > bytes) length = bytes;
		memcpy(buffer, src, length);
	}

	MathUnpacker test;
	test.Import(buffer, length * 8);
	for (int i = 0; i < Values.size(); i++) {
		u64 x = test.Read(Moduli[i]);
		if (x != Values[i]) {
			printf("Unpack Fail66: test%%%d = %d (!= %d) buffer.size()=%d\n", Moduli[i], x, Values[i], test.Buffer.size()); 
		}
	}
}
std::vector<u8> MathPacker::Export() {
	std::vector<u8> out;
	size_t bits = (size_t)Bits();
	if (!bits) return out;
	size_t bytes = (bits + 7) / 8;
	out.resize(bytes);
	memcpy(&out[0], &Buffer[0], bytes);

	MathUnpacker test;
	test.Import(out);
	for (int i = 0; i < Values.size(); i++) {
		u64 x = test.Read(Moduli[i]);
		if (x != Values[i]) {
			printf("Unpack Fail83: test%%%d = %d (!= %d) buffer.size()=%d\n", Moduli[i], x, Values[i], test.Buffer.size());
		}
	}

	return out;
}
void MathUnpacker::Import(const u8* buffer, u64 bits) {
	u64 words = (bits + 63) / 64;
	Buffer.resize((size_t)words);
	u8* dst = (u8*)&Buffer[0];
	memset(dst, 0, (size_t)(words * sizeof(u64)));
	memcpy(dst, buffer, (size_t)(bits/8));
	if (bits % 8) {
		u8 mask = 255 >> (8 - (bits%8));
		dst[bits/8] = buffer[bits/8] & mask;
	}
}
void MathUnpacker::Import(const std::vector<u8>& buffer) {
	Import(&buffer[0], buffer.size() * 8);
}
void MathPacker::Write(u64 value, u64 modulus) {
	value++;
	modulus++;

	if (value >= modulus) __debugbreak();
	if (!Buffer.size()) Buffer.push_back(0);
	u64 term = Scale;
	Scale *= modulus;
	if (!Scale) {
		Buffer[Buffer.size()-1] += term * value;
		Buffer.push_back(0);
		Scale = 1;
	} else if (Scale / modulus == term) {
		Buffer[Buffer.size()-1] += term * value;
	} else {
		Buffer.push_back(value);
		Scale = modulus;
	}
	Values.push_back(value);
	Moduli.push_back(modulus);
}
u64 MathUnpacker::Read(u64 modulus) {
	modulus++;

	if (!Buffer.size()) return 0;
	u64 lastScale = Scale;
	Scale *= modulus;
	if (!Scale) {
		u64 out = Buffer[0] % modulus;
		Buffer.erase(Buffer.begin());
		Scale = 1;
		return out - 1;
	}
	if (Scale / modulus != lastScale) {
		Buffer.erase(Buffer.begin());
		Scale = modulus;
	}
	u64 out = Buffer[0] % modulus;
	Buffer[0] /= modulus;
	return out - 1;
}
u64 MathPacker::Bits() {
	if (!Buffer.size()) return 0;
	u64 size = Buffer.size()*64 - __builtin_clzll(Scale);
	if (__builtin_popcntll(Scale) == 1) size--;
	return size;
}
u64 MathPacker::FitModulus(u64 target) {
	u64 mod = 0xFFFFFFFFFFFFFFFFull / Scale;
	if (__builtin_popcntll(Scale) == 1) mod++;
	if (mod <= 1) return target;
	if (target && mod > target) mod = target;
	return mod;
}
u64 MathUnpacker::FitModulus(u64 target) {
	u64 mod = 0xFFFFFFFFFFFFFFFFull / Scale;
	if (__builtin_popcntll(Scale) == 1) mod++;
	if (mod <= 1) return target;
	if (target && mod > target) mod = target;
	return mod;
}

string ParseStaticCode(MathUnpacker& data) {
	string out;
	for(;;) {
		char sym = data.Read(37);
		if (!sym) break;
		if (sym > 10) {
			sym += 'a' - 11;
		} else {
			sym += '0' - 1;
		}
		out += sym;
	}
	return out;
}

void BufferStaticCode(vector<MessageDataPoint>& buf, string str) {
	MessageDataPoint cursor;
	cursor.modulus = 37;
	for (auto c : str) {
		char x = tolower(c);
		int sym = 0;
		if (x >= 'a' && x <= 'z') {
			sym = (x - 'a') + 11;
		} else if (x >= '0' && x <= '9') {
			sym = (x - '0') + 1;
		}
		if (sym) {
			cursor.value = sym;
			buf.push_back(cursor);
		}
	}
	cursor.value = 0;
	buf.push_back(cursor);
}
