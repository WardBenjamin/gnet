// gnet.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "GraphNet.h"

void GNet_Startup() {

}
void GNet_Shutdown(int block) {

}
void GNet_Connect(const char* hostname) {

}

GNet_File* GNet_Download(GNet_ID id) {
	return 0;
}
GNet_File* GNet_Load_File(const char* filename) {
	return 0;
}
GNet_File* GNet_Load_Data(void* data, unsigned long long length) {
	return 0;
}
void GNet_Free_File(GNet_File* file) {
	return;
}
void GNet_Upload(GNet_File* file) {
	return;
}
