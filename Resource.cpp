/* Copyright © 2018, Mykos Hudson-Crisp <micklionheart@gmail.com>
 * All rights reserved. */

#include "Resource.h"
#include <mutex>
#include <map>

std::mutex loaderMutex;
std::map<u64, std::function<Ref<Resource>(FILE*fp)>> loaders;

Resource::Resource() {
	fileRefCount = 0;
	version = 0;
	savePending = false;
}
Resource::~Resource() {
	// update file index
}
std::vector<char> Resource::Export() {
	std::vector<char> out;
	return out;
}

void Resource::AddFileRef() {
}
void Resource::RemoveFileRef() {
}

RefCount::RefCount() {
	refCount = 0;
}
RefCount::~RefCount() {
}
void RefCount::AddMemRef() {
	refCount++;
}
void RefCount::RemoveMemRef() {
	if (--refCount <= 0) delete this;
}

