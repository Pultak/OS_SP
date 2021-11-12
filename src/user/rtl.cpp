#include "rtl.h"

std::atomic<kiv_os::NOS_Error> kiv_os_rtl::Last_Error;

kiv_hal::TRegisters Prepare_SysCall_Context(kiv_os::NOS_Service_Major major, uint8_t minor) {
	kiv_hal::TRegisters regs;
	regs.rax.h = static_cast<uint8_t>(major);
	regs.rax.l = minor;
	return regs;
}


bool kiv_os_rtl::Read_File(const kiv_os::THandle file_handle, char* const buffer, const size_t buffer_size, size_t &read) {
	kiv_hal::TRegisters regs =  Prepare_SysCall_Context(kiv_os::NOS_Service_Major::File_System, static_cast<uint8_t>(kiv_os::NOS_File_System::Read_File));
	regs.rdx.x = static_cast<decltype(regs.rdx.x)>(file_handle);
	regs.rdi.r = reinterpret_cast<decltype(regs.rdi.r)>(buffer);
	regs.rcx.r = buffer_size;	
	
	const bool result = kiv_os::Sys_Call(regs);
	read = regs.rax.r;
	return result;
}

bool kiv_os_rtl::Write_File(const kiv_os::THandle file_handle, const char *buffer, const size_t buffer_size, size_t &written) {
	kiv_hal::TRegisters regs = Prepare_SysCall_Context(kiv_os::NOS_Service_Major::File_System, static_cast<uint8_t>(kiv_os::NOS_File_System::Write_File));
	regs.rdx.x = static_cast<decltype(regs.rdx.x)>(file_handle);
	regs.rdi.r = reinterpret_cast<decltype(regs.rdi.r)>(buffer);
	regs.rcx.r = buffer_size;

	const bool result = kiv_os::Sys_Call(regs);
	written = regs.rax.r;
	return result;
}

bool kiv_os_rtl::Set_Working_Dir(const char* path) {
	kiv_hal::TRegisters regs = Prepare_SysCall_Context(kiv_os::NOS_Service_Major::File_System, static_cast<uint8_t>(kiv_os::NOS_File_System::Set_Working_Dir));
	regs.rdx.r = reinterpret_cast<decltype(regs.rdx.r)>(path);
	const bool result = kiv_os::Sys_Call(regs);
	return result;
}

bool kiv_os_rtl::Get_Working_Dir(const char* buffer, size_t buffer_size, size_t& read) {
	kiv_hal::TRegisters regs = Prepare_SysCall_Context(kiv_os::NOS_Service_Major::File_System, static_cast<uint8_t>(kiv_os::NOS_File_System::Get_Working_Dir));

	regs.rdx.r = reinterpret_cast<decltype(regs.rdx.r)>(buffer);
	regs.rcx.r = buffer_size;

	const bool result = kiv_os::Sys_Call(regs);

	read = static_cast<size_t>(regs.rax.r);

	return result;
}

bool kiv_os_rtl::md(const kiv_hal::TRegisters& regs) {
	const auto std_out = static_cast<kiv_os::THandle>(regs.rbx.x);
	//char* directory_name = reinterpret_cast<char*>(regs.rdi.r);
	char* directory_name = "C:\\slozka";
	if (directory_name == nullptr || strlen(directory_name) == 0) {
		const char* error_message = "Name not specified.\n";
		size_t written;
		kiv_os_rtl::Write_File(std_out, error_message, strlen(error_message), written);
	}
	else {
		
		auto flags = static_cast<kiv_os::NOpen_File>(0);

		auto attributes = static_cast<uint8_t>(kiv_os::NFile_Attributes::Directory);

		kiv_os::THandle handle;
		kiv_os::NOS_Error error;
		if (kiv_os_rtl::Open_Filesystem(directory_name, flags, attributes, handle, error)) {
			printf("otevren fs");
		}
		else {
			if (error == kiv_os::NOS_Error::Invalid_Argument) {
				const char* error_message = "File already exists or invalid name specified.\n";
				size_t written;
				kiv_os_rtl::Write_File(std_out, error_message, strlen(error_message), written);
			}
			else {
				printf("error handle not implemented");
			}
		}
	}

	return false;

}

bool kiv_os_rtl::Open_Filesystem(const char* file_name, kiv_os::NOpen_File flags, uint8_t attributes, kiv_os::THandle& handle, kiv_os::NOS_Error& error) {
	kiv_hal::TRegisters regs = Prepare_SysCall_Context(kiv_os::NOS_Service_Major::File_System, static_cast<uint8_t>(kiv_os::NOS_File_System::Open_File));
	regs.rdx.r = reinterpret_cast<decltype(regs.rdx.r)>(file_name);
	regs.rcx.l = static_cast<decltype(regs.rcx.l)>(flags);
	regs.rdi.i = attributes;
	const bool result = kiv_os::Sys_Call(regs);
	
	if (result) {
		handle = regs.rax.x;
	}
	else {
		error = static_cast<kiv_os::NOS_Error>(regs.rax.x);
	}

	return result;
}
