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
	char buffer[buffer_size];
	char directory[buffer_size];
	size_t counter;
	std::vector<Program> program_vector;
	
	const char* intro = "Vitejte v kostre semestralni prace z KIV/OS.\n" \
						"Shell zobrazuje echo zadaneho retezce. Prikaz exit ukonci shell.\n";
	kiv_os_rtl::Write_File(std_out, intro, strlen(intro), counter);


	const char* prompt = "C:\\>";
	size_t chars_written;
	do {
		kiv_os_rtl::Write_File(std_out, prompt, strlen(prompt), counter);

		if (kiv_os_rtl::Read_File(std_in, buffer, buffer_size, counter)) {
			if ((counter > 0) && (counter == buffer_size)) counter--;
			buffer[counter] = 0;	//udelame z precteneho vstup null-terminated retezec

			program_vector = ProcessLine(buffer);
			for (auto it = program_vector.begin(); it != program_vector.end(); it++)
			{
				if (strcmp(it->command.c_str(), "echo") == 0)
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
							const char* print = "\nECHO is on\n";
							kiv_os_rtl::Write_File(std_out, print, strlen(print), counter);
						}
						else
						{
							
							const char* print = "\nECHO is off.\n";
							kiv_os_rtl::Write_File(std_out, print, strlen(print), counter);
						}
					}
				}
				else if (strcmp(it->command.c_str(), "cd") == 0)
				{
					if (!it->argument.empty())
					{
						if (kiv_os_rtl::Set_Working_Dir(it->argument.c_str()))
						{
							if (kiv_os_rtl::Get_Working_Dir(directory, buffer_size, chars_written))
							{

							}
							else
							{
								const char* print = "\nCouldn't read working directory\n";
								kiv_os_rtl::Write_File(std_out, print, strlen(print), counter);
							}
						}
						else
						{
							const char* print = "\nDirectory not found\n";
							kiv_os_rtl::Write_File(std_out, print, strlen(print), counter);
						}
					}
					else
					{
						const char* print = "\nDirectory not specified\n";
						kiv_os_rtl::Write_File(std_out, print, strlen(print), counter);

					}
				}
			}
			if (!program_vector.empty()) {
				Execute_Commands(program_vector, regs);
			}

			const char* new_line = "\n";
			kiv_os_rtl::Write_File(std_out, new_line, strlen(new_line), counter);

			if (echo_on)
			{
				kiv_os_rtl::Write_File(std_out, buffer, strlen(buffer), counter);	//a vypiseme ho
				kiv_os_rtl::Write_File(std_out, new_line, strlen(new_line), counter);
			}
		}
		else
			break;	//EOF
	} while (strcmp(buffer, "exit") != 0);

	

	return 0;	
}