#pragma once

#include "md.h"
#include <iostream>

size_t __stdcall md(const kiv_hal::TRegisters& regs)
{
	const kiv_os::THandle std_in = static_cast<kiv_os::THandle>(regs.rax.x);
	const kiv_os::THandle std_out = static_cast<kiv_os::THandle>(regs.rbx.x);

	const char* file_to_open = reinterpret_cast<const char*>(regs.rdi.r);

	kiv_os::NOpen_File open_file_flag = static_cast<kiv_os::NOpen_File>(0);

	kiv_os::THandle file_handle = kiv_os::Invalid_Handle;

	if (file_to_open && strlen(file_to_open))
	{		
		if (auto ret_code = kiv_os_rtl::Open_File(file_to_open, open_file_flag, kiv_os::NFile_Attributes::Directory, file_handle))
		{
			std::cout << "\nOPENED\n";
		}
		else
		{
			std::cout << "\nOPEN FAILED: " << ret_code << std::endl;
		}
		
	}
	return 0;
}
