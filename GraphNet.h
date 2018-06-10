/* Copyright © 2018, Mykos Hudson-Crisp <micklionheart@gmail.com>
* All rights reserved. */
#pragma once

#ifdef GNET_EXPORTS
#define GNET_API extern "C" __declspec(dllexport)
#else  
#define GNET_API extern "C" __declspec(dllimport)
#endif  

#define BLOCK_SIZE 4096

typedef char GNet_ID[32];

GNET_API void GNet_Startup();
GNET_API void GNet_Shutdown(int blocking);
GNET_API void GNet_Connect(const char* hostname);

typedef struct GNet_File {
	int uses;
	GNet_ID id;
	char* buffer;
	unsigned long long length;
	int* bitmap;
} GNet_File;

GNET_API GNet_File* GNet_File_Open(const char* filename);
GNET_API GNet_File* GNet_File_Create(const void* data, unsigned long long length);
GNET_API GNet_File* GNet_File_Download(GNet_ID id);
GNET_API void GNet_File_Upload(GNet_File* file);
GNET_API void GNet_File_Free(GNet_File* file);

typedef struct GNet_Endpoint {
	int uses;
	GNet_ID id;
	GNet_ID secret;
} GNet_Endpoint;

typedef void(*GNet_Endpoint_Callback)(const void* token, GNet_Endpoint* src, const void* data, unsigned long length);

GNET_API GNet_Endpoint* GNet_Endpoint_Create();
GNET_API float GNet_Endpoint_Latency(GNet_Endpoint* node);
GNET_API GNet_Endpoint* GNet_Endpoint_Import(GNet_ID secret);
GNET_API void GNet_Endpoint_Free(GNet_Endpoint* node);

GNET_API int GNet_Endpoint_Listen(GNet_Endpoint* node, const char* service, GNet_Endpoint_Callback callback, const void* token);
GNET_API void GNet_Endpoint_Send(GNet_Endpoint* node, const char* service, const void* data, unsigned long length);
