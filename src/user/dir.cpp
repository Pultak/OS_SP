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
	bool first_dir = true;
	bool empty_arg = false;
	bool files_only_flag = false;
	kiv_os::THandle file_handle = kiv_os::Invalid_Handle;
	const size_t buffer_size = sizeof(kiv_os::TDir_Entry);
	char buffer[buffer_size];
	size_t read = 0;
	size_t written = 0;
	const char* new_line = "\n";
	std::string output = "";
	uint16_t count_files = 0;
	uint16_t count_dirs = 0;
	std::string whole_path = "";
	std::string working_dir = "";
	char work_dir[256];

	if (!kiv_os_rtl::Get_Working_Dir(work_dir, 256, written))
	{
		kiv_os_rtl::Exit(kiv_os::NOS_Error::Unknown_Error);
		return 0;
	}
	working_dir = work_dir;

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
	if (strlen(argument_s.c_str()) >= strlen("\\*.*"))
	{
		size_t pos = argument_s.find("\\*.*");

		if (pos == 0)
		{
			files_only_flag = true;
			argument += strlen("\\*.*");
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
		if (auto result = kiv_os_rtl::Open_File(directories.at(0).c_str(), kiv_os::NOpen_File::fmOpen_Always, kiv_os::NFile_Attributes::Directory, file_handle))
		{
			if ((strcmp(work_dir, "C:\\") == 0) && first_dir && empty_arg)
			{
				output.append("\nDirectory of " + working_dir + "\n\n");
				first_dir = false;
			}
			else if (empty_arg)
			{
				std::string temp = directories.at(0).substr(2, directories.at(0).size() - 2);
				output.append("\nDirectory of " + working_dir + temp + "\n\n");
				output.append("<DIR>\t.\n");
				output.append("<DIR>\t..\n");
			}
			else
			{
				output.append("\nDirectory of " + working_dir + directories.at(0) + "\n\n");
				output.append("<DIR>\t.\n");
				output.append("<DIR>\t..\n");
			}
			read = 1;
			while (read)
			{
				memset(buffer, 0, buffer_size);
				if (result = kiv_os_rtl::Read_File(file_handle, buffer, buffer_size, read))
				{
					if (buffer_size == read)
					{
						kiv_os::TDir_Entry* file = reinterpret_cast<kiv_os::TDir_Entry*>(buffer);

						auto fNameLen = strlen(file->file_name);
						auto len = fNameLen < sizeof(kiv_os::TDir_Entry::file_name) ? fNameLen : sizeof(kiv_os::TDir_Entry::file_name);

						if (file->file_attributes == static_cast<uint16_t>(kiv_os::NFile_Attributes::Directory))
						{
							if (recursive_flag)
							{
								whole_path.append(directories.at(0));
								whole_path.append("\\");
								whole_path.append(file->file_name, len);
								directories.push_back(whole_path);
								whole_path.clear();
							}
							if (!files_only_flag)
							{
								output.append("<DIR>\t");
								output.append(file->file_name, len);
								output.append("\n");
								count_dirs++;
							}
						}
						else
						{
							output.append("\t");
							output.append(file->file_name, len);
							output.append("\n");
							count_files++;
						}
					}
					else
					{
						kiv_os_rtl::Exit(kiv_os::NOS_Error::IO_Error);
						return 0;
					}
				}
				else
				{
					read = 0;
				}
			}
		}
		else
		{
			kiv_os_rtl::Exit(kiv_os::NOS_Error::File_Not_Found);
			return 0;
		}

		auto it = directories.begin();
		directories.erase(it);
	}

	output.append("\n");
	output.append(std::to_string(count_files));
	output.append(" File(s)\n");

	output.append(std::to_string(count_dirs));
	output.append(" Dir(s)");

	kiv_os_rtl::Write_File(std_out, output.data(), output.size(), written);

	return 0;
}