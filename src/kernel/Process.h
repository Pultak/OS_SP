#pragma once
#include <memory.h>
#include <memory>
#include <iostream>
#include <map>
#include <filesystem>
#include <Windows.h>
//#include "ProcessControlBlock.h"
//#include "ThreadControlBlock.h"
#include "SleepListener.h"
#include "Synchronization.h"
#include "../../src/api/api.h"
#include "Blockable.h"
#include "Thread.h"

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
	char workingDir[128];
};


class Process: public Blockable {

public:
	/// <summary>
	/// External parameters of process
	/// </summary>
	char programName[42];
	kiv_os::THandle handle;
	kiv_os::THandle stdInput;
	kiv_os::THandle stdOutput;
	ProcessState state = ProcessState::Ready;
	kiv_os::NOS_Error exitCode = kiv_os::NOS_Error::Success;

	std::filesystem::path workingDirectory;

	std::map<kiv_os::NSignal_Id, kiv_os::TThread_Proc> signalHandlers;

private:
	std::map<kiv_os::THandle, Thread*> tcb;

public:

	//char* getProgramName();

	void addNewThread(kiv_os::THandle threadHandle);
	Thread* getThread(kiv_os::THandle threadHandle);
	void removeThread(kiv_os::THandle threadHandle);
	void notifyAllThreads();

public:
	Process(kiv_os::THandle handle, kiv_os::THandle stdIn, kiv_os::THandle stdOut, const char* program, std::filesystem::path wd);
	~Process() {
	}




};
