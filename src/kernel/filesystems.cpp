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

VFS* filesystems::Filesystem_exists(std::filesystem::path path) {
	auto current_path = path.root_path();
	VFS* current_fs = filesystems::Get_Filesystem(current_path.string());
	if (current_fs != nullptr) {
		return current_fs;
	}
	return nullptr;
}

IOHandle* filesystems::Open_File(const char* input_file_name, kiv_os::NOpen_File flags, uint8_t attributes, kiv_os::NOS_Error& error) {
	//std::filesystem::path resolved_path_relative_to_fs;
	std::filesystem::path input_path = input_file_name;
	std::string file_name = input_path.filename().string();
	IOHandle* file = nullptr;
	auto fs = Filesystem_exists(input_path);
	if (fs != nullptr) {
		auto length = input_path.relative_path().string().length() + 1;
		char* name = new char[length];
		strcpy_s(name, length, input_path.relative_path().string().c_str());
		File* f = new File();
		auto result = fs->open(name, flags, attributes, f);
		if (result == kiv_os::NOS_Error::Success) {

			//opening system reserved tasklist file?
			if (strcmp(name, "tasklist") == 0) {
				file = new PCBFileHandle(fs, f);
			}
			//opening other file
			else {
				file = new FileHandle(fs, f);
			}

		}
		else {
			error = kiv_os::NOS_Error::File_Not_Found;
		}
	}
	else {
		error = kiv_os::NOS_Error::Unknown_Filesystem;
	}
	return file;
}

void filesystems::parse_path(const char* abs_path, const char* rel_path, std::string& result) {
	std::vector<std::string> abs;
	std::string pth = abs_path;
	std::string delim = "\\";
	size_t pos = 0;
	std::string dir;
	while ((pos = pth.find(delim)) != std::string::npos) {
		dir = pth.substr(0, pos);
		abs.push_back(dir);
		pth.erase(0, pos + delim.length());
	}
	if (!pth.empty()) {
		abs.push_back(pth);
	}
	std::vector<std::string> rel;
	std::string pth1 = rel_path;
	std::string delim1 = "\\";
	size_t pos1 = 0;
	std::string dir1;
	while ((pos1 = pth1.find(delim1)) != std::string::npos) {
		dir1 = pth1.substr(0, pos1);
		rel.push_back(dir1);
		pth1.erase(0, pos1 + delim1.length());
	}
	rel.push_back(pth1);
	for (int i = 0; i < rel.size(); i++) {
		if (strcmp(rel[i].c_str(), "..") == 0) {
			abs.pop_back();
		}
		else if (strcmp(rel[i].c_str(), ".") == 0) {}
		else {
			abs.push_back(rel[i]);
		}
		if (abs.size() <= 0) {
			abs.push_back("C:");
		}

	}
	for (int i = 0; i < abs.size() - 1; i++) {
		result += abs[i] + "\\";
	}
	if (abs.size() == 1) {
		result += abs[abs.size() - 1] + "\\";
	}
	else {
		result += abs[abs.size() - 1];
	}
}