#include "dir.h"
#include <iostream>
#include <vector>
/*
size_t __stdcall dir(const kiv_hal::TRegisters& regs)
{
	const kiv_os::THandle std_out = static_cast<kiv_os::THandle>(regs.rbx.x);
	const char* argument = reinterpret_cast<const char*>(regs.rdi.r);
	std::string argument_s = argument;
	std::vector<std::string> directories;
	std::vector<std::string> lines;
	std::string line = "";
	bool recursive_flag = false;
	bool empty_arg = false;
	kiv_os::THandle file_handle = kiv_os::Invalid_Handle;
	const size_t buffer_size = 256;
	char buffer[buffer_size];
	size_t read = 0;
	size_t written = 0;
	const char* new_line = "\n";


	if (strlen(argument_s.c_str()) >= strlen("/S"))
	{
		size_t pos = argument_s.find("/S");

		if (pos == 0)
		{
			recursive_flag = true;
			argument += strlen("/S");
			while (*argument == ' ')
			{
				argument++;
			}
			argument_s = argument;
		}
	}
	if (argument_s.empty())
	{
		empty_arg = true;
	}
	
	directories.push_back(".");

	while (!directories.empty())
	{
		if (auto result = kiv_os_rtl::Open_File(directories.at(0).c_str(), kiv_os::NOpen_File::fmOpen_Always, kiv_os::NFile_Attributes::Directory, file_handle))
		{
			if (result = kiv_os_rtl::Read_File(file_handle, buffer, buffer_size, read))
			{
				for (int i = 0; i < read; i++)
				{
					if (buffer[i] == 3 || buffer[i] == 4 || buffer[i] == 5)
					{
						lines.push_back(line);
						line.clear();
						break;
					}
					else if (buffer[i] == '\n')
					{
						lines.push_back(line);
						kiv_os_rtl::Write_File(std_out, line.c_str(), strlen(line.c_str()), written);
						kiv_os_rtl::Write_File(std_out, new_line, strlen(new_line), written);
						line.clear();
					}
					else
					{
						line.push_back(buffer[i]);
					}
				}
			}
			else
			{
				std::cout << "\nfailed to read\n";
			}
		}
		else
		{
			std::cout << "\nfailed to open\n";
		}

	}

	return 0;
}
*/

size_t __stdcall dir(const kiv_hal::TRegisters& regs)
{
	const kiv_os::THandle std_in = static_cast<kiv_os::THandle>(regs.rax.x);
	const kiv_os::THandle std_out = static_cast<kiv_os::THandle>(regs.rbx.x);

	const char* file_to_open = reinterpret_cast<const char*>(regs.rdi.r);

	size_t read;

	kiv_os::THandle file_handle = kiv_os::Invalid_Handle;

	char buffer[256];

	if (file_to_open && strlen(file_to_open))
	{
		//auto ret_code = kiv_os_rtl::Delete_File(file_to_open);

		uint8_t atr = 0;
		if (auto ret_code = kiv_os_rtl::Open_File(file_to_open, static_cast<kiv_os::NOpen_File>(atr), kiv_os::NFile_Attributes::System_File, file_handle))
		{
			std::cout << "\nOPENED\n";
		}
		else
		{
			std::cout << "\nOPEN FAILED: " << ret_code << std::endl;
		}
		
		if (auto ret_code = kiv_os_rtl::Write_File(file_handle, "HAHAHAHHAHAH", strlen("HAHAHAHHAHAH"), read))
		{
			std::cout << "\WRITE\n";
		}
		else
		{
			std::cout << "\nWRITE FAILED: " << ret_code << std::endl;
		}
		
		if (auto ret_code = kiv_os_rtl::Read_File(file_handle, buffer, 256, read))
		{
			//for (int i = 0; i < read; i++) {
			//	printf("%c", buffer[i]);
			//}
			std::cout << "file handle: " << file_handle;

			std::cout << buffer <<"\n";
		}
		else
		{
			std::cout << "\nREAD FAILED: " << ret_code << std::endl;
		}


	}
	return 0;
}