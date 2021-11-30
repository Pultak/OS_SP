#pragma once
#include "handles.h"

#include <map>
#include <mutex>
#include <random>
#include "Synchronization.h"
#include <memory>

std::map<kiv_os::THandle, HANDLE> Handles;
std::map<std::thread::id, kiv_os::THandle> id2Handle;
std::map<std::thread::id, kiv_os::THandle> parentHandles;
//std::mutex Handles_Guard;
kiv_os::THandle Last_Handle = 0;

std::random_device rd;
std::mt19937 gen(rd());
std::uniform_int_distribution<> dis(1, 6);

const std::unique_ptr<Synchronization::Spinlock> lock = std::make_unique<Synchronization::Spinlock>(0);



kiv_os::THandle handles::Convert_Native_Handle(const std::thread::id tId, const HANDLE hnd, const kiv_os::THandle parentHandle) {
	//std::lock_guard<std::mutex> guard(Handles_Guard);
	lock->lock();
	Last_Handle += dis(gen);	//vygenerujeme novy interni handle s nahodnou hodnotou

	Handles.insert(std::pair<kiv_os::THandle, HANDLE>(Last_Handle, hnd));
	id2Handle.insert(std::pair<std::thread::id, kiv_os::THandle>(tId, Last_Handle));
	
	//are we converting thread handle atm?
	if (parentHandle != kiv_os::Invalid_Handle) {
		parentHandles.insert(std::pair<std::thread::id, kiv_os::THandle>(tId, parentHandle));
	}

	lock->unlock();
	return Last_Handle;
}

HANDLE handles::Resolve_kiv_os_Handle(const kiv_os::THandle hnd) {
	//std::lock_guard<std::mutex> guard(Handles_Guard);
	lock->lock();

	auto resolved = Handles.find(hnd);
	if (resolved != Handles.end()) {
		lock->unlock();
		return resolved->second;
	}
	else
		lock->unlock();
		return INVALID_HANDLE_VALUE;
}

kiv_os::THandle handles::getTHandleById(const std::thread::id id) {
	//std::lock_guard<std::mutex> guard(Handles_Guard);
	lock->lock();

	kiv_os::THandle result = kiv_os::Invalid_Handle;
	auto it = id2Handle.find(id);
	if (it != id2Handle.end()) {
		result = it->second;
	}
	lock->unlock();
	return result;
}

kiv_os::THandle handles::getParentTHandleById(const std::thread::id id) {
	//std::lock_guard<std::mutex> guard(Handles_Guard);
	lock->lock();

	kiv_os::THandle result = kiv_os::Invalid_Handle;
	auto it = parentHandles.find(id);
	if (it != parentHandles.end()) {
		result = it->second;
	}
	lock->unlock();
	return result;
}


bool handles::Remove_Handle(const kiv_os::THandle hnd) {
	lock->lock();
	//std::lock_guard<std::mutex> guard(Handles_Guard);
	auto result = Handles.erase(hnd) == 1;
	lock->unlock();
	return result;
}

kiv_os::THandle handles::removeHandleById(const std::thread::id id, bool isThread){
	//std::lock_guard<std::mutex> guard(Handles_Guard);
	lock->lock();

	kiv_os::THandle handle = kiv_os::Invalid_Handle;
	auto it = id2Handle.find(id);
	if (it != id2Handle.end()) {
		handle = it->second;
	}
	Handles.erase(handle);
	id2Handle.erase(id);
	lock->unlock();
	return handle;
}
