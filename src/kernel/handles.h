#pragma once

#include <Windows.h>
#include <thread>
#include "..\api\api.h"


namespace handles {
	
	/// <summary>
	/// This function converts the native thread handle to OS Thandle.
	/// The handles are saved to internal map and in addition we also saved their ids.
	/// For the purpose of creating threads(non process) there is also saved parent handle
	/// </summary>
	/// <param name="tId">new process/thread id</param>
	/// <param name="hnd">Native process/handle of the thread</param>
	/// <param name="parentHandle">handle of the parent for new threads</param>
	/// <returns>new OS THandle</returns>
	kiv_os::THandle Convert_Native_Handle(const std::thread::id tId, const HANDLE hnd, const kiv_os::THandle parentHandle);
	
	HANDLE Resolve_kiv_os_Handle(const kiv_os::THandle hnd);
	
	kiv_os::THandle getTHandleById(const std::thread::id id);
	
	kiv_os::THandle getParentTHandleById(const std::thread::id id);
	
	bool Remove_Handle(const kiv_os::THandle hnd);
	
	kiv_os::THandle removeHandleById(const std::thread::id id, bool isThread);
};