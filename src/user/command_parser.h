#pragma once
#include <vector>
#include <string>
#include <bitset>
#include <iostream>
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


	Program::Program() {
		command = "";
		argument = "";
	}
	void Print() {
		int flags = redirection_in << 3 | redirection_out << 2 | pipe_in << 1 | pipe_out;
		std::cout << "\nProgram: " << command << ", " << argument << ", rin, rout, pin, pout: " << std::bitset<4>(flags) << "\n";
	}
};

std::vector<Program> ProcessLine(char* line);
void Execute_Commands(std::vector<Program>& program_vector, const kiv_hal::TRegisters& regs);



