#pragma once

#include "command_parser.h"
#include "echo.h"
#include <iostream>

/* Function to remove leading spaces from a command/line */
static void RemoveLeadingWhitespace(char* line) //MAKE STATIC LATER?
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
static void RemoveTrailingWhitespace(char* line)
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

/* Function to remove unnecessary whitespace from C string */
static void RemoveWhitespace(char* line)
{
	RemoveLeadingWhitespace(line);
	RemoveTrailingWhitespace(line);
}

/* Counts number of commands based on number of |<> */
static int CountCommands(char* line)
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

/* Executes a command - input: char* command, char* argument */
static void ExecuteCommand(char* command, char* arg)
{
	if (strcmp(command, "echo") == 0)
	{
		echo_prot(arg);
	}
}

/* Finds position of the next |<> */
static int FindPipeOrRedirect(int index, char* line)
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

/* Processes a single command - bounded by |<> */
static Program ProcessCommand(char* command, char operation_left, char operation_right)
{
	Program prog_ret;
	std::cout << prog_ret.command;
	if (command[0] == 0)
	{
		return prog_ret;
	}
	if (operation_left == '>')
	{
		prog_ret.command = command;
		prog_ret.redirection_in = true;
		return prog_ret;
	}
	if (operation_left == '<')
	{
		prog_ret.command = command;
		prog_ret.redirection_out = true;
		return prog_ret;
	}

	RemoveWhitespace(command);
	int index = 0;
	int index_arg = 0;
	char command_result[9];
	char argument_result[256];
	//load command into command_result - check for string of letters only
	while (command[index] >= 65 && command[index] <= 90 || command[index] >= 97 && command[index] <= 122)
	{
		command_result[index] = command[index];
		index++;
	}
	//if input ends after command - no argument - should be handled for each command differently
	if (command[index] == 0)
	{
		std::cout << "No argument" << "\n";
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
	ExecuteCommand(command_result, argument_result);
	std::cout << "cmd: " << command_result << "\n";
	std::cout << "arg: " << argument_result << "\n";
	return prog_ret;

}

/* Processes a whole line of input */
std::vector<Program> ProcessLine(char* line)
{
	int index_line = 0;
	int index_end_cmd = 0;
	int index_cmd = 0;
	int index_arg = 0;
	char command[256];
	char op_left = 0;
	char op_right = 0;

	std::cout << "\n";

	//loop through commands until EOL
	while(line[index_line])
	{
		index_cmd = 0;

		//load left operation |<>
		if (index_line == 0)
		{
			op_left = 0;
		}
		else
		{
			op_left = op_right;
		}

		index_end_cmd = FindPipeOrRedirect(index_line, line);

		//copy chars from line to command
		while(index_line <index_end_cmd && line[index_line])
		{
			command[index_cmd] = line[index_line];
			index_cmd++;
			index_line++;
		}

		//null terminate command
		command[index_cmd] = 0;

		//load right operation |<>
		if (line[index_line])
		{
			op_right = line[index_line];
			index_line++;
		}
		else
		{
			op_right = 0;
		}

		ProcessCommand(command, op_left, op_right);
		//std::cout << "index cmd, line, endcmd: " << index_cmd << " " << index_line << " " << index_end_cmd << "\n";

	}
	std::vector<Program> ret;
	return ret;
}
