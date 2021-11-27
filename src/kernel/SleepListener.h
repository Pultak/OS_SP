#pragma once
#include <memory>
#include "../../src/kernel/Synchronization.h"
#include "../../src/api/api.h"
class SleepListener {
public:
	SleepListener(kiv_os::THandle handle) : sleeperHandle(handle) {
		lock = std::make_unique<Synchronization::Spinlock>(true);
	}


public:
	/// <summary>
	/// Handle of the locked thread
	/// </summary>
	kiv_os::THandle sleeperHandle;
	std::unique_ptr<Synchronization::Spinlock> lock;
	/// <summary>
	/// Handle of the thread that notified the locked thread
	/// </summary>
	kiv_os::THandle notifierHandle = kiv_os::Invalid_Handle;
	/// <summary>
	/// Flag if the listener was already notified
	/// </summary>
	bool notified = false;


};
