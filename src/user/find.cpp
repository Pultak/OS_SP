#pragma once

#include "find.h"
#include <vector>
#include <string>
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

	//check if find is with the correct parameters
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
		kiv_os_rtl::Exit(kiv_os::NOS_Error::Invalid_Argument);
		return 0;
	}

	//open file if there is additional argument after /c /v""
	if (path_c && strlen(path_c))
	{
		if (kiv_os_rtl::Open_File(path_c, kiv_os::NOpen_File::fmOpen_Always, kiv_os::NFile_Attributes::System_File, file_handle))
		{
			read_from_file = true;
		}
		else
		{
			kiv_os_rtl::Exit(kiv_os::NOS_Error::File_Not_Found);
			return 0;
		}
	}
	//else read from in handle
	else
	{
		file_handle = std_in;
	}

	//iterate through input until we read EOT/ETX or read returns 0
	while (flag_continue)
	{
		if (kiv_os_rtl::Read_File(file_handle, buffer, buffer_size, counter))
		{
			//add line for each read
			line_count++;

			for (int i = 0; i < counter; i++)
			{
				if (buffer[i] == static_cast<char>(kiv_hal::NControl_Codes::EOT) || buffer[i] == static_cast<char>(kiv_hal::NControl_Codes::ETX))
				{
					flag_continue = false;
					break;
				}
				else if (buffer[i] == '\n')
				{
					//add additional line if there is newline inside file
					line_count++;
				}
			}
		}
		else
		{
			flag_continue = false;
		}
	}

	//write line count to output
	std::string line = "";
	line.append(std::to_string(line_count));
	line.append(" ");
	if (!kiv_os_rtl::Write_File(std_out, line.c_str(), strlen(line.c_str()), written))
	{
		kiv_os_rtl::Exit(kiv_os::NOS_Error::IO_Error);
		return 0;
	}
	
	kiv_os_rtl::Write_File(std_out, new_line, strlen(new_line), written);

	//close file handle if we read from file
	if (read_from_file)
	{
		kiv_os_rtl::Close_Handle(file_handle);
	}

	return 0;
}
