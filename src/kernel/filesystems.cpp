#include "filesystems.h"
#include "fat12fs.h"

namespace Files {
	std::map<std::string, std::unique_ptr<VFS>> Filesystems;
}
void filesystems::InitFilesystems() {
    
    kiv_hal::TRegisters regs{};
	for (regs.rdx.l = 0; ; regs.rdx.l++) {
		kiv_hal::TDrive_Parameters params{};
		regs.rax.h = static_cast<uint8_t>(kiv_hal::NDisk_IO::Drive_Parameters);;
		regs.rdi.r = reinterpret_cast<decltype(regs.rdi.r)>(&params);
		kiv_hal::Call_Interrupt_Handler(kiv_hal::NInterrupt::Disk_IO, regs);

		if (!regs.flags.carry) {
			
			auto disk_num = regs.rdx.l;
			auto disk_params = reinterpret_cast<kiv_hal::TDrive_Parameters*>(regs.rdi.r);

			auto fs = new FAT(disk_num, *disk_params);
			
			filesystems::Add_To_Filesystems(R"(C:\)", fs);

			break;


		}

		if (regs.rdx.l == 255) break;
	}
        
}

VFS* filesystems::Get_Filesystem(const std::string& file_name) {
	auto result = Files::Filesystems.find(file_name);
	if (result != Files::Filesystems.end()) {
		return result->second.get();
	}

	return nullptr;
}

void filesystems::Add_To_Filesystems(const std::string& name, VFS* vfs) {
	Files::Filesystems[name] = std::unique_ptr<VFS>(vfs);
}

VFS* filesystems::Filesystem_exists(std::filesystem::path path, std::filesystem::path& path_relative_to_fs, std::filesystem::path& absolute_path) {
	auto current_path = path.root_path();
	VFS* current_fs = filesystems::Get_Filesystem(current_path.string());
	
	if (current_fs != nullptr) {
		return current_fs;
	}
	return nullptr;
}

/*void Open_File(kiv_hal::TRegisters& registers) {
	char* file_name = reinterpret_cast<char*>(registers.rdx.r);
	auto flags = static_cast<kiv_os::NOpen_File>(registers.rcx.l);
	auto attributes = static_cast<uint8_t>(registers.rdi.i);
	kiv_os::NOS_Error error = kiv_os::NOS_Error::Success;
	auto handle = Open_File(file_name, flags, attributes, error);

	if (error == kiv_os::NOS_Error::Success) {
		registers.rax.x = handle;
	}
	else {
		registers.flags.carry = 1;
		registers.rax.x = static_cast<decltype(registers.rax.x)>(error);
	}
}*/

IOHandle* filesystems::Open_File(const char* input_file_name, kiv_os::NOpen_File flags, uint8_t attributes, kiv_os::NOS_Error& error) {
	std::filesystem::path resolved_path_relative_to_fs;
	std::filesystem::path absolute_path;

	std::filesystem::path input_path = input_file_name;
	std::string file_name = input_path.filename().string();
	auto fs = Filesystem_exists(input_path, resolved_path_relative_to_fs, absolute_path);
	if (fs != nullptr) {
		printf(" fs nalezen ");
		auto length = resolved_path_relative_to_fs.string().length() + 1;
		char* name = new char[length];
		strcpy_s(name, length, resolved_path_relative_to_fs.string().c_str());
		
		File f{};
		auto result = fs->open("slozka4\\slozka5\\slozka6", flags, attributes, f);
		if (result == kiv_os::NOS_Error::Success) {
			printf(" pridan soubor ");
		}
		else {
			printf(" pridani souboru se nepodarilo ");
		}
	}
	else {
		error = kiv_os::NOS_Error::File_Not_Found;
	}
	return kiv_os::Invalid_Handle;

}