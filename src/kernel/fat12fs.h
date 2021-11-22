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

	bool file_exist(const char* pth, int32_t d, int32_t& found_d) override;




	~FAT() {}

private:
	uint8_t disk;
	kiv_hal::TDrive_Parameters parameters;

};