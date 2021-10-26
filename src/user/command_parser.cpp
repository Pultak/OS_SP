#pragma once

#include "command_parser.h"
#include "echo.h"
#include <iostream>

/* Function to remove unnecessary whitespace from C string */
void RemoveWhitespace(char* line)
{
	RemoveLeadingWhitespace(line);
	RemoveTrailingWhitespace(line);
}

/* Function to remove leading spaces from a command/line */
void RemoveLeadingWhitespace(char* line) //MAKE STATIC LATER?
{
	//return if pointer is null
	if (line == nullptr)
	{
		return;
	}
	char* buffer = line;
	int index_line = 0;
	int index_buffer = 0;
	//count spaces
	while (line[index_line] == ' ') {
		index_line++;
	}
	//copy chars from line to beginning of buffer
	while (line[index_line])
	{
		buffer[index_buffer] = line[index_line];
		index_line++;
		index_buffer++;
	}
	//remove additional chars from end of buffer
	while (index_buffer < index_line)
	{
		buffer[index_buffer] = 0;
		index_buffer++;
	}
}

/* Function to remove trailing spaces from a command/line */
void RemoveTrailingWhitespace(char* line)
{
	//return if pointer is null
	if (line == nullptr)
	{
		return;
	}
	char* buffer = line;
	int index_line = 0;
	//find end of line
	while (line[index_line])
	{
		index_line++;
	}
	//skip null
	index_line--;

	//remove spaces
	while (line[index_line] == ' ') {
		line[index_line] = 0;
		index_line--;
	}
}

int CountCommands(char* line)
{
	int count = 1;
	int index = 0;
	while (line[index])
	{
		if (line[index] == '|' || line[index] == '<' || line[index] == '>')
		{
			count++;
		}
		index++;
	}
	return count;
}

static void MatchCommand(char* command, char* arg)
{
	if (strcmp(command, "echo") == 0)
	{
		echo_prot(arg);
	}
}

int FindPipeOrRedirect(int index, char* line)
{
	while (line[index])
	{
		if (line[index] == '|' || line[index] == '<' || line[index] == '>')
		{
			break;
		}
		index++;
	}
	return index;
}

static char* ProcessCommand(char* command)
{
	if (command[0] == 0)
	{
		return 0;
	}
	RemoveLeadingWhitespace(command);
	RemoveTrailingWhitespace(command);
	int index = 0;
	int index_arg = 0;
	char command_result[9];
	char argument_result[256];
	//load command into command_result - check for string of letters only
	while (command[index] >= 65 && command[index] <= 90 || command[index] >= 97 && command[index] <= 122)
	{
		//longest command is 8 chars
		if (index >= 8)
		{
			std::cout << "ERROR: command too long" << "\n";
			break;
		}
		command_result[index] = command[index];
		index++;
	}
	//if input ends after command - no argument - should be handled for each command differently
	if (command[index] == 0)
	{
		std::cout << "No argument" << "\n";
		
	}
	//command has to be followed by a space or a dot - really only these two cases? could also be handled separately for each command
	else if (!(command[index] == ' ' || command[index] == '.'))
	{
		std::cout << "ERROR: command not recognized" << "\n";
	}
	//null terminate command result
	command_result[index] = 0;
	
	//load argument into argument_result
	while (command[index])
	{
		argument_result[index_arg] = command[index];
		index++;
		index_arg++;
	}

	//null terminate argument result
	argument_result[index_arg] = 0;
	RemoveLeadingWhitespace(argument_result);
	MatchCommand(command_result, argument_result);
	std::cout << "cmd: " << command_result << "\n";
	std::cout << "arg: " << argument_result << "\n";
	return command_result;

}

void ProcessLine(char* line)
{
	int index_line = 0;
	int index_end_cmd = 0;
	int index_cmd = 0;
	int index_arg = 0;
	char command[256];

	std::cout << "\n";

	//loop through commands until EOL
	while(line[index_line])
	{
		index_cmd = 0;

		index_end_cmd = FindPipeOrRedirect(index_line, line);
		//std::cout << "index cmd, line, endcmd: " << index_cmd << " " << index_line << " " << index_end_cmd << "\n";
		

		//copy chars from line to command
		while(index_line <index_end_cmd && line[index_line])
		{
			command[index_cmd] = line[index_line];
			index_cmd++;
			index_line++;
		}

		//std::cout << "index cmd, line, endcmd: " << index_cmd << " " << index_line << " " << index_end_cmd << "\n";

		//null terminate command
		command[index_cmd] = 0;
		//skip the |<> if not EOL
		if (line[index_line])
		{
			index_line++;
		}

		ProcessCommand(command);

	}
}
