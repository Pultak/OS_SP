#pragma once
#include "../../src/api/api.h"
#include <map>
#include <memory>

#include "Synchronization.h"

class IOHandle {
public:
	/**
	* Making the class abstract by defining virtual destructor
	*/
	virtual ~IOHandle() = 0;

	virtual kiv_os::NOS_Error write(const char* buffer, const size_t size, size_t& written) = 0;
	virtual kiv_os::NOS_Error read(const size_t size, char* buffer, size_t& read) = 0;
	virtual kiv_os::NOS_Error seek(const size_t value, const kiv_os::NFile_Seek position, kiv_os::NFile_Seek op, size_t& res) {
		return kiv_os::NOS_Error::IO_Error; 
	};

	virtual void close() { }
};
