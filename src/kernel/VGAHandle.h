#pragma once

#include "IOHandle.h"

class VGAHandle : public IOHandle {

	kiv_os::NOS_Error write(const char* buffer, const size_t size, size_t& written) override;
	kiv_os::NOS_Error read(const size_t size, char* buffer, size_t& read) override;

	void close() { }


};