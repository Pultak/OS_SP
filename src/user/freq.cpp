#pragma once

#include "freq.h"
#include <sstream>
#include <string> 
#include <array>
#include <iostream>

void freq_prot(const kiv_hal::TRegisters& regs)
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

	while (flag_continue)
	{
		if (kiv_os_rtl::Read_File(std_in, buffer, buffer_size, counter))
		{
			for (int i = 0; i < counter; i++)
			{
				char c = buffer[i];
				if (c == 'q')
				{
					flag_continue = false;
				}
				if (c >= 0)
				{
					chars.at(c)++;
				}
			}
		}
	}

	for (int i = 0; i < 128; i++) {
		int ch = chars.at(i);
		char c = i;
		if (ch)
		{
			line_ss.str(std::string());
			line_ss << "char: " << c << " " << ch;
			line = line_ss.str();

			kiv_os_rtl::Write_File(std_out, new_line, strlen(new_line), counter);
			kiv_os_rtl::Write_File(std_out, line.c_str(), strlen(line.c_str()), counter);
			kiv_os_rtl::Write_File(std_out, new_line, strlen(new_line), counter);
		}
		
	}

}