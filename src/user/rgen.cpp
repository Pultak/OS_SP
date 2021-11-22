#pragma once

#include "rgen.h"
#include <ctime>
#include <iostream>

bool eof = false;

size_t stdin_read(const kiv_hal::TRegisters& regs)
{
	const kiv_os::THandle std_in = static_cast<kiv_os::THandle>(regs.rax.x);

	const size_t buffer_size = 256;
	char buffer[buffer_size];
	size_t counter;

	bool flag_continue = true;

	while (flag_continue)
	{
		if (kiv_os_rtl::Read_File(std_in, buffer, buffer_size, counter))
		{
			for (int i = 0; i < counter; i++)
			{
				if (buffer[i] == 3 || buffer[i] == 4 || buffer[i] == 5)
				{
					flag_continue = false;
					break;
				}
			}
		}
	}

	eof = true;
	return 0;
}

extern "C" size_t __stdcall rgen(const kiv_hal::TRegisters& regs)
{
	const kiv_os::THandle std_in = static_cast<kiv_os::THandle>(regs.rax.x);
	const kiv_os::THandle std_out = static_cast<kiv_os::THandle>(regs.rbx.x);

	srand(static_cast <unsigned> (time(0)));

	eof = false;
	kiv_os::THandle stdin_thread;
	kiv_os_rtl::Create_Thread(&stdin_read, &eof, std_in, std_out, stdin_thread);

	float a = 5.0;

	int counter = 0;
	while (!eof && counter < 50)
	{
		std::cout << (float(rand()) / float((RAND_MAX)) * a) << std::endl;
		counter++;
	}

	return 0;
}