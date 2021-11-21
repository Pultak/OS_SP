
#include <thread>
#include "handles.h"
#include "Process.h"

void Process::addNewThread(kiv_os::THandle threadHandle){
	tcbLock->lock();
	auto newThread = std::make_unique<Thread>(new Thread(threadHandle, handle));
	tcb.insert(std::make_pair(threadHandle, newThread));
	tcbLock->unlock();
}

Thread* Process::getThread(kiv_os::THandle threadHandle){
	tcbLock->lock();
	auto it = tcb.find(threadHandle);
	if (it != tcb.end()) {
		return it->second.get();
	}
	tcbLock->unlock();
	return nullptr;
}

void Process::removeThread(kiv_os::THandle threadHandle){
	tcbLock->lock();
	auto it = tcb.find(threadHandle);
	if (it != tcb.end()) {
		tcb.erase(it);
	}
	tcbLock->unlock();
}


