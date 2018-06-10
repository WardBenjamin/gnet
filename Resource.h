/* Copyright © 2018, Mykos Hudson-Crisp <micklionheart@gmail.com>
 * All rights reserved. */
#pragma once

#include <string>
#include <vector>
#include <functional>
#include "Binary.h"
#include "NumericTypes.h"

template <class T> class Ref {
public:
	T* p;
	Ref() : p(0) {};
	~Ref() { if (p) p->RemoveMemRef(); };
	Ref(const Ref<T>& _p) : p(_p.p) { if (p) p->AddMemRef(); };
	Ref(T* _p) : p(_p) { if (p) p->AddMemRef(); };
	template <class U> Ref(Ref<U> _p) : p(_p.Cast<T>()) {
		if (p) p->AddMemRef();
	};
	void operator=(const Ref<T>& o) {
		if (p) p->RemoveMemRef();
		p = o.p;
		if (p) p->AddMemRef();
		return;
	}
	bool operator==(const Ref<T>& o) const { return p == o.p; }
	bool operator!=(const Ref<T>& o) const { return p != o.p; }
	bool operator<(const Ref<T>& o) const { return p < o.p; }
	bool operator==(const T* o) const { return p == o; }
	bool operator!=(const T* o) const { return p != o; }
	T& operator*() const { return *p; }
	T* operator->() const { return p; }
	bool operator!() const { return p == 0; }
	operator bool() const { return p != 0; }
	T* Ptr() const { return p; }
	template <class U> U* Cast() const { return p ? dynamic_cast<U*>(p) : 0; }
};

struct RefCount {
	RefCount();
	virtual ~RefCount();
	void AddMemRef();
	void RemoveMemRef();
	size_t refCount;
};

typedef Binary<256> ResourceID;
struct Resource : public RefCount {
	static void RegisterLoaders();
	static void RegisterLoader(u64 magic, std::function<Ref<Resource>(FILE*fp)> loader);
	static Ref<Resource> Get(std::string url);
	static Ref<Resource> Read(FILE* fp);

	Resource();
	virtual ~Resource();
	virtual std::vector<char> Export();
	
	void AddFileRef();
	void RemoveFileRef();
	
	ResourceID id;
	size_t refCount, fileRefCount;
	std::string name;
	unsigned version;
	bool savePending;
};

