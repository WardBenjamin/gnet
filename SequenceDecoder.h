/* Copyright © 2018, Mykos Hudson-Crisp <micklionheart@gmail.com>
* All rights reserved. */
#pragma once

#include "NumericTypes.h"

template <int Bits>
struct SequenceDecoder {
	const u64 HighMask = -(1 << Bits);
	const u64 LowMask = (1 << Bits) - 1;
	const u64 Modulus = 1 << Bits;
	const u64 Radius = 1 << (Bits - 1);
	
	SequenceDecoder() {
		Latest = 0;
	}
	u64 Decode(u64 lsb) {
		u64 guess = (Latest & HighMask) ^ (lsb & LowMask);
		if (guess + Radius <= Latest) guess += Modulus;
		return guess;
	}
	void Confirm(u64 id) {
		if (Latest < id) {
			Latest = id;
		}
	}

	u64 Latest;
};

