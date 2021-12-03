#pragma once

#include "tasklist.h"
#include <iostream>
#include <string>

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


	size_t counter = 0;

	kiv_os_rtl::Write_File(std_out, "Program name\tInput\tOutput\tState\tCode\tWorking dir\n", 49, counter);

	while (read)
	{
		read = 0;
		if (!kiv_os_rtl::Read_File(file_handle, buffer, buffer_size, read))break;
		process = reinterpret_cast<ProcessEntry*>(buffer);
		auto pNameLength = strlen(process->programName);
		if(!kiv_os_rtl::Write_File(std_out, process->programName, pNameLength, counter))break;
		for(int i = 0; i < 2 - (pNameLength / 8); ++i)
			kiv_os_rtl::Write_File(std_out, "\t", 1, counter); 
		auto stdInString = std::to_string(process->stdIn);
		kiv_os_rtl::Write_File(std_out, stdInString.c_str(), stdInString.length(), counter);
		kiv_os_rtl::Write_File(std_out, "\t", 1, counter);

		auto stdOutString = std::to_string(process->stdOut);
		kiv_os_rtl::Write_File(std_out, stdOutString.c_str(), stdOutString.length(), counter);
		kiv_os_rtl::Write_File(std_out, "\t", 1, counter);

		auto stateString = std::to_string((uint8_t)process->state);
		kiv_os_rtl::Write_File(std_out, stateString.c_str(), stateString.length(), counter);
		kiv_os_rtl::Write_File(std_out, "\t", 1, counter);

		auto exitString = std::to_string((uint8_t)process->exitCode);
		kiv_os_rtl::Write_File(std_out, exitString.c_str(), exitString.length(), counter);
		kiv_os_rtl::Write_File(std_out, "\t", 1, counter);
		kiv_os_rtl::Write_File(std_out, process->workingDir, strlen(process->workingDir), counter);

		kiv_os_rtl::Write_File(std_out, "\n", 1, counter);

	}


	return 0;



}
