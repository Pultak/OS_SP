
#include <thread>
#include "handles.h"
#include "Process.h"

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


