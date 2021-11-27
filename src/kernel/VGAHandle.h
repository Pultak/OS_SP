#pragma once

#include "IOHandle.h"
#include "../../src/api/api.h"

class VGAHandle : public IOHandle {
public:
	explicit VGAHandle() {};
	kiv_os::NOS_Error write(const char* buffer, const size_t size, size_t& written) override;
	kiv_os::NOS_Error read(const size_t size, char* buffer, size_t& read) override;

	void close() override;

	~VGAHandle() {	}
};