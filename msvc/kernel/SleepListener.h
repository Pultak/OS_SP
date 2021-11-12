#pragma once
#include "../../src/api/api.h"
#include "memory"
#include "../../src/kernel/Synchronization.h"
class SleepListener{
public:
	SleepListener(kiv_os::THandle handle): sleeperHandle(handle) {
		lock = std::make_unique<Synchronization::Spinlock>(false);
	}


public:
	kiv_os::THandle sleeperHandle;
	std::unique_ptr<Synchronization::Spinlock> lock;


};

