#pragma once

#include "command_parser.h"

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

int FindPipeOrRedirect(char* line)
{
	int index = 0;
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
}

void ProcessLine(char* line)
{
	//check how many |<>, then loop through each command
	int nr_cmds = CountCommands(line);

	for (int i = 0; i < nr_cmds; i++)
	{
		ProcessCommand();
	}
}
