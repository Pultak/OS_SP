
#include <thread>
#include "handles.h"
#include "Process.h"

Process::Process(kiv_os::THandle handle, kiv_os::THandle stdIn, kiv_os::THandle stdOut, const char* program, std::filesystem::path wd)
	: handle(handle), stdInput(stdIn), stdOutput(stdOut), workingDirectory(wd) {
	tcbLock = new Synchronization::Spinlock(0);
	//memcpy(programName, '\0', sizeof(programName));
	auto length = strlen(program);
	strcpy_s(programName, length + 1, program);
	if (length < sizeof(programName))
		programName[length] = 0;
}

/*char* Process::getProgramName(){
	return programName;
}*/

void Process::addNewThread(kiv_os::THandle threadHandle){
	tcbLock->lock();
	Thread* newThread = new Thread(threadHandle, this->handle);
	
	tcb.insert(std::make_pair(threadHandle, newThread));
	tcbLock->unlock();
}

Thread* Process::getThread(kiv_os::THandle threadHandle){
	tcbLock->lock();
	auto it = tcb.find(threadHandle);
	if (it != tcb.end()) {
		tcbLock->unlock();
		return it->second;
	}
	tcbLock->unlock();
	return nullptr;
}

void Process::removeThread(kiv_os::THandle threadHandle){
	tcbLock->lock();
	auto it = tcb.find(threadHandle);
	if (it != tcb.end()) {
		delete it->second;
		tcb.erase(it);
	}
	tcbLock->unlock();
}

void Process::notifyAllThreads(){
	tcbLock->lock();
	for (const auto& entry : tcb) {
		entry.second->notifyRemoveListeners(entry.second->threadHandle);
	}
	tcbLock->unlock();
}


