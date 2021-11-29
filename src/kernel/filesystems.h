#pragma once
#include<map>
#include<string>
#include<filesystem>
#include<memory>
#include<iostream>
#include "vfs.h"
#include "IOHandle.h"
#include "FileHandle.h"
#include "PCBFile.h"

namespace filesystems {

	void InitFilesystems();

	void parse_path(const char* abs_path, const char* rel_path, std::string& result);

	void Add_To_Filesystems(const std::string& name, VFS* vfs);

	VFS* Filesystem_exists(std::filesystem::path path);

	VFS* Get_Filesystem(const std::string& file_name);

	IOHandle* Open_File(const char* input_file_name, kiv_os::NOpen_File flags, uint8_t attributes, kiv_os::NOS_Error& error);

}
