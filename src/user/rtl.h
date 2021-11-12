#pragma once

#include "..\api\api.h"
#include <atomic>

namespace kiv_os_rtl {

	extern std::atomic<kiv_os::NOS_Error> Last_Error;

	bool Read_File(const kiv_os::THandle file_handle, char* const buffer, const size_t buffer_size, size_t &read);
		//zapise do souboru identifikovaneho deskriptor data z buffer o velikosti buffer_size a vrati pocet zapsanych dat ve written
		//vraci true, kdyz vse OK
		//vraci true, kdyz vse OK

	bool Write_File(const kiv_os::THandle file_handle, const char *buffer, const size_t buffer_size, size_t &written);
	//zapise do souboru identifikovaneho deskriptor data z buffer o velikosti buffer_size a vrati pocet zapsanych dat ve written
	//vraci true, kdyz vse OK
	//vraci true, kdyz vse OK

	bool Set_Working_Dir(const char* path);

	bool Get_Working_Dir(const char* buffer, size_t buffer_size, size_t& read);

	bool md(const kiv_hal::TRegisters& regs);
	
	bool Open_Filesystem(const char* file_name, kiv_os::NOpen_File flags, uint8_t attributes, kiv_os::THandle& handle, kiv_os::NOS_Error& error);

}