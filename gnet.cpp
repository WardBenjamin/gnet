/* Copyright © 2018, Mykos Hudson-Crisp <micklionheart@gmail.com>
* All rights reserved. */

#include <mutex>
#include <map>
#include "GraphNet.h"
#include "UDP.h"
#include "blake2b.h"

using namespace std;

GNET_API void GNet_Startup() {
	UDP::Start();
}
GNET_API void GNet_Shutdown(int block) {
	UDP::Stop();
}
GNET_API void GNet_Connect(const char* hostname) {
	UDP::Connect(hostname);
}

mutex fileCacheMutex;
map<Binary256, GNet_File*> fileCache;

GNet_File* GNet_File_Alloc() {
	GNet_File* out = new GNet_File;
	memset(out->id, 0, 32);
	out->uses = 1;
	out->buffer = 0;
	out->length = 0;
	out->bitmap = 0;
	return out;
};

GNet_File* GNet_File_Get_Cache(Binary256 id) {
	lock_guard<mutex> L(fileCacheMutex);
	auto& f = fileCache[id];
	if (!f) {
		f = GNet_File_Alloc();
		memcpy(f->id, id, 32);
	}
	return f;
}

GNET_API GNet_File* GNet_File_Download(GNet_ID id) {
	auto f = GNet_File_Get_Cache(id);
	return f;
}

GNET_API GNet_File* GNet_File_Open(const char* filename) {
	FILE* fp = fopen(filename, "rb");
	if (!fp) return 0;
	fseek(fp, 0, SEEK_END);
	unsigned long long length = ftell(fp);
	rewind(fp);
	char* data = new char[length];
	fread(data, 1, length, fp);
	fclose(fp);
	Binary256 id;
	blake2b(id, 32, 0, 0, data, length);
	auto f = GNet_File_Get_Cache(id);
	f->length = length;
	if (f->buffer) delete f->buffer;
	f->buffer = data;
	if (f->bitmap) delete f->bitmap;
	int ints = f->length / (BLOCK_SIZE * CHAR_BIT * sizeof(int)) + 1;
	f->bitmap = new int[ints];
	memset(f->bitmap, 0xFF, ints * sizeof(int));
	return f;
}
GNET_API GNet_File* GNet_File_Create(const void* data, unsigned long long length) {
	Binary256 id;
	blake2b(id, 32, 0, 0, data, length);
	auto f = GNet_File_Get_Cache(id);
	if (f->length < length) {
		f->length = length;
		if (f->buffer) delete f->buffer;
		f->buffer = new char[f->length];
		if (f->bitmap) delete f->bitmap;
		int ints = f->length / (BLOCK_SIZE * CHAR_BIT * sizeof(int)) + 1;
		f->bitmap = new int[ints];
		memset(f->bitmap, 0xFF, ints * sizeof(int));
	}
	memcpy(f->buffer, data, length);
	return f;
}
GNET_API void GNet_File_Free(GNet_File* f) {
	if (!f) return;
	int use = f->uses--;
	if (use > 0) return;
	if (f->buffer) delete f->buffer;
	if (f->bitmap) delete f->bitmap;
	delete f;
}
GNET_API void GNet_File_Upload(GNet_File* file) {
	return;
}
