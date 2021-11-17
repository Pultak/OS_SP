#pragma once
#include "../api/api.h"
#include <string>
#include<vector>

struct File {
    kiv_os::THandle handle;
    uint8_t attributes;
    size_t size;
    size_t position;
    char* name;
};

class VFS {
public:
	virtual kiv_os::NOS_Error mkdir(const char* name, uint8_t attr) = 0;

    virtual kiv_os::NOS_Error rmdir(const char* name) = 0;


    virtual kiv_os::NOS_Error dirread(const char* name, std::vector<kiv_os::TDir_Entry>& entries) = 0;


	virtual kiv_os::NOS_Error open(const char* name, kiv_os::NOpen_File flags, uint8_t attributes, File& file) = 0;


	~VFS() {};
};