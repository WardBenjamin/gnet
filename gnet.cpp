// gnet.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "GraphNet.h"

GNET_API void GNet_Startup() {

}
GNET_API void GNet_Shutdown(int block) {

}
GNET_API void GNet_Connect(const char* hostname) {

}

GNET_API GNet_File* GNet_Download(GNet_ID id) {
	return 0;
}
GNET_API GNet_File* GNet_Load_File(const char* filename) {
	return 0;
}
GNET_API GNet_File* GNet_Load_Data(void* data, unsigned long long length) {
	return 0;
}
GNET_API void GNet_Free_File(GNet_File* file) {
	return;
}
GNET_API void GNet_Upload(GNet_File* file) {
	return;
}
