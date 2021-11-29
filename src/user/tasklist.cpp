#pragma once

#include "tasklist.h"
#include <iostream>

size_t __stdcall tasklist(const kiv_hal::TRegisters& regs)
{
	const kiv_os::THandle std_in = static_cast<kiv_os::THandle>(regs.rax.x);
	const kiv_os::THandle std_out = static_cast<kiv_os::THandle>(regs.rbx.x);

	kiv_os::THandle file_handle = kiv_os::Invalid_Handle;

	const size_t buffer_size = sizeof(ProcessEntry);
	ProcessEntry* process;
	size_t read = 1;
	char buffer[buffer_size];

	kiv_os_rtl::Open_File("C:\\tasklist", (kiv_os::NOpen_File)0, kiv_os::NFile_Attributes::System_File, file_handle);
	while (read)
	{
		read = 0;
		if (!kiv_os_rtl::Read_File(file_handle, buffer, buffer_size, read))break;
		process = reinterpret_cast<ProcessEntry*>(buffer);
		std::cout << process->handle <<std::endl;
	}


	return 0;



}
