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


enum class ProcessState {
	Ready = 0,
	Running = 1,
	Waiting = 2, //can we see when the process runs out of resources?
	Terminated = 3
};




class Process {

public:
	/// <summary>
	/// External parameters of process
	/// </summary>
	kiv_os::THandle handle;
	kiv_os::THandle stdInput;
	kiv_os::THandle stdOutput;
	ProcessState state = ProcessState::Ready;
	kiv_os::NOS_Error exitCode = kiv_os::NOS_Error::Success;
	char* programName;

	std::filesystem::path workingDirectory;

	std::map<kiv_os::NSignal_Id, kiv_os::TThread_Proc> signalHandlers;

public:
	/// <summary>
	/// Internal parameters of process
	/// </summary>
	//std::shared_ptr<ThreadControlBlock> tcb;
	std::list<SleepListener*> listeners;

	Synchronization::Spinlock* listenersLock = new Synchronization::Spinlock(0);
	/*
	private:

		void addListener(SleepListener* listener);
		*/

public:

	Process(kiv_os::THandle handle, kiv_os::THandle stdIn, kiv_os::THandle stdOut, char* program) : handle(handle), stdInput(stdIn), stdOutput(stdOut), programName(program) {

		//no other signals are pressent atm
		//signalHandlers[kiv_os::NSignal_Id::Terminate] = defaultSignalHandler;
	}

	~Process() {
		//todo check exit codes by parent -> block process?
		delete listenersLock;
	}

};
