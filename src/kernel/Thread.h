#pragma once
#include "Blockable.h"

class Thread: public Blockable{
public:
	kiv_os::THandle threadHandle;
	kiv_os::THandle parentHandle;

public:
	Thread(kiv_os::THandle tHandle, kiv_os::THandle pHandle) : threadHandle(tHandle), parentHandle(pHandle){

	}
	~Thread() {
	}

};