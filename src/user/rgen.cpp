#pragma once

#include "rgen.h"
#include <ctime>
#include <string>

bool continue_flag = false;
kiv_os::THandle std_in;

extern "C"  size_t __stdcall stdin_read(const kiv_hal::TRegisters& regs)
{
	const size_t buffer_size = 10;
	char buffer[buffer_size];
	size_t counter = 0;

	bool flag_continue = true;

	//read input until EOT/ETX
	while (flag_continue)
	{
		if (kiv_os_rtl::Read_File(std_in, buffer, buffer_size, counter))
		{
			for (int i = 0; i < counter; i++)
			{
				if (buffer[i] == (uint8_t)kiv_hal::NControl_Codes::ETX || buffer[i] == (uint8_t)kiv_hal::NControl_Codes::EOT)
				{
					flag_continue = false;
					break;
				}
			}
		}
		else
		{
			//break if read returns 0
			kiv_os_rtl::Exit(kiv_os::NOS_Error::IO_Error);
			flag_continue = false;
			break;
		}
	}

	continue_flag = false;
	return 0;
}

extern "C" size_t __stdcall rgen(const kiv_hal::TRegisters& regs)
{
	std_in = static_cast<kiv_os::THandle>(regs.rax.x);
	const kiv_os::THandle std_out = static_cast<kiv_os::THandle>(regs.rbx.x);
	std::string rand_num;
	size_t written;
	continue_flag = true;
	kiv_os::THandle stdin_thread;

	//seed rand
	srand(static_cast <unsigned> (time(0)));

	//create thread
	kiv_os_rtl::Create_Thread(stdin_read, reinterpret_cast<const char*>(continue_flag), std_in, std_out, stdin_thread);

	float a = 5.0;

	//print random floats until thread sets flag to 0
	while (continue_flag)
	{
		rand_num = std::to_string(float(rand()) / float((RAND_MAX)) * a);
		if (!kiv_os_rtl::Write_File(std_out, rand_num.c_str(), strlen(rand_num.c_str()), written))
		{
			kiv_os_rtl::Exit(kiv_os::NOS_Error::IO_Error);
			break;
		}
		kiv_os_rtl::Write_File(std_out, "\n", strlen("\n"), written);
	}

	//close the handle for thread
	kiv_os_rtl::Close_Handle(stdin_thread);

	return 0;
}