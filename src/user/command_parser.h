#pragma once
#include <vector>
#include <string>

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
};



std::vector<Program> ProcessLine(char* line);



