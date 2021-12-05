#pragma once

#include "sort.h"
#include <vector>
#include <string>
#include <algorithm>

extern "C" size_t __stdcall sort(const kiv_hal::TRegisters& regs)
{
	const kiv_os::THandle std_in = static_cast<kiv_os::THandle>(regs.rax.x);
	const kiv_os::THandle std_out = static_cast<kiv_os::THandle>(regs.rbx.x);
	const char* path = reinterpret_cast<const char*>(regs.rdi.r);
	kiv_os::THandle file_handle = kiv_os::Invalid_Handle;
	bool read_from_file = false;
	bool flag_continue = true;
	std::vector<std::string> files;
	const size_t buffer_size = 256;
	char buffer[buffer_size];
	size_t counter = 0;
	std::string file = "";
	size_t written = 0;
	const char* new_line = "\n";

	//check if we should read from input or file
	if (path && strlen(path))
	{
		if (kiv_os_rtl::Open_File(path, kiv_os::NOpen_File::fmOpen_Always, kiv_os::NFile_Attributes::System_File, file_handle))
		{
			read_from_file = true;
		}
		else
		{
			kiv_os_rtl::Exit(kiv_os::NOS_Error::File_Not_Found);
			return 0;
		}

	}
	else
	{
		file_handle = std_in;
	}

	//continue until EOT/ETX
	while (flag_continue)
	{
		counter = 0;
		if (kiv_os_rtl::Read_File(file_handle, buffer, buffer_size, counter))
		{
			//add chars to file, until EOT/ETX or newline or new read and then add the file to files vector
			if (!read_from_file)
			{
				files.push_back(file);
				file.clear();
			}
			for (int i = 0; i < counter; i++)
			{
				if (buffer[i] == static_cast<char>(kiv_hal::NControl_Codes::EOT) || buffer[i] == static_cast<char>(kiv_hal::NControl_Codes::ETX))
				{
					files.push_back(file);
					file.clear();
					flag_continue = false;
					break;
				}
				else if (buffer[i] == '\n')
				{
					files.push_back(file);
					file.clear();
				}
				else
				{
					file.push_back(buffer[i]);
				}
			}
		}
		else
		{
			break;
		}
	}

	//sort files
	std::sort(files.begin(), files.end());

	//write files to output
	for (auto& line : files)
	{
		if (!kiv_os_rtl::Write_File(std_out, line.c_str(), strlen(line.c_str()), written))
		{
			kiv_os_rtl::Exit(kiv_os::NOS_Error::IO_Error);
			return 0;
		}
		kiv_os_rtl::Write_File(std_out, new_line, strlen(new_line), written);
	}

	return 0;
}
