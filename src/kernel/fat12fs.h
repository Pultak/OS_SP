#pragma once
#include "vfs.h"
#include "fsutils.h"

class FAT: public VFS{
public:

	FAT(uint8_t disk, kiv_hal::TDrive_Parameters params);

	kiv_os::NOS_Error mkdir(const char* name, uint8_t attr) override;

	kiv_os::NOS_Error dirread(const char* name, std::vector<kiv_os::TDir_Entry>& entries) override;

	kiv_os::NOS_Error open(const char* name, kiv_os::NOpen_File flags, uint8_t attributes, File& file) override;

	~FAT() {}

private:
	uint8_t disk;
	kiv_hal::TDrive_Parameters parameters;

};