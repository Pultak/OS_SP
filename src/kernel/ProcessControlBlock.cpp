#include "ProcessControlBlock.h"


void ProcessControlBlock::AddNewProcess(kiv_os::THandle handle, kiv_os::THandle stdIn, kiv_os::THandle stdOut, char* program, std::filesystem::path actualDir){
	lockMaster->lock();
	//todo trylock faster?
	auto newProcess = std::make_unique<Process>(handle, stdIn, stdOut, program);

	table.emplace(std::make_pair(handle, newProcess));
	lockMaster->unlock();
}

std::shared_ptr<Process> ProcessControlBlock::getProcess(kiv_os::THandle handle){
	lockMaster->lock();
	auto it = table.find(handle);
	if (it != table.end()) {
		lockMaster->unlock();
		return it->second;
	}
	else {
		lockMaster->unlock();
		return nullptr;
	}
}

void ProcessControlBlock::signalProcesses(kiv_os::NSignal_Id signal){
	lockMaster->lock();


	lockMaster->unlock();
}
