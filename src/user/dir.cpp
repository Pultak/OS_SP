#include "dir.h"
#include <vector>
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


	//get current directory
	if (!kiv_os_rtl::Get_Working_Dir(work_dir, 256, written))
	{
		kiv_os_rtl::Exit(kiv_os::NOS_Error::Unknown_Error);
		return 0;
	}
	working_dir = work_dir;

	//check if dir should be recursive
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
	//check if we are looking only for files
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
	//check for argument - push back into directories '.' if no argument, otherwise push argument into directories
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
			//print the current directory in which we are right now looking
			if (first_dir && empty_arg)
			{
				output.append("\nDirectory of " + working_dir + "\n\n");
				first_dir = false;
				if (!(strcmp(work_dir, "C:\\") == 0))
				{
					output.append("<DIR>\t.\n");
					output.append("<DIR>\t..\n");
				}
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
			//read current directory
			read = 1;
			while (read)
			{
				memset(buffer, 0, buffer_size);
				if (result = kiv_os_rtl::Read_File(file_handle, buffer, buffer_size, read)) //read a single directory/file
				{
					if (buffer_size == read) // check if we read the whole file/directory
					{
						kiv_os::TDir_Entry* file = reinterpret_cast<kiv_os::TDir_Entry*>(buffer);

						auto fNameLen = strlen(file->file_name);
						auto len = fNameLen < sizeof(kiv_os::TDir_Entry::file_name) ? fNameLen : sizeof(kiv_os::TDir_Entry::file_name);

						//check if it's a directory
						if (file->file_attributes == static_cast<uint16_t>(kiv_os::NFile_Attributes::Directory))
						{
							if (recursive_flag) //add directory to directories -> to trigger another iteration
							{
								whole_path.append(directories.at(0));
								whole_path.append("\\");
								whole_path.append(file->file_name, len);
								directories.push_back(whole_path);
								whole_path.clear();
							}
							if (!files_only_flag) //add directory to output if we don't want to print only files
							{
								output.append("<DIR>\t");
								output.append(file->file_name, len);
								output.append("\n");
								count_dirs++;
							}
						}
						else //or if it's a file -> add to output
						{
							output.append("\t");
							output.append(file->file_name, len);
							output.append("\n");
							count_files++;
						}
					}
					else //if (buffer_size == read)
					{
						kiv_os_rtl::Close_Handle(file_handle);
						kiv_os_rtl::Exit(kiv_os::NOS_Error::IO_Error);
						return 0;
					}
				} // if (result = kiv_os_rtl::Read_File(file_handle, buffer, buffer_size, read))
				else
				{
					read = 0;
				}
			}
			kiv_os_rtl::Close_Handle(file_handle);
		}
		else //kiv_os_rtl::Open_File() failed
		{
			kiv_os_rtl::Exit(kiv_os::NOS_Error::File_Not_Found);
			return 0;
		}

		auto it = directories.begin();
		directories.erase(it);
	}

	//add number of files and dirs to output
	output.append("\n");
	output.append(std::to_string(count_files));
	output.append(" File(s)\n");

	output.append(std::to_string(count_dirs));
	output.append(" Dir(s)");

	//write output to file
	if (!kiv_os_rtl::Write_File(std_out, output.data(), output.size(), written))
	{
		kiv_os_rtl::Exit(kiv_os::NOS_Error::IO_Error);
	}

	return 0;
}