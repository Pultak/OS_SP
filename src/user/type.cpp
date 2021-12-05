#pragma once
#pragma once

#include "type.h"
#include "rtl.h"
#include <vector>
#include <string>

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

	//check if we should read from file or from input
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

	kiv_os_rtl::Write_File(std_out, new_line, strlen(new_line), written);

	//read until EOT/ETX or read returns 0
	while (flag_continue)
	{
		counter = 0;
		if (kiv_os_rtl::Read_File(file_handle, buffer, buffer_size, counter))
		{
			//add chars to line until EOT/ETX or newline -> when we read the whole buffer write each line to output
			for (int i = 0; i < counter; i++)
			{
				if (buffer[i] == static_cast<char>(kiv_hal::NControl_Codes::EOT) || buffer[i] == static_cast<char>(kiv_hal::NControl_Codes::ETX))
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

			for (auto& iline : lines)
			{
				kiv_os_rtl::Write_File(std_out, iline.c_str(), strlen(iline.c_str()), written);
				kiv_os_rtl::Write_File(std_out, new_line, strlen(new_line), written);
			}
			lines.clear();

		}
		else
		{
			flag_continue = false;
		}
	}

	//close file handle if we read from file
	if (read_from_file)
	{
		kiv_os_rtl::Close_Handle(file_handle);
	}

	return 0;
}
