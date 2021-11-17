#pragma once
#include <vector>
#include <string>
#include <bitset>
#include <iostream>



struct Program {
	std::string command;
	std::string argument;

	bool redirection_in = false;
	bool redirection_out = false;
	bool pipe_in = false;
	bool pipe_out = false;

	Program::Program() {
		command = "";
		argument = "";
	}
	void Print() {
		int flags = redirection_in << 3 | redirection_out << 2 | pipe_in << 1 | pipe_out;
		std::cout << "Program: " << command << ", " << argument << ", rin, rout, pin, pout: " << std::bitset<4>(flags) << "\n";
	}
};

std::vector<Program> ProcessLine(char* line);



