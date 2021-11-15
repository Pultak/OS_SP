#pragma once
#include "../api/api.h"

#include "Synchronization.h"
#include "Process.h"
#include <memory>

class ProcessControlBlock{
public:

	ProcessControlBlock(){
		lockMaster = new Synchronization::Spinlock(0);
	}
	~ProcessControlBlock() {
		delete lockMaster;
	}
	void AddNewProcess(kiv_os::THandle handle, kiv_os::THandle stdIn, kiv_os::THandle stdOut, char* program, std::filesystem::path actualDir);
	Process* getProcess(kiv_os::THandle handle);
	bool removeProcess(kiv_os::THandle handle);
	void signalProcesses(kiv_os::NSignal_Id signal);

private:
	std::map<kiv_os::THandle, Process*> table;

	static Synchronization::Spinlock* lockMaster;


};

