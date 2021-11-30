#pragma once
#pragma once

#include "sort.h"
#include <vector>
#include <string>
#include <queue>
#include <iostream>

size_t __stdcall type(const kiv_hal::TRegisters& regs)
{
	const kiv_os::THandle std_in = static_cast<kiv_os::THandle>(regs.rax.x);
	const kiv_os::THandle std_out = static_cast<kiv_os::THandle>(regs.rbx.x);
	const char* path = reinterpret_cast<const char*>(regs.rdi.r);
	kiv_os::THandle file_handle = kiv_os::Invalid_Handle;
	bool read_from_file = false;
	bool flag_continue = true;
	std::vector<std::string> lines;
	const size_t buffer_size = 256;
	char buffer[buffer_size];
	size_t counter = 0;
	std::string line = "";
	size_t written = 0;
	const char* new_line = "\n";


	if (path && strlen(path))
	{
		if (kiv_os_rtl::Open_File(path, (kiv_os::NOpen_File)0, kiv_os::NFile_Attributes::System_File, file_handle))
		{
			read_from_file = true;
		}
		else
		{
			kiv_os_rtl::Exit((uint16_t)kiv_os::NOS_Error::File_Not_Found);
			return 0;
		}

	}
	else
	{
		file_handle = std_in;
	}


	kiv_os_rtl::Write_File(std_out, new_line, strlen(new_line), written);
	while (flag_continue)
	{
		counter = 0;
		if (kiv_os_rtl::Read_File(file_handle, buffer, buffer_size, counter))
		{
			/*if (!read_from_file)
			{
				lines.push_back(line);
				line.clear();
				kiv_os_rtl::Write_File(std_out, new_line, strlen(new_line), written);
			}*/
			for (int i = 0; i < counter; i++)
			{
				if (buffer[i] == 3 || buffer[i] == 4 || buffer[i] == 5)
				{
					lines.push_back(line);
					line.clear();
					flag_continue = false;
					break;
				}
				else if (buffer[i] == '\n')
				{
					lines.push_back(line);
					line.clear();
				}
				else
				{
					line.push_back(buffer[i]);
				}
			}

			for (auto& line : lines)
			{
				kiv_os_rtl::Write_File(std_out, line.c_str(), strlen(line.c_str()), written);
				kiv_os_rtl::Write_File(std_out, new_line, strlen(new_line), written);
			}
			lines.clear();

		}
		else
		{
			flag_continue = false;
		}
	}



	return 0;
}
