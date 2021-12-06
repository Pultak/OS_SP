#pragma once
#include "Synchronization.h"

#include "../api/api.h"

class Blockable {

public:
	void addListener(Synchronization::Semaphore* newListener);

	void removeListener(const kiv_os::THandle thisHandle);


	void notifyRemoveListeners(kiv_os::THandle actualHandle);


	~Blockable() {
	}
};