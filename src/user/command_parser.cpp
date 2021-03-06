#pragma once

#include "command_parser.h"
#include "rtl.h"

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

/* Counts number of pipes */
static int CountPipes(std::vector<Program> vector)
{
	int count = 0;
	for (auto program : vector)
	{
		if (program.pipe_out == true)
		{
			count++;
		}
	}
	return count;
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
	Program program_ret;
	RemoveWhitespace(command);

	//return empty empty program if there is no input
	if (command[0] == 0)
	{
		return program_ret;
	}
	//set redirection and pipe flags
	if (operation_left == '|')
	{
		program_ret.pipe_in = true;
	}
	if (operation_right == '|')
	{
		program_ret.pipe_out = true;
	}
	if (operation_right == '<')
	{
		program_ret.redirection_in = true;
	}
	if (operation_right == '>')
	{
		program_ret.redirection_out = true;
	}
	//load whole command string into program if it's a file
	if (operation_left == '>')
	{
		program_ret.redirection_in = true;
		program_ret.command = command;
		program_ret.file = true;
		return program_ret;
	}
	if (operation_left == '<')
	{
		program_ret.redirection_in = true;
		program_ret.command = command;
		program_ret.file = true;
		return program_ret;
	}

	int index = 0;
	int index_arg = 0;
	char command_result[256];
	char argument_result[256];
	//load command into command_result - check for string of letters only
	while (command[index] >= 65 && command[index] <= 90 || command[index] >= 97 && command[index] <= 122)
	{
		command_result[index] = command[index];
		index++;
	}

	//null terminate command result and load into program struct
	command_result[index] = 0;
	program_ret.command = command_result;

	//load argument into argument_result
	while (command[index])
	{
		argument_result[index_arg] = command[index];
		index++;
		index_arg++;
	}

	//null terminate argument result, remove additional whitespace and load into program struct
	argument_result[index_arg] = 0;
	RemoveLeadingWhitespace(argument_result);
	program_ret.argument = argument_result;

	//ExecuteCommand(command_result, argument_result);
	return program_ret;
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
	Program program_ret;
	std::vector<Program> vector_program_ret;


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

		//process single command and put it into the programs vector
		program_ret = ProcessCommand(command, op_left, op_right);
		vector_program_ret.push_back(program_ret);
	}
	return vector_program_ret;
}

void Execute_Commands(std::vector<Program>& program_vector, const kiv_hal::TRegisters& regs) {

	size_t index = program_vector.size()-1;
	kiv_os::NOS_Error exit_code = kiv_os::NOS_Error::Success;
	kiv_os::THandle in_reg = regs.rax.x;
	kiv_os::THandle out_reg = regs.rbx.x;
	kiv_os::THandle in = in_reg;
	kiv_os::THandle out = out_reg;
	kiv_os::THandle program_handle = kiv_os::Invalid_Handle;
	kiv_os::THandle signaled_handle = kiv_os::Invalid_Handle;
	kiv_os::THandle signal_ret;
	size_t written;
	Program signaled_program = Program();
	std::vector<kiv_os::THandle> active_handles;
	std::vector<kiv_os::THandle> pipes;

	//create pipes for each program
	for (int i = 0; i < CountPipes(program_vector); i++)
	{
		kiv_os::THandle pipe_temp[2];
		kiv_os_rtl::Create_Pipe(pipe_temp);

		pipes.push_back(pipe_temp[1]);
		pipes.push_back(pipe_temp[0]);
	}

	//loop through programs backwards
	for(auto program_it = program_vector.rbegin(); program_it != program_vector.rend(); program_it++)
	{
		Program& program = *program_it;

		in = in_reg;
		out = out_reg;
		
		//skip the program if it's cd or echo on/off - handled in shell, or if it's a file
		if (strcmp(program.command.c_str(), "cd") == 0)
		{
			continue;
		}
		else if (strcmp(program.command.c_str(), "echo") == 0 && 
				(strcmp(program.argument.c_str(), "on") == 0 || strcmp(program.argument.c_str(), "off") == 0))
		{
			continue;
		}
		else if(program.file)
		{
			index--;
			continue;
		}
		//if this is the last program, ignore redirection and pipe out
		if (!((program_vector.size() - 1) == index) )
		{
			if (program.redirection_in)
			{				
				auto result = kiv_os_rtl::Open_File(program_vector.at(index + 1).command.c_str(), (kiv_os::NOpen_File)0, kiv_os::NFile_Attributes::System_File, in);

				if (result)
				{
					program.redirection_in_handle = in;
					//if there is a pipe after the file and there is a program to pipe it into, we want to pipe the current program out
					if (program_vector.at(index + 1).pipe_out && (program_vector.size() > index + 2))
					{
						program.pipe_out = true;
					}
				}
				//failed to open the file
				else
				{
					in = in_reg;
					std::string message = "\nFailed to open: '";
					message.append(program_vector.at(index + 1).command.c_str());
					message.append("'\n");
					kiv_os_rtl::Write_File(out_reg, message.c_str(), message.size(), written);
					break;
				}
			}
			if (program.redirection_out)
			{
				//open the next program = file, if open fails print a message and break cycle
				if (kiv_os_rtl::Open_File(program_vector.at(index + 1).command.c_str(), (kiv_os::NOpen_File)0, kiv_os::NFile_Attributes::System_File, out))
				{
					program.redirection_out_handle = out;
				}
				else
				{
					out = out_reg;
					std::string message = "\nFailed to open: '";
					message.append(program_vector.at(index + 1).command.c_str());
					message.append("'\n");
					kiv_os_rtl::Write_File(out_reg, message.c_str(), message.size(), written);
					break;
				}
			}

			if (program.pipe_out)
			{
				//assign pipe handle to the current program in the vector
				out = pipes.at(pipes.size()-1);
				pipes.pop_back();
				program.pipe_out_handle = out;
			}
		}
		if (program.pipe_in)
		{
			//create pipe handle to the current program
			in = pipes.at(pipes.size() - 1);
			pipes.pop_back();
			program.pipe_in_handle = in;
		}

		auto success = kiv_os_rtl::Create_Process(program.command.c_str(), program.argument.c_str(), in, out, program_handle);
		if (!success)
		{
			//write error message of failed program and close pipe handles if it has been piped
			std::string message = "'";
			message.append(program.command.c_str());
			message.append("' is not recognized as an internal command.\n");
			kiv_os_rtl::Write_File(out_reg, message.c_str(), message.size(), written);
			if(program.pipe_in)
			{
				kiv_os_rtl::Close_Handle(program.pipe_in_handle);
			}
			if (program.pipe_out)
			{
				kiv_os_rtl::Close_Handle(program.pipe_out_handle);
			}
			break;
		}
		else
		{
			//push handle to the active_handles vector and assign handle to the current program in the vector
			program.handle = program_handle;
			active_handles.push_back(program_handle);
		}
		index--;
	}

	//while there are active processes
	while (active_handles.size())
	{
		auto it_program_vector = program_vector.begin();
		auto it_active_handles = active_handles.begin();

		index = 0;

		//wait for process to finish and read its exit code
		kiv_os_rtl::Wait_For(active_handles.data(), static_cast<uint16_t>(active_handles.size()), signal_ret);
		signaled_handle = active_handles.at(signal_ret);
		kiv_os_rtl::Read_Exit_Code(signaled_handle, exit_code);

		//find the process in the program vector by handle
		for (auto& program : program_vector)
		{
			if (program.handle == signaled_handle)
			{
				signaled_program = program;
				//print error if ocurred
				if (exit_code != kiv_os::NOS_Error::Success)
				{
					std::string error_message = signaled_program.command;
					switch (exit_code)
					{
					case kiv_os::NOS_Error::File_Not_Found:
						error_message.append(": File Not Found.\n");
						kiv_os_rtl::Write_File(out_reg, error_message.c_str(), error_message.size(), written);
						break;
					case kiv_os::NOS_Error::Invalid_Argument:
						error_message.append(": Invalid Argument.\n");
						kiv_os_rtl::Write_File(out_reg, error_message.c_str(), error_message.size(), written);
						break;
					case kiv_os::NOS_Error::IO_Error:
						error_message.append(": IO Error.\n");
						kiv_os_rtl::Write_File(out_reg, error_message.c_str(), error_message.size(), written);
						break;
					case kiv_os::NOS_Error::Unknown_Error:
						error_message.append(": Unknown Error.\n");
						kiv_os_rtl::Write_File(out_reg, error_message.c_str(), error_message.size(), written);
						break;
					}
				}
				break;
			}
			index++;
		}

		//close pipes in case program was piped and erase it from active processes and program vector
		if (signaled_program.pipe_in)
		{
			kiv_os_rtl::Close_Handle(signaled_program.pipe_in_handle);
		}
		if (signaled_program.pipe_out)
		{
			kiv_os_rtl::Close_Handle(signaled_program.pipe_out_handle);
		}
		if (signaled_program.redirection_in)
		{
			kiv_os_rtl::Close_Handle(signaled_program.redirection_in_handle);
		}
		if (signaled_program.redirection_out)
		{
			kiv_os_rtl::Close_Handle(signaled_program.redirection_out_handle);
		}
		program_vector.erase(it_program_vector + index);
		active_handles.erase(it_active_handles + signal_ret);
	}
}
