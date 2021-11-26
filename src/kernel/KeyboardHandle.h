#pragma once
#include "IOHandle.h"

class KeyboardHandle : public IOHandle {

	kiv_os::NOS_Error write(const char* buffer, const size_t size, size_t& written) override;
	/// <summary>
	/// 
	/// </summary>
	/// <param name="size"></param>
	/// <param name="buffer"></param>
	/// <param name="read"></param>
	/// <returns></returns>
	kiv_os::NOS_Error read(const size_t size, char* buffer, size_t& read) override;

	void close() { }


};