#include "global.h"
#include "echo.h"
#include <iostream>

bool echo_on = true;

size_t __stdcall echo(const kiv_hal::TRegisters& regs) {
	const kiv_os::THandle std_out = static_cast<kiv_os::THandle>(regs.rbx.x);

	size_t counter = 0;
	const char* new_line = "\n";

	const char* test = "TEST ECHO PROCESS - regs";



	kiv_os_rtl::Write_File(std_out, new_line, strlen(new_line), counter);
	kiv_os_rtl::Write_File(std_out, test, strlen(test), counter);
	kiv_os_rtl::Write_File(std_out, new_line, strlen(new_line), counter);


	//std::cout << "TEST ECHO PROCESS\n";
	return 0;
}

void echo_prot(char* input)
{
	if (input[0] == 0)
	{
		if (echo_on)
		{
			std::cout << "\nECHO is on.\n";
		}
		else
		{
			std::cout << "\nECHO is off.\n";
		}
	}
	if ((input[0] == 'o' || input[0] == 'O') && (input[1] == 'n' || input[1] == 'N') && (input[2] == 0))
	{
		std::cout << "ECHO turning on.\n";

		echo_on = true;
	}
	else if ((input[0] == 'o' || input[0] == 'O') && (input[1] == 'f' || input[1] == 'F') && (input[2] == 'f' || input[2] == 'F') && (input[3] == 0))
	{
		std::cout << "ECHO turning off.\n";

		echo_on = false;
	}
	else
	{
		std::cout << input << "\n";
	}
}