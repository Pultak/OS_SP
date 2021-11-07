#include "vfs.h"
#include "fsutils.h"

class FAT : public VFS {
public:

	explicit FAT(uint8_t disk, kiv_hal::TDrive_Parameters params);

	kiv_os::NOS_Error mkdir(const char* name, uint8_t attr) override;

	int rmdir(const char* name) override;

	std::vector<unsigned char> fat_table;

private:
	uint8_t disk;
	kiv_hal::TDrive_Parameters parameters;

};