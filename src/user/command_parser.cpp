#pragma once

#include "command_parser.h"

void RemoveLeadingWhitespace(char* line)
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
