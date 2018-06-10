/* Copyright © 2018, Mykos Hudson-Crisp <micklionheart@gmail.com>
 * All rights reserved. */
#pragma once

#pragma pack(push, 1)
struct Float16 {
	short bits;
	inline Float16() : bits(0) {};
	//template <class X> inline Float16(X f) {*this = float(f);}
	//template <class X> inline Float16& operator=(X f) {return *this = float(f);}
	//template <class X> inline operator X() {return X(float(*this));}
	inline Float16& operator=(float f) {
		unsigned inbits = reinterpret_cast<unsigned&>(f);
		int sign = (inbits >> 16) & 0x00008000;
		int exponent = ((inbits >> 23) & 0x000000FF) - (127 - 15);
		int mantissa = inbits & 0x007FFFFF;
		if (exponent <= 0) {
			if (exponent < -10) {
				bits = 0;
			} else {
				mantissa = (mantissa | 0x00800000) >> (1 - exponent);
				if (mantissa & 0x00001000) mantissa += 0x00002000;
				bits = sign | (mantissa >> 13);
			}
		} else if (exponent == 0xFF - (127 - 15)) {
			if (mantissa == 0) {
				bits = sign | 0x7C00;
			} else {
				mantissa >>= 13;
				bits = sign | 0x7C00 | mantissa | (mantissa == 0);
			}
		} else {
			if (mantissa & 0x00001000) {
				mantissa += 0x00002000;
				if (mantissa & 0x00800000) {
					mantissa = 0;
					exponent += 1;
				}
			}
			if (exponent > 30) {
				bits = sign | 0x7C00;
			} else {
				bits = sign | (exponent << 10) | (mantissa >> 13);
			}
		}
		return *this;
	}
	inline operator float() {
		unsigned outbits = 0;
		int sign = (bits >> 15) & 0x00000001;
		int exponent = (bits >> 10) & 0x0000001F;
		int mantissa = bits & 0x000003FF;
		if (exponent == 0) {
			if (mantissa == 0) {
				outbits = sign << 31;
				return reinterpret_cast<float&>(outbits);
			} else {
				while ((mantissa & 0x00000400) == 0) {
					mantissa <<= 1;
					exponent -= 1;
				}
				exponent += 1;
				mantissa &= ~0x00000400;
			}
		} else if (exponent == 31) {
			if (mantissa == 0) {
				outbits = (sign << 31) | 0x7F800000;
				return reinterpret_cast<float&>(outbits);
			} else {
				outbits = (sign << 31) | 0x7F800000 | (mantissa << 13);
				return reinterpret_cast<float&>(outbits);
			}
		}
		exponent += 127 - 15;
		mantissa <<= 13;
		outbits = (sign << 31) | (exponent << 23) | mantissa;
		return reinterpret_cast<float&>(outbits);
	}
};
#pragma pack(pop)

