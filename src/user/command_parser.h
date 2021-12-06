#pragma once
#include <vector>
#include <string>
#include "..\api\api.h"

struct Program {
	std::string command;
	std::string argument;

	bool redirection_in = false;
	bool redirection_out = false;
	bool pipe_in = false;
	bool pipe_out = false;
	bool file = false;

	kiv_os::THandle handle = kiv_os::Invalid_Handle;
	kiv_os::THandle pipe_in_handle = kiv_os::Invalid_Handle;
	kiv_os::THandle pipe_out_handle = kiv_os::Invalid_Handle;
	kiv_os::THandle redirection_in_handle = kiv_os::Invalid_Handle;
	kiv_os::THandle redirection_out_handle = kiv_os::Invalid_Handle;



	Program::Program() {
		command = "";
		argument = "";
	}
};

std::vector<Program> ProcessLine(char* line);
void Execute_Commands(std::vector<Program>& program_vector, const kiv_hal::TRegisters& regs);



