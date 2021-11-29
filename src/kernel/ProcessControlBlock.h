#pragma once
#include <memory>

#include "Synchronization.h"
#include "Process.h"

#include "../api/api.h"

class ProcessControlBlock {
public:

	ProcessControlBlock() {
		lockMaster = new Synchronization::Spinlock(0);
	}
	~ProcessControlBlock() {
		delete lockMaster;
	}
	/// <summary>
	/// Function adds newly created process to the pcb
	/// </summary>
	/// <param name="handle">handle of the process</param>
	/// <param name="stdIn">standart input</param>
	/// <param name="stdOut">standart output</param>
	/// <param name="program">process running program name</param>
	/// <param name="actualDir">actual directory location</param>
	void AddNewProcess(kiv_os::THandle handle, kiv_os::THandle stdIn, kiv_os::THandle stdOut, char* program, std::filesystem::path actualDir);
	Process* getProcess(kiv_os::THandle handle) const;
	
	bool removeProcess(kiv_os::THandle handle);
	
	/// <summary>
	/// Send signal to every process in the pcb
	/// </summary>
	/// <param name="signal">called signal</param>
	void signalProcesses(kiv_os::NSignal_Id signal) const ;

	/// <summary>
	/// Notify listeners of every process located in pcb
	/// </summary>
	void notifyAllListeners() const ;

	void* getAllProcesses(size_t& processCount);

	bool isUpdated();

private:

	Synchronization::Spinlock* lockMaster;


};

