#pragma once

#include "rtl.h"


enum class ProcessState {
	Ready = 0,
	Running = 1,
	Waiting = 2, //can we see when the process runs out of resources?
	Terminated = 3
};

struct ProcessEntry {
	kiv_os::THandle handle;
	kiv_os::THandle stdIn;
	kiv_os::THandle stdOut;
	ProcessState state;
	kiv_os::NOS_Error exitCode;
	char programName[42];
	char workingDir[256];
};


extern "C" size_t __stdcall tasklist(const kiv_hal::TRegisters& regs);
