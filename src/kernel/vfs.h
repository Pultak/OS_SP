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
	virtual kiv_os::NOS_Error mkdir(const char* pth, uint8_t attr) = 0;

    virtual kiv_os::NOS_Error rmdir(const char* pth) = 0;


    virtual kiv_os::NOS_Error dirread(const char* pth, std::vector<kiv_os::TDir_Entry>& entries) = 0;


	virtual kiv_os::NOS_Error open(const char* pth, kiv_os::NOpen_File flags, uint8_t attributes, File& file) = 0;

    virtual bool file_exist(const char* pth, int32_t d, int32_t& found_d) = 0;

    virtual kiv_os::NOS_Error write(File f, size_t size, size_t offset, const char* buffer, size_t& written) = 0;

	~VFS() {};

};