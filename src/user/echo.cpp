#include "global.h"
#include "echo.h"
#include <iostream>

bool echo_on = true;

size_t __stdcall echo(const kiv_hal::TRegisters& regs) {
	std::cout << "TEST ECHO PROCESS";
	return 0;
}

void echo_prot(char* input)
{
	if (input[0] == 0)
	{
		if (echo_on)
		{
			std::cout << "ECHO is on.\n";
		}
		else
		{
			std::cout << "ECHO is off.\n";
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