#pragma once

#include "md.h"

size_t __stdcall md(const kiv_hal::TRegisters& regs)
{
	const kiv_os::THandle std_in = static_cast<kiv_os::THandle>(regs.rax.x);
	const kiv_os::THandle std_out = static_cast<kiv_os::THandle>(regs.rbx.x);

	const char* file_to_open = reinterpret_cast<const char*>(regs.rdi.r);

	kiv_os::NOpen_File open_file_flag = static_cast<kiv_os::NOpen_File>(0);

	kiv_os::THandle file_handle = kiv_os::Invalid_Handle;

	if (file_to_open && strlen(file_to_open))
	{		
		if (kiv_os_rtl::Open_File(file_to_open, open_file_flag, kiv_os::NFile_Attributes::Directory, file_handle))
		{
			//once file opened - close handle
			kiv_os_rtl::Close_Handle(file_handle);
		}
		else
		{
			kiv_os_rtl::Exit(kiv_os::NOS_Error::Unknown_Error);
		}		
	}
	else
	{
		kiv_os_rtl::Exit(kiv_os::NOS_Error::Invalid_Argument);
	}

	return 0;
}
