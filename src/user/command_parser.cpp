#pragma once

#include "command_parser.h"
#include "echo.h"

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
	Program program_ret;
	RemoveWhitespace(command);

	//return empty empty program if there is no input
	if (command[0] == 0)
	{
		return program_ret;
	}
	//load whole command string into program if it's a file
	if (operation_left == '>')
	{
		program_ret.command = command;
		program_ret.redirection_in = true;
		program_ret.file = true;
		return program_ret;
	}
	if (operation_left == '<')
	{
		program_ret.command = command;
		program_ret.redirection_out = true;
		program_ret.file = true;
		return program_ret;
	}
	//set redirection and pipe flags for other cases
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
		program_ret = ProcessCommand(command, op_left, op_right);
		//program_ret.Print();

		//std::cout << "index cmd, line, endcmd: " << index_cmd << " " << index_line << " " << index_end_cmd << "\n";
		vector_program_ret.push_back(program_ret);
	}
	return vector_program_ret;
}

void Execute_Commands(std::vector<Program> program_vector, const kiv_hal::TRegisters& regs) {

	size_t index = 0;
	uint16_t exit_code = 0;
	kiv_os::THandle in = regs.rax.x;
	kiv_os::THandle out = regs.rbx.x;
	kiv_os::THandle current_pipe[2] = { kiv_os::Invalid_Handle, kiv_os::Invalid_Handle };
	kiv_os::THandle process_handle = kiv_os::Invalid_Handle;
	kiv_os::THandle signal_ret;
	std::vector<kiv_os::THandle> processes_handles;
	std::vector<kiv_os::THandle> pipes_in;
	std::vector<kiv_os::THandle> pipes_out;
	std::vector<Program> running_processes;



	for (auto program : program_vector)
	{
		//might need to transfer |
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
			continue;
		}
		if (program.redirection_in)
		{
			auto result = kiv_os_rtl::Open_File(program_vector.at(index+1).command.c_str(), kiv_os::NOpen_File::fmOpen_Always, kiv_os::NFile_Attributes::System_File, in);
			std::cout << "result: " << result;
		}
		if (program.redirection_out)
		{
			//kiv_os_rtl::Open_File(program_vector.at(index+1).command.c_str(), kiv_os::NOpen_File::fmOpen_Always, kiv_os::NFile_Attributes::System_File, out);
		}
		if (program.pipe_in)
		{
			in = current_pipe[0];
			pipes_in.push_back(in);
		}
		if (program.pipe_out)
		{
			kiv_os_rtl::Create_Pipe(current_pipe);
			out = current_pipe[1];
			pipes_out.push_back(out);
		}
		program.Print();

		auto success = kiv_os_rtl::Create_Process(program.command.c_str(), program.argument.c_str(), 100, out, process_handle);
		if (!success)
		{
			std::cout << "\nInvalid argument\n";
		}
		else
		{
			processes_handles.push_back(process_handle);
			running_processes.push_back(program);
		}
		index++;
	}

	auto it = processes_handles.begin();

	/*while (processes_handles.size())
	{
		kiv_os_rtl::Wait_For(processes_handles.data(), processes_handles.size(), signal_ret);
		// -1 fix --- remove later
		kiv_os_rtl::Read_Exit_Code(processes_handles.at(signal_ret-1), exit_code);

		if (running_processes.at(signal_ret - 1).pipe_in)
		{
			kiv_os_rtl::Close_Handle(pipes_in.at(0));
			pipes_in.erase(pipes_in.begin());
		}
		if (running_processes.at(signal_ret - 1).pipe_out)
		{
			kiv_os_rtl::Close_Handle(pipes_out.at(0));
			pipes_out.erase(pipes_out.begin());
		}

		processes_handles.erase(it + signal_ret);
	}*/
	while (!processes_handles.empty()) {
		kiv_os_rtl::Wait_For(processes_handles.data(), processes_handles.size(), signal_ret);

		kiv_os_rtl::Read_Exit_Code(processes_handles[signal_ret], exit_code);

		//print exit code
		std::cout << "\nexit code: " << exit_code << "\n";
		processes_handles.erase(processes_handles.begin() + signal_ret);
	}

	for (auto pipe : pipes_in)
	{
		kiv_os_rtl::Close_Handle(pipe);
	}
	for (auto pipe : pipes_out)
	{
		kiv_os_rtl::Close_Handle(pipe);
	}



}
