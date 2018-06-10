/* Copyright © 2018, Mykos Hudson-Crisp <micklionheart@gmail.com>
 * All rights reserved. */

#include "KeyPair.h"
extern "C" {
	#include "ecc.h"
	#include "blake2b.h"
};

#define FUCKING_OMG
#ifdef FUCKING_OMG
KeyPair masterKey(
	"\xB7\x6B\x5B\x4A\xD2\xB2\x55\xD2\x3E\xAA\x4A\xE6\x0C\x1D\x0D\xB4"
	"\xA4\xC8\x6D\x6C\xB9\x38\x6B\xEA\x83\xA5\xD3\x55\x3F\x1F\xC0\xC1",
	"\xF9\x71\xCA\x4E\x20\x47\x9D\x58\x1B\xFB\x4C\x0B\x92\xF5\x43\x79"
	"\x78\x0D\x0E\xF3\x4C\xA5\x6B\xF6\xD7\x5D\xE4\xF7\x35\xF0\x88\xF9");
#else
KeyPair masterKey(
	"\xB7\x6B\x5B\x4A\xD2\xB2\x55\xD2\x3E\xAA\x4A\xE6\x0C\x1D\x0D\xB4"
	"\xA4\xC8\x6D\x6C\xB9\x38\x6B\xEA\x83\xA5\xD3\x55\x3F\x1F\xC0\xC1");
#endif

const unsigned char DAT_BYTE = 0x03;

KeyPair::KeyPair(const void* publicKey_, const void* privateKey_) {
	hasPrivate = false;
	publicKey[0] = DAT_BYTE;
	if (publicKey_) memcpy(&publicKey[1], publicKey_, 32);
	if (privateKey_) memcpy(&privateKey[0], privateKey_, 32);
}
void KeyPair::Keygen() {
	for(;;) {
		ecc_make_key(publicKey, privateKey);
		if (publicKey[0] == DAT_BYTE) break;
	}
	hasPrivate = true;
}
void KeyPair::ImportPublic(const Binary256& in) {
	publicKey[0] = DAT_BYTE;
	memcpy(&publicKey[1], in, 32);
}
void KeyPair::ExportPublic(Binary256& out) {
	memcpy(out, &publicKey[1], 32);
}
bool KeyPair::ImportPrivate(const Binary256& in) {
	memcpy(&privateKey[0], in, 32);
	hasPrivate = true;
	//if (!ecc_make_key_seed(publicKey, privateKey)) return false;
	return publicKey[0] == DAT_BYTE;
}
void KeyPair::ExportPrivate(Binary256& out) {
	memcpy(out, &privateKey[0], 32);
}
bool KeyPair::Sign(Signature& out, const Digest& hash) {
	return ecdsa_sign(privateKey, hash.Byte, out.Byte);
}
bool KeyPair::Verify(const Signature& in, const Digest& hash) {
	return ecdsa_verify(publicKey, hash.Byte, in.Byte);
}
bool KeyPair::SignData(Signature& out, const void* data, size_t len) {
	Digest hash;
	blake2b(hash, sizeof(hash), 0, 0, data, len);
	return Sign(out, hash);
}
bool KeyPair::VerifyData(const Signature& in, const void* data, size_t len) {
	Digest hash;
	blake2b(hash, sizeof(hash), 0, 0, data, len);
	return Verify(in, hash);
}
bool KeyPair::Encrypt(Ciphertext& out, Digest key) {
	KeyPair temp;
	temp.Keygen();
	memcpy(out, &temp.publicKey[1], sizeof(out));
	if (!ecdh_shared_secret(publicKey, temp.privateKey, key.Byte)) return false;
	blake2b(key, sizeof(key), 0, 0, key, sizeof(key));
	return true;
}
bool KeyPair::Decrypt(const Ciphertext& in, Digest key) {
	unsigned char temp[33];
	temp[0] = DAT_BYTE;
	memcpy(&temp[1], in, 32);
	if (!ecdh_shared_secret(temp, privateKey, key.Byte)) return false;
	blake2b(key, sizeof(key), 0, 0, key, sizeof(key));
	return true;
}

