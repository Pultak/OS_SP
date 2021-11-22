#pragma once
#include<map>
#include<string>
#include<filesystem>
#include<memory>
#include<iostream>
#include "vfs.h"

namespace Files {
    std::map<std::string, std::unique_ptr<VFS>> Filesystems;
}
void InitFilesystems();

void Add_To_Filesystems(const std::string &name, VFS* vfs);

VFS* Filesystem_exists(std::filesystem::path path, std::filesystem::path& path_relative_to_fs, std::filesystem::path& absolute_path);

VFS* Get_Filesystem(const std::string& file_name);

void Open_File(kiv_hal::TRegisters& registers);

kiv_os::THandle Open_File(const char* input_file_name, kiv_os::NOpen_File flags, uint8_t attributes, kiv_os::NOS_Error& error);
