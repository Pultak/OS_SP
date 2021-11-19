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
	kiv_os::THandle sleeperHandle;
	std::unique_ptr<Synchronization::Spinlock> lock;


};

