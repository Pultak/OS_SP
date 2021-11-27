#pragma once
#include "vfs.h"
#include "fsutils.h"

class FAT: public VFS{
public:

	FAT(uint8_t disk, kiv_hal::TDrive_Parameters params);

	kiv_os::NOS_Error mkdir(const char* pth, uint8_t attr) override;

	kiv_os::NOS_Error rmdir(const char* pth) override;

	kiv_os::NOS_Error dirread(const char* pth, std::vector<kiv_os::TDir_Entry>& entries) override;

	kiv_os::NOS_Error open(const char* pth, kiv_os::NOpen_File flags, uint8_t attributes, File& file) override;

	bool file_exist(const char* pth) override;

	kiv_os::NOS_Error write(File f, size_t size, size_t offset, const char* buffer, size_t& written) override;

	kiv_os::NOS_Error read(File f, size_t size, size_t offset, std::vector<char>& out) override;

	std::vector<char> convert_dirs_to_chars(const std::vector<kiv_os::TDir_Entry>& directories) override;

	kiv_os::NOS_Error set_file_attribute(const char* name, uint8_t attribute) override;

	kiv_os::NOS_Error get_file_attribute(const char* name, uint8_t& out_attribute) override;

	~FAT() {}

private:
	uint8_t disk;
	kiv_hal::TDrive_Parameters parameters;

};