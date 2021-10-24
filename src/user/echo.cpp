#include "global.h"
#include <iostream>

bool echo_on = true;

char* echo_prot(char* input)
{
	//no input, print echo status
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
		return 0;
	}
	if (strcmp(input, "on"))
	{
		echo_on = true;
	}
	else if (strcmp(input, "off"))
	{
		echo_on = false;
	}
	else
	{
		std::cout << input << "\n";
	}
	return 0;
}