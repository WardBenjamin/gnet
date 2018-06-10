/* Copyright © 2018, Mykos Hudson-Crisp <micklionheart@gmail.com>
 * All rights reserved. */
#pragma once

#include "Binary.h"

typedef Binary512 Signature;
typedef Binary256 Ciphertext;
typedef Binary256 Digest;
typedef Binary256 PublicKey;
typedef Binary256 PrivateKey;

struct KeyPair {
	KeyPair(const void* publicKey = 0, const void* privateKey = 0);
	void Keygen();
	void ImportPublic(const Binary256& in);
	void ExportPublic(Binary256& out);
	bool ImportPrivate(const Binary256& in);
	void ExportPrivate(Binary256& out);
	bool Sign(Signature& out, const Digest& hash);
	bool Verify(const Signature& in, const Digest& hash);
	bool SignData(Signature& out, const void* data, size_t len);
	bool VerifyData(const Signature& in, const void* data, size_t len);
	bool Encrypt(Ciphertext& out, Digest key);
	bool Decrypt(const Ciphertext& in, Digest key);

	bool hasPrivate;
	unsigned char publicKey[33];
	unsigned char privateKey[32];
};

extern KeyPair masterKey;

