#pragma once

#include "rd.h"
#include <iostream>

size_t __stdcall rd(const kiv_hal::TRegisters& regs)
{
	const kiv_os::THandle std_in = static_cast<kiv_os::THandle>(regs.rax.x);
	const kiv_os::THandle std_out = static_cast<kiv_os::THandle>(regs.rbx.x);

	const char* file_to_delete = reinterpret_cast<const char*>(regs.rdi.r);

	kiv_os::THandle file_handle = kiv_os::Invalid_Handle;

	if (file_to_delete && strlen(file_to_delete))
	{
		if (!kiv_os_rtl::Delete_File(file_to_delete))
		{
			kiv_os_rtl::Exit(kiv_os::NOS_Error::File_Not_Found);
		}
	}
	return 0;
}
