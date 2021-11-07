#include "fsutils.h"
#include "../api/api.h"
#include "../api/hal.h"

std::vector<std::string> get_directories(const char* path){
	std::vector<std::string> directories;
	std::string pth = path;
	std::string delim = "/";
	size_t pos = 0;
	std::string dir;
	while ((pos = pth.find(delim)) != std::string::npos) {
		dir = pth.substr(0, pos);
		directories.push_back(dir);
	}
	return directories;
}

std::vector<unsigned char> load_first_table() {
	std::vector<unsigned char> fat_table_content;

	kiv_hal::TRegisters read_reg;
	kiv_hal::TDisk_Address_Packet read_packet;

	auto s = malloc(512 * 9);
	read_packet.count = 9;
	read_packet.sectors = (void*)s;
	read_packet.lba_index = 1;

	read_reg.rdx.l = 129;
	read_reg.rax.h = static_cast<decltype(read_reg.rax.h)>(kiv_hal::NDisk_IO::Read_Sectors);
	read_reg.rdi.r = reinterpret_cast<decltype(read_reg.rdi.r)>(&read_packet);

	kiv_hal::Call_Interrupt_Handler(kiv_hal::NInterrupt::Disk_IO, read_reg);

	char* buffer = reinterpret_cast<char*>(read_packet.sectors);

	for (int i = 0; i < sizeof(s); i++) {
		fat_table_content.push_back(buffer[i]);
	}

	free(s);
	return fat_table_content;
}

std::vector<unsigned char> load_second_table() {
	std::vector<unsigned char> fat_table_content;

	kiv_hal::TRegisters read_reg;
	kiv_hal::TDisk_Address_Packet read_packet;

	auto s = malloc(512 * 9);
	read_packet.count = 9;
	read_packet.sectors = (void*)s;
	read_packet.lba_index = 10;

	read_reg.rdx.l = 129;
	read_reg.rax.h = static_cast<decltype(read_reg.rax.h)>(kiv_hal::NDisk_IO::Read_Sectors);
	read_reg.rdi.r = reinterpret_cast<decltype(read_reg.rdi.r)>(&read_packet);

	kiv_hal::Call_Interrupt_Handler(kiv_hal::NInterrupt::Disk_IO, read_reg);

	char* buffer = reinterpret_cast<char*>(read_packet.sectors);

	for (int i = 0; i < sizeof(s); i++) {
		fat_table_content.push_back(buffer[i]);
	}

	free(s);
	return fat_table_content;
}

void write_to_fs(int start, std::vector<char> data) {
	kiv_hal::TRegisters write_reg;
	kiv_hal::TDisk_Address_Packet write_packet;

	write_packet.count = data.size() / 512 + (data.size() % 512 != 0);
	write_packet.lba_index = static_cast<size_t>(start) + 31;
}

std::vector<unsigned char> read_data(int start, int sectors_count) {
	kiv_hal::TRegisters read_regs;
	kiv_hal::TDisk_Address_Packet read_packet;
	
	read_packet.count = sectors_count;
	auto s = malloc(512 * static_cast<size_t>(sectors_count));
	read_packet.sectors = (void*)s;
	read_regs.rdx.l = 129;
	read_regs.rax.h = static_cast<decltype(read_regs.rax.h)>(kiv_hal::NDisk_IO::Read_Sectors);
	read_regs.rdi.r = reinterpret_cast<decltype(read_regs.rdi.r)>(&read_packet);
	
	kiv_hal::Call_Interrupt_Handler(kiv_hal::NInterrupt::Disk_IO, read_regs);

	char* buffer = reinterpret_cast<char*>(read_packet.sectors);
	std::vector<unsigned char> content(buffer, buffer + (static_cast<size_t>(sectors_count) * 512));
	free(s);
	return content;
}