#include "fat12fs.h"


FAT::FAT(uint8_t disk_num, kiv_hal::TDrive_Parameters params): disk(disk_num), parameters(params) {
	fat_table = load_first_table();
}

kiv_os::NOS_Error FAT::mkdir(const char* name, uint8_t attr)
{
	std::vector<std::string> path = get_directories(name);
	return kiv_os::NOS_Error();
}
