#include "ProcessControlBlock.h"

/// <summary>
/// pcb
/// </summary>
std::map<kiv_os::THandle, Process*> table;

void ProcessControlBlock::AddNewProcess(kiv_os::THandle handle, kiv_os::THandle stdIn, kiv_os::THandle stdOut, char* program, std::filesystem::path actualDir) {
	lockMaster->lock();
	auto newProcess = new Process(handle, stdIn, stdOut, program, actualDir);

	table.emplace(std::make_pair(handle, newProcess));
	lockMaster->unlock();
}

Process* ProcessControlBlock::getProcess(kiv_os::THandle handle) const{
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

bool ProcessControlBlock::removeProcess(kiv_os::THandle handle) {
	lockMaster->lock();
	bool processRemoved = false;
	auto it = table.find(handle);
	if (it != table.end()) {
		auto proc = it->second;
		table.erase(it);
		delete proc;
		processRemoved = true;
	}
	lockMaster->unlock();
	return processRemoved;
}

void ProcessControlBlock::signalProcesses(kiv_os::NSignal_Id signal) {
	lockMaster->lock();

	for (const auto& entry : table) {
		auto handlers = entry.second->signalHandlers;
		//get assigned subrutine handler to the signal
		auto handler = handlers.find(signal);
		if (handler != handlers.end()) {
			//call the interrupt
			kiv_hal::TRegisters regs{};
			regs.rcx.l = static_cast<decltype(regs.rcx.l)>(signal);
			handler->second(regs);
		}
	}
	lockMaster->unlock();
}

void ProcessControlBlock::notifyAllListeners(){
	lockMaster->lock();
	for (const auto& entry : table) {
		entry.second->notifyAllThreads();
		entry.second->notifyRemoveListeners(entry.second->handle);
	}

	lockMaster->unlock();
}
