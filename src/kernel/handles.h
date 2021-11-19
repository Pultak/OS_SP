#pragma once

#include <Windows.h>
#include <thread>
#include "..\api\api.h"


namespace handles {
	kiv_os::THandle Convert_Native_Handle(const std::thread::id tId, const HANDLE hnd);
	HANDLE Resolve_kiv_os_Handle(const kiv_os::THandle hnd);
	kiv_os::THandle getTHandleById(const std::thread::id id);
	bool Remove_Handle(const kiv_os::THandle hnd);
	kiv_os::THandle removeHandleById(const std::thread::id id);
};