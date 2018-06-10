/* Copyright © 2018, Mykos Hudson-Crisp <micklionheart@gmail.com>
* All rights reserved. */
#pragma once

#include "NumericTypes.h"
#include <vector>

struct MathPacker {
	MathPacker();
	MathPacker(const MathPacker& other);

	u64 FitModulus(u64 target = 0);
	void Write(u64 value, u64 modulus);
	void Write(s64 value, s64 min, s64 max);

	u64 Bits();
	void Export(u8* buffer, size_t length = 0);
	std::vector<u8> Export();

	std::vector<u64> Buffer;
	std::vector<u64> Moduli;
	std::vector<u64> Values;
	u64 Scale;
};

struct MathUnpacker {
	MathUnpacker();
	MathUnpacker(const std::vector<u8>& buffer, u64 bits = 0);

	u64 FitModulus(u64 target = 0);
	u64 Read(u64 modulus);
	s64 Read(s64 min, s64 max);
	
	void Import(const u8* buffer, u64 bits);
	void Import(const std::vector<u8>& buffer);

	std::vector<u64> Buffer;
	u64 Scale;
};

struct MessageDataPoint {
	u64 value, modulus;
};

std::string ParseStaticCode(MathUnpacker& data);
void BufferStaticCode(std::vector<MessageDataPoint>& buf, std::string str);
