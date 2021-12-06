#include "ProcessControlBlock.h"

/// <summary>
/// pcb
/// </summary>
std::map<kiv_os::THandle, Process*> table;

const std::unique_ptr<Synchronization::Spinlock> lockMaster = std::make_unique<Synchronization::Spinlock>(0);
bool tableUpdated = 0;


ProcessControlBlock::ProcessControlBlock() {
}

ProcessControlBlock::~ProcessControlBlock() {
}

void ProcessControlBlock::AddNewProcess(kiv_os::THandle handle, kiv_os::THandle stdIn, kiv_os::THandle stdOut, char* program, std::filesystem::path actualDir) {
	lockMaster->lock();
	auto newProcess = new Process(handle, stdIn, stdOut, program, actualDir);

	table.emplace(std::make_pair(handle, newProcess));
	tableUpdated = true;
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
	tableUpdated = true;
	lockMaster->unlock();
	return processRemoved;
}

void ProcessControlBlock::signalProcesses(kiv_os::NSignal_Id signal) const {
	lockMaster->lock();

	auto it = table.rbegin();
	while (it != table.rend()) {
		const auto& handlers = it->second->signalHandlers;
		//get assigned subrutine handler to the signal
		auto handler = handlers.find(signal);
		if (handler != handlers.end()) {
			//call the interrupt
			kiv_hal::TRegisters regs{};
			regs.rcx.l = static_cast<decltype(regs.rcx.l)>(signal);
			handler->second(regs);
		}
		++it;
	}
	lockMaster->unlock();
}

void ProcessControlBlock::notifyAllListeners() const {
	lockMaster->lock();

	auto it = table.rbegin();
	while (it != table.rend()) {
		it->second->notifyAllThreads();
		it->second->notifyRemoveListeners(it->second->handle);
		++it;
	}
	lockMaster->unlock();
}

ProcessEntry* ProcessControlBlock::getAllProcesses(size_t& processCount){
	lockMaster->lock();
	if (!table.empty()) {
		auto tableSize = table.size();
		ProcessEntry* result = new ProcessEntry[tableSize];
		uint16_t index;
		auto it = table.begin();
		for (index = 0; index < tableSize; ++index, ++it) {
			const auto& process = it->second;
			result[index].handle = process->handle;
			result[index].stdIn = process->stdInput;
			result[index].stdOut = process->stdOutput;
			result[index].state = process->state;
			result[index].exitCode = process->exitCode;

			auto progNameL = strlen(process->programName);
			auto pNameLength = sizeof(ProcessEntry::programName) > progNameL ? sizeof(ProcessEntry::programName) : progNameL;
			strcpy_s(result[index].programName, pNameLength, process->programName);

			auto wDirL = process->workingDirectory.string().length();
			auto workDirLength = sizeof(ProcessEntry::workingDir) > wDirL ? sizeof(ProcessEntry::workingDir) : wDirL;
			
			strcpy_s(result[index].workingDir, workDirLength, process->workingDirectory.string().c_str());
		}
		//we collected all -> table is no longer diff from tasklisk file
		tableUpdated = false;
		processCount = index;
		lockMaster->unlock();
		return result;
	}else {
		lockMaster->unlock();
		return nullptr;
	}
}

bool ProcessControlBlock::isUpdated(){
	return tableUpdated;
}
