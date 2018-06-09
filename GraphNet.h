/* Copyright © 2018, Mykos Hudson-Crisp <micklionheart@gmail.com>
* All rights reserved. */
#pragma once

#ifdef GNET_EXPORTS
#define GNET_API __declspec(dllexport)
#else  
#define GNET_API __declspec(dllimport)
#endif  

#ifdef __CPLUSPLUS
extern "C" {
#endif

	typedef char GNet_ID[32];

	GNET_API void GNet_Startup();
	GNET_API void GNet_Shutdown(int block);
	GNET_API void GNet_Connect(const char*);

	typedef struct GNet_File {
		int uses;
		GNet_ID id;
		void* buffer;
		unsigned long long length;
		int* bitmap;
	} GNet_File;

	GNET_API GNet_File* GNet_Download(GNet_ID id);
	GNET_API GNet_File* GNet_Load_File(const char* filename);
	GNET_API GNet_File* GNet_Load_Data(void* data, unsigned long long length);
	GNET_API void GNet_Free_File(GNet_File* file);
	GNET_API void GNet_Upload(GNet_File* file);

	typedef struct GNet_Variable {
		int uses;
		GNet_ID id;
		GNet_ID secret;
		GNet_File* exact;
		double approx;
	} GNet_Variable;

	GNET_API GNet_Variable* GNet_Create_Variable();
	GNET_API void GNet_Write_Variable_Approx(GNet_Variable* var, double value);
	GNET_API void GNet_Write_Variable_Exact(GNet_Variable* var, void* data, unsigned long long length);
	GNET_API double GNet_Read_Variable_Approx(GNet_Variable* var);
	GNET_API GNet_File* GNet_Read_Variable_Exact(GNet_Variable* var);
	GNET_API void GNet_Free_Variable(GNet_Variable* var);

	typedef struct GNet_Endpoint {
		int uses;
		GNet_ID id;
		GNet_ID secret;
	} GNet_Endpoint;

	GNET_API GNet_Endpoint* GNet_Create_Endpoint();
	GNET_API GNet_Endpoint* GNet_Import_Endpoint(GNet_ID secret);
	GNET_API void GNet_Free_Endpoint(GNet_Endpoint* node);

	GNET_API GNet_File* GNet_Pull_File(GNet_Endpoint* node, const char* filename);
	GNET_API void GNet_Push_File(GNet_Endpoint* node, const char* filename, GNet_File* file);

	GNET_API GNet_File* GNet_Pull_Message(GNet_Endpoint* node);
	GNET_API void GNet_Push_Message(GNet_Endpoint* node, GNet_File* message);

#ifdef __CPLUSPLUS
};
#endif
