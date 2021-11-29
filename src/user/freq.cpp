#pragma once

#include "freq.h"
#include <sstream>
#include <string> 
#include <array>
#include <iostream>

//comments missing

size_t __stdcall freq(const kiv_hal::TRegisters& regs)
{
	const kiv_os::THandle std_in = static_cast<kiv_os::THandle>(regs.rax.x);
	const kiv_os::THandle std_out = static_cast<kiv_os::THandle>(regs.rbx.x);
	std::stringstream line_ss;
	std::string line;
	const char* new_line = "\n";

	const size_t buffer_size = 256;
	char buffer[buffer_size];
	size_t counter;
	std::array<char, 128> chars = { 0 };

	bool flag_continue = true;

	std::cout << "freq : " << std_in << ".\n";

	while (flag_continue)
	{
		std::cout << "before read\n";

		if (kiv_os_rtl::Read_File(std_in, buffer, buffer_size, counter))
		{
			std::cout <<"\nbuffer" << buffer;

			for (int i = 0; i < counter; i++)
			{
				char c = buffer[i];

				if (c == 5 || c == 4 || c == 3)
				{
					flag_continue = false;
					break;
				}
				if (c >= 0)
				{
					chars.at(c)++;
				}
			}
		}
		else
		{
			break;
		}
	}
	std::cout << "read fin\n";

	for (int i = 0; i < 128; i++) {
		int ch = chars.at(i);
		char c = i;
		if (ch)
		{
			line_ss.str(std::string());
			line_ss << "0x" << std::hex << (int)c;
			line = line_ss.str();
			line.append("x : " + std::to_string(ch));

			kiv_os_rtl::Write_File(std_out, new_line, strlen(new_line), counter);
			kiv_os_rtl::Write_File(std_out, line.c_str(), strlen(line.c_str()), counter);
			kiv_os_rtl::Write_File(std_out, new_line, strlen(new_line), counter);
		}
		
	}
	return 0;
	
}