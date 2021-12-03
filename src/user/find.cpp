#pragma once

#include "find.h"
#include <vector>
#include <string>
#include <iostream>
#include "command_parser.h"

size_t __stdcall find(const kiv_hal::TRegisters& regs)
{
	const kiv_os::THandle std_in = static_cast<kiv_os::THandle>(regs.rax.x);
	const kiv_os::THandle std_out = static_cast<kiv_os::THandle>(regs.rbx.x);
	const char* path_c = reinterpret_cast<char*>(regs.rdi.r);
	kiv_os::THandle file_handle = kiv_os::Invalid_Handle;
	bool read_from_file = false;
	bool flag_continue = true;
	const size_t buffer_size = 256;
	char buffer[buffer_size];
	size_t counter = 0;
	size_t written = 0;
	const char* new_line = "\n";
	uint16_t line_count = 0;
	std::string path_s = path_c;
	std::string parameter_check = "/c /v\"\"";
	bool flag_invalid_line = false;

	if (strlen(path_s.c_str()) >= parameter_check.size())
	{
		size_t pos = path_s.find(parameter_check);

		if (pos == 0)
		{
			path_c += parameter_check.size();
			while (*path_c == ' ')
			{
				path_c++;
			}
		}
	}
	else
	{
		kiv_os_rtl::Exit((uint16_t)kiv_os::NOS_Error::Invalid_Argument);
		return 0;
	}

	if (path_c && strlen(path_c))
	{
		if (kiv_os_rtl::Open_File(path_c, kiv_os::NOpen_File::fmOpen_Always, kiv_os::NFile_Attributes::System_File, file_handle))
		{
			read_from_file = true;
		}
		else
		{
			return kiv_os_rtl::Exit((uint16_t)kiv_os::NOS_Error::File_Not_Found);
		}
	}
	else
	{
		file_handle = std_in;
	}

	while (flag_continue)
	{
		if (kiv_os_rtl::Read_File(file_handle, buffer, buffer_size, counter))
		{
			line_count++;

			for (int i = 0; i < counter; i++)
			{
				if (buffer[i] == 3 || buffer[i] == 4)
				{
					flag_continue = false;
					break;
				}
				else if (buffer[i] == '\n')
				{
					line_count++;
				}
				else if (buffer[i] == -51)
				{
					flag_invalid_line = true;
				}
			}
			if (flag_invalid_line)
			{
				line_count--;
			}
		}
		else
		{
			flag_continue = false;
		}
	}

	std::string line = "";
	line.append(std::to_string(line_count));
	line.append(" ");
	kiv_os_rtl::Write_File(std_out, line.c_str(), strlen(line.c_str()), written);
	kiv_os_rtl::Write_File(std_out, new_line, strlen(new_line), written);

	return 0;
}
