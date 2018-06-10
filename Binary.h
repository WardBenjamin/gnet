/* Copyright © 2018, Mykos Hudson-Crisp <micklionheart@gmail.com>
 * All rights reserved. */
#pragma once

#include <string>
#include <vector>
#include <string.h>

#ifdef USE_GMP
#include <stddef.h>
#include <gmpxx.h>
mpz_class bin2mpz(const void* data, size_t bytes);
template<class T> mpz_class bin2mpz(const T& data) {return bin2mpz(data, sizeof(data));}
void mpz2bin(void* data, size_t bytes, mpz_class X);
std::vector<char> mpz2bin(mpz_class X);
#endif

std::string Bin2Hex(const void* data, size_t len);
std::vector<char> Hex2Bin(std::string hex);
std::string Int2Str(signed long long value);
bool Str2Bool(std::string x);
void RandomBytes(void* out, size_t length);

#pragma pack(push, 1)
template <int N> struct Binary {
	static Binary<N> FromHex(std::string str) {
		Binary<N> out;
		auto data = Hex2Bin(str);
		if (data.size() > sizeof(out)) data.resize(sizeof(out));
		memcpy(out, data.data(), data.size());
		return out;
	}
	Binary() {memset(Byte, 0, sizeof(Byte));}
	Binary(const void* data) {memcpy(Byte, data, sizeof(Byte));}
	unsigned char& operator[](int i) {return Byte[i];}
	operator void*() {return Byte;}
	operator const void*() const {return Byte;}
	operator std::string() const {return Bin2Hex(Byte, sizeof(Byte));}
	void operator=(const Binary<N>& other) {
		for(unsigned i=0;i<sizeof(Byte);i++) {
			Byte[i] = other.Byte[i];
		}
	}
	void operator^=(const Binary<N>& other) {
		for(unsigned i=0;i<sizeof(Byte);i++) {
			Byte[i] ^= other.Byte[i];
		}
	}
	void operator|=(const Binary<N>& other) {
		for(unsigned i=0;i<sizeof(Byte);i++) {
			Byte[i] |= other.Byte[i];
		}
	}
	void operator&=(const Binary<N>& other) {
		for(unsigned i=0;i<sizeof(Byte);i++) {
			Byte[i] &= other.Byte[i];
		}
	}
	bool Add(const Binary<N>& other) {
		unsigned short carry = 0;
		for(unsigned i=sizeof(Byte);i--;) {
			carry += Byte[i] + other.Byte[i];
			Byte[i] = carry;
			carry >>= 8;
		}
		return carry;
	}
	bool Sub(const Binary<N>& other) {
		bool carry = false;
		for(unsigned i=sizeof(Byte);i--;) {
			unsigned short temp = other.Byte[i] + carry;
			carry = temp > Byte[i];
			Byte[i] -= temp;
		}
		return carry;
	}
	void ModAdd(const Binary<N>& other, const Binary<N>& mod) {
		if (Add(other)) Sub(mod);
	}
	void ModSub(const Binary<N>& other, const Binary<N>& mod) {
		if (Sub(other)) Add(mod);
	}
	template<int M>
	void ModExp(const Binary<M>& other, const Binary<N>& mod) {
		auto g = *this;
		memset(Byte, 0, sizeof(Byte));
		Byte[sizeof(Byte) - 1] = 1;
		for(int i=0;i<M;i++) {
			if (other.GetBit(i)) {
				ModMul(g, mod);
			}
			g.ModMul(g, mod);
		}
		if (!((*this) < mod)) Sub(mod);
	}
	bool GetBit(int i) const {
		if (i < 0 || i >= N) return false;
		return (Byte[(N-i-1)/8] & (1 << (i%8)));
	}
#ifdef USE_GMP
	void Mul(const Binary<N>& other) {
		auto x = bin2mpz(Byte);
		x *= bin2mpz(other.Byte);
		mpz2bin(Byte, sizeof(Byte), x);
	}
	void ModMul(const Binary<N>& other, const Binary<N>& mod) {
		auto x = bin2mpz(Byte);
		x *= bin2mpz(other.Byte);
		x %= bin2mpz(mod.Byte);
		mpz2bin(Byte, sizeof(Byte), x);
	}
	void ModInv(const Binary<N>& mod) {
		auto x = bin2mpz(Byte);
		auto m = bin2mpz(mod.Byte);
		mpz_invert(x.get_mpz_t(), x.get_mpz_t(), m.get_mpz_t());
		mpz2bin(Byte, sizeof(Byte), x);
	}
#else
	void Mul(const Binary<N>& other) {
		auto g = *this;
		memset(Byte, 0, sizeof(Byte));
		for(int i=0;i<N;i++) {
			if (other.GetBit(i)) {
				Add(g);
			}
			g.ShiftLeft();
		}
	}
	void ModMul(const Binary<N>& other, const Binary<N>& mod) {
		Binary<N> g = *this;
		Binary<N> xx;
		for(int i=0;i<N;i++) {
			if (other.GetBit(i)) {
				xx.ModAdd(g, mod);
			}
			if (g.ShiftLeft()) g.Sub(mod);
		}
		*this = xx;
	}
	void ModInv(const Binary<N>& mod) {
		auto expo = mod;
		Binary<N> two;
		two.Byte[sizeof(two.Byte)-1]=2;
		expo.Sub(two);
		auto x = *this;
		x.ModExp(expo, mod);
		*this = x;
	}
#endif
	bool ShiftLeft() {
		bool out = Byte[0] >> 7;
		for(int i=0;i<N/8;i++) {
			Byte[i] <<= 1;
			if (i >= N/8-1) break;
			Byte[i] ^= (Byte[i+1] >> 7);
		}
		return out;
	}
	bool operator<(const Binary<N>& other) const {
		return memcmp(this, &other, sizeof(Byte)) < 0;
	}
	bool operator==(const Binary<N>& other) const {
		int sum = 0;
		for(unsigned i=0;i<sizeof(Byte);i++) {
			sum |= Byte[i] ^ other.Byte[i];
		}
		return !sum;
	}
	bool operator!=(const Binary<N>& other) const {
		return !((*this) == other);
	}
	operator bool() const {
		bool out = 0;
		for(unsigned i=0;i<sizeof(Byte);i++) {
			out |= Byte[i];
		}
		return out;
	}
	bool Zero() const {
		bool out = 0;
		for(unsigned i=0;i<sizeof(Byte);i++) {
			out |= Byte[i];
		}
		return !out;
	}
	void Random() {
		RandomBytes(Byte, sizeof(Byte));
	}
	int HammingWeight() {
		int out = 0;
		for(unsigned i=0;i<sizeof(Byte);i++) {
			out += __builtin_popcount(Byte[i]);
		}
		return out;
	}
	unsigned char Byte[N / 8];
};
typedef Binary<128> Binary128;
typedef Binary<256> Binary256;
typedef Binary<512> Binary512;
#pragma pack(pop)

