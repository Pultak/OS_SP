#include "shell.h"
#include "rtl.h"
#include "command_parser.h"
#include "global.h"
#include "freq.h"
#include <iostream>

size_t __stdcall shell(const kiv_hal::TRegisters &regs) {

	const kiv_os::THandle std_in = static_cast<kiv_os::THandle>(regs.rax.x);
	const kiv_os::THandle std_out = static_cast<kiv_os::THandle>(regs.rbx.x);
	bool echo_on = true;

	const size_t buffer_size = 256;
	char buffer[buffer_size + 1];
	buffer[buffer_size] = 0;
	char directory[buffer_size];
	const char* new_line = "\n";
	size_t counter;
	std::vector<Program> program_vector;
	bool continue_flag = true;
	size_t chars_written;

	
	const char* intro = "Vitejte v semestralni praci z KIV/OS.\n";

	if (!kiv_os_rtl::Write_File(std_out, intro, strlen(intro), counter))
	{
		kiv_os_rtl::Exit(kiv_os::NOS_Error::IO_Error);
		return 0;
	}

	if (!kiv_os_rtl::Get_Working_Dir(directory, buffer_size, chars_written))
	{
		kiv_os_rtl::Exit(kiv_os::NOS_Error::Unknown_Error);
		return 0;
	}

	const char* prompt = "C:\\>";
	do {
		if (echo_on)
		{
			if (!kiv_os_rtl::Write_File(std_out, directory, strlen(directory), counter))
			{
				kiv_os_rtl::Exit(kiv_os::NOS_Error::IO_Error);
				return 0;
			}

			if (strcmp(directory, "C:\\") == 0)
			{
				kiv_os_rtl::Write_File(std_out, ">", 1, counter);
			}
			else
			{
				kiv_os_rtl::Write_File(std_out, "\\>", 2, counter);
			}
		}




		if (kiv_os_rtl::Read_File(std_in, buffer, buffer_size, counter)) {
			buffer[counter] = 0;	//udelame z precteneho vstup null-terminated retezec

			if (buffer[counter - 1] == static_cast<uint8_t>(kiv_hal::NControl_Codes::ETX))
			{
				kiv_os_rtl::Write_File(std_out, new_line, strlen(new_line), counter);
				continue;
			}

			program_vector = ProcessLine(buffer);
			for (auto it = program_vector.begin(); it != program_vector.end(); it++)
			{
				if (strcmp(it->command.c_str(), "exit") == 0) {
					continue_flag = false;
					break;
				}
				else if (strcmp(it->command.c_str(), "echo") == 0)
				{
					if (strcmp(it->argument.c_str(), "on") == 0)
					{
						echo_on = true;
					}
					else if (strcmp(it->argument.c_str(), "off") == 0)
					{
						echo_on = false;
					}
					if (strcmp(it->argument.c_str(), "") == 0)
					{
						if (echo_on)
						{
							const char* print = "ECHO is on.\n";
							it->argument=print;
						}
						else
						{
							const char* print = "ECHO is off.\n";
							it->argument = print;
						}
					}
				}
				else if (strcmp(it->command.c_str(), "cd") == 0)
				{
					if (!it->argument.empty())
					{
						if (kiv_os_rtl::Set_Working_Dir(it->argument.c_str()))
						{
							if (!kiv_os_rtl::Get_Working_Dir(directory, buffer_size, chars_written))
							{
								const char* print = "\nCouldn't read working directory";
								kiv_os_rtl::Write_File(std_out, print, strlen(print), counter);
							}
						}
						else
						{
							const char* print = "\nDirectory not found";
							kiv_os_rtl::Write_File(std_out, print, strlen(print), counter);
						}
					}
					else
					{
						const char* print = "\nDirectory not specified";
						kiv_os_rtl::Write_File(std_out, print, strlen(print), counter);

					}
				}
			}
			if (continue_flag)
			{
				if (!kiv_os_rtl::Write_File(std_out, new_line, strlen(new_line), counter))
				{
					kiv_os_rtl::Exit(kiv_os::NOS_Error::IO_Error);
					return 0;
				}

				if (!program_vector.empty()) {
					Execute_Commands(program_vector, regs);

					if (!kiv_os_rtl::Write_File(std_out, new_line, strlen(new_line), counter))
					{
						kiv_os_rtl::Exit(kiv_os::NOS_Error::IO_Error);
						return 0;
					}
				}
			}
		}
		else
			break;	//EOF
	} while (continue_flag);

	return 0;	
}