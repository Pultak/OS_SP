#include "dir.h"
#include <iostream>
#include <vector>
#include <sstream>
#include <string>

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
	const size_t buffer_size = sizeof(kiv_os::TDir_Entry);
	char buffer[buffer_size];
	size_t read = 1;
	size_t written = 0;
	const char* new_line = "\n";
	std::string output = "";
	uint16_t count_files = 0;
	uint16_t count_dirs = 0;




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
		directories.push_back(".");
		empty_arg = true;
	}
	else
	{
		directories.push_back(argument_s);
	}	

	while (!directories.empty())
	{
		auto it = directories.begin();
		if (auto result = kiv_os_rtl::Open_File(directories.at(0).c_str(), (kiv_os::NOpen_File)0, kiv_os::NFile_Attributes::Directory, file_handle))
		{
			while (read)
			{
				if (result = kiv_os_rtl::Read_File(file_handle, buffer, buffer_size, read))
				{
					std::cout << "\nbuff a read: " << buffer_size <<  " " << read ;

					if (buffer_size == read)
					{
						kiv_os::TDir_Entry* file = reinterpret_cast<kiv_os::TDir_Entry*>(buffer);
						if (file->file_attributes == static_cast<uint16_t>(kiv_os::NFile_Attributes::Directory))
						{
							if (recursive_flag)
							{
								directories.push_back(file->file_name);
							}
							output.append("<DIR>\t");
							output.append(file->file_name);
							output.append("\n");
							count_dirs++;
						}
						else
						{
							output.append("\t\t");
							output.append(file->file_name);
							output.append("\n");
							count_files++;
						}
					}
					else
					{
						std::cout << "\nwrong size of TDir_Entry\n";
						read = 0;
					}
					read = 0;
				}
				else
				{
					std::cout << "\nfailed to read\n";
					return 0;
				}
			}
		}
		else
		{
			std::cout << "\nfailed to open\n";
		}

		directories.erase(it);
	}

	output.append(std::to_string(count_files));
	output.append(" File(s)");

	output.append(std::to_string(count_dirs));
	output.append(" Dir(s)");

	kiv_os_rtl::Write_File(std_out, output.data(), output.size(), written);

	std::cout << "END";
	return 0;
}/*

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
		
	/*	if (auto ret_code = kiv_os_rtl::Write_File(file_handle, "11111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111112222222222222222222222222222222222222222222222222222222222222222222222222222222222"
			, strlen("11111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111112222222222222222222222222222222222222222222222222222222222222222222222222222222222"), read))
		{
			std::cout << "\WRITE\n";
		}
		else
		{
			std::cout << "\nWRITE FAILED: " << ret_code << std::endl;
		}
		
		if (auto ret_code = kiv_os_rtl::Read_File(file_handle, buffer, 256, read))
		{
			std::cout << "\n\nREAD:::: " << read;
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
}*/