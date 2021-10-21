#pragma once

#include "command_parser.h"
#include <iostream>

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

void ProcessCommand(char* command)
{
	//check for string of letters only
	if (command[0] == 0)
	{
		return;
	}
	RemoveLeadingWhitespace(command);
	int index = 0;
	char command_result[9];
	while (command[index] >= 65 && command[index] <= 90 || command[index] >= 97 && command[index] <= 122)
	{
		if (index >= 8)
		{
			std::cout << "ERROR" << "\n";
			break;
		}
		command_result[index] = command[index];
		index++;
	}
	command_result[index] = 0;

	std::cout << "cmd: " << command_result << "\n";
}

void ProcessLine(char* line)
{
	int index_line = 0;
	int index_end_cmd = 0;
	int index_cmd = 0;
	char command[256];
	std::cout << "\n";

	//loop through commands until EOL
	while(line[index_line])
	{
		index_cmd = 0;
		index_end_cmd = FindPipeOrRedirect(index_line, line);
		std::cout << "index cmd, line, endcmd: " << index_cmd << " " << index_line << " " << index_end_cmd << "\n";
		

		//copy chars from line to command
		while(index_line <index_end_cmd && line[index_line])
		{
			command[index_cmd] = line[index_line];
			index_cmd++;
			index_line++;
		}

		std::cout << "index cmd, line, endcmd: " << index_cmd << " " << index_line << " " << index_end_cmd << "\n";

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
