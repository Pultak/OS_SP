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

bool kiv_os_rtl::Set_Working_Dir(const char* dir) {
	kiv_hal::TRegisters regs = Prepare_SysCall_Context(kiv_os::NOS_Service_Major::File_System, static_cast<uint8_t>(kiv_os::NOS_File_System::Set_Working_Dir));
	regs.rdx.x = reinterpret_cast<decltype(regs.rdx.x)>(dir);

	const bool result = kiv_os::Sys_Call(regs);
	return result;
}

bool kiv_os_rtl::Get_Working_Dir(char* buffer, const size_t buffer_size, size_t& chars_written) {
	kiv_hal::TRegisters regs = Prepare_SysCall_Context(kiv_os::NOS_Service_Major::File_System, static_cast<uint8_t>(kiv_os::NOS_File_System::Get_Working_Dir));
	regs.rdx.x = reinterpret_cast<decltype(regs.rdx.x)>(buffer);
	regs.rcx.r = buffer_size;

	const bool result = kiv_os::Sys_Call(regs);
	chars_written = regs.rax.r;
	return result;
}

bool kiv_os_rtl::Open_File(char* file_name, kiv_os::NOpen_File file_open, kiv_os::NFile_Attributes file_attribute, kiv_os::THandle &file_handle_ret) {
	kiv_hal::TRegisters regs = Prepare_SysCall_Context(kiv_os::NOS_Service_Major::File_System, static_cast<uint8_t>(kiv_os::NOS_File_System::Open_File));
	regs.rdx.r = reinterpret_cast<decltype(regs.rdx.x)>(file_name);
	regs.rcx.r = static_cast<decltype(regs.rcx.r)>(file_open);
	regs.rdi.r = static_cast<decltype(regs.rcx.r)>(file_attribute);

	const bool result = kiv_os::Sys_Call(regs);
	file_handle_ret = regs.rax.r;
	return result;
}

bool kiv_os_rtl::Seek(kiv_os::THandle file_handle, const uint16_t position, kiv_os::NFile_Seek file_seek_pos, kiv_os::NFile_Seek file_seek_op, uint16_t &position_ret){
	kiv_hal::TRegisters regs = Prepare_SysCall_Context(kiv_os::NOS_Service_Major::File_System, static_cast<uint8_t>(kiv_os::NOS_File_System::Seek));
	regs.rdx.x = static_cast<decltype(regs.rdx.x)>(file_handle);
	regs.rdi.r = static_cast<decltype(regs.rdi.r)>(position);
	regs.rcx.l = static_cast<decltype(regs.rcx.l)>(file_seek_pos);
	regs.rcx.h = static_cast<decltype(regs.rcx.h)>(file_seek_op);

	const bool result = kiv_os::Sys_Call(regs);
	position_ret = regs.rax.r;
	return result;
}

bool kiv_os_rtl::Close_Handle(kiv_os::THandle handle) {
	kiv_hal::TRegisters regs = Prepare_SysCall_Context(kiv_os::NOS_Service_Major::File_System, static_cast<uint8_t>(kiv_os::NOS_File_System::Close_Handle));
	regs.rdx.x = static_cast<decltype(regs.rdx.x)>(handle);

	const bool result = kiv_os::Sys_Call(regs);
	return result;
}

bool kiv_os_rtl::Delete_File(char* file_name) {
	kiv_hal::TRegisters regs = Prepare_SysCall_Context(kiv_os::NOS_Service_Major::File_System, static_cast<uint8_t>(kiv_os::NOS_File_System::Delete_File));
	regs.rdx.r = reinterpret_cast<decltype(regs.rdx.x)>(file_name);

	const bool result = kiv_os::Sys_Call(regs);
	return result;
}

bool kiv_os_rtl::Set_File_Attribute(char* file_name, kiv_os::NFile_Attributes file_attribute) {
	kiv_hal::TRegisters regs = Prepare_SysCall_Context(kiv_os::NOS_Service_Major::File_System, static_cast<uint8_t>(kiv_os::NOS_File_System::Set_File_Attribute));
	regs.rdx.r = reinterpret_cast<decltype(regs.rdx.x)>(file_name);
	regs.rdi.r = static_cast<decltype(regs.rdi.r)>(file_attribute);

	const bool result = kiv_os::Sys_Call(regs);
	return result;
}

bool kiv_os_rtl::Get_File_Attribute(char* file_name, uint8_t &file_attribute_ret) {
	kiv_hal::TRegisters regs = Prepare_SysCall_Context(kiv_os::NOS_Service_Major::File_System, static_cast<uint8_t>(kiv_os::NOS_File_System::Get_File_Attribute));
	regs.rdx.r = reinterpret_cast<decltype(regs.rdx.x)>(file_name);

	const bool result = kiv_os::Sys_Call(regs);
	file_attribute_ret = regs.rdi.r;
	return result;
}

bool kiv_os_rtl::Create_Pipe(kiv_os::THandle *file_handles) {
	kiv_hal::TRegisters regs = Prepare_SysCall_Context(kiv_os::NOS_Service_Major::File_System, static_cast<uint8_t>(kiv_os::NOS_File_System::Create_Pipe));
	regs.rdx.r = reinterpret_cast<decltype(regs.rdx.x)>(file_handles);

	const bool result = kiv_os::Sys_Call(regs);
	return result;
}

bool kiv_os_rtl::Create_Process(char* process_name, char* argument, kiv_os::THandle stdin_handle, kiv_os::THandle stdout_handle, kiv_os::THandle &process_handle_ret) {
	return 0;

}

bool kiv_os_rtl::Create_Thread(char* thread_name, char* argument, kiv_os::THandle stdin_handle, kiv_os::THandle stdout_handle, kiv_os::THandle& thread_handle_ret) {
	return 0;

}

bool kiv_os_rtl::Wait_For(kiv_os::THandle* handles_to_wait, uint16_t num_of_handles, kiv_os::THandle &handle_signal_ret) {
	return 0;

}

bool kiv_os_rtl::Read_Exit_Code(kiv_os::THandle process_handle, uint16_t &exit_code_ret) {
	return 0;

}

bool kiv_os_rtl::Exit(uint16_t exit_code) {
	return 0;

}

bool kiv_os_rtl::Shutdown() {
	return 0;

}

bool kiv_os_rtl::Register_Signal_Handler(kiv_os::NSignal_Id signal_id, kiv_os::THandle process_handle) {
	return 0;

}