#pragma once
#include "Process.h"
#include "../api/api.h"

#include "Synchronization.h"

class ProcessControlBlock{
public:

	ProcessControlBlock(){
		lockMaster = std::make_unique<Synchronization::Spinlock>(new Synchronization::Spinlock());
	}

	void AddNewProcess(kiv_os::THandle handle, kiv_os::THandle stdIn, kiv_os::THandle stdOut, char* program, std::filesystem::path actualDir);
	std::shared_ptr<Process> getProcess(kiv_os::THandle handle);
	void signalProcesses(kiv_os::NSignal_Id signal);

private:
	std::map<kiv_os::THandle, std::shared_ptr<Process>> table;

	static std::unique_ptr<Synchronization::Spinlock> lockMaster;


};

