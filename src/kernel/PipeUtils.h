#pragma once
#include "IOHandle.h"
#include <vector>
#include "Synchronization.h"

class Pipe  {
public:
	bool writeClosed = false;
	bool readClosed = false;
	size_t writePtr = 0;
	size_t readPtr = 0;

	const size_t pipeSize;
	char* buffer;

	Synchronization::IntSpinlock* readLock;
	Synchronization::IntSpinlock* writeLock;


public:
	Pipe(const size_t size) : pipeSize(size) {
		buffer = new char[size];
		//
		readLock = new Synchronization::IntSpinlock(size);
		//input can write until buffer full
		writeLock = new Synchronization::IntSpinlock(0);
	}

	~Pipe() {
		delete buffer;
		delete readLock;
		delete writeLock;
	}
};

class PipeOut: public IOHandle{
public:

	kiv_os::NOS_Error write(const char* buffer, const size_t size, size_t& written) override;

	kiv_os::NOS_Error read(const size_t size, char* out_buffer, size_t& read) override {
		return kiv_os::NOS_Error::IO_Error;
	};


	void close() override;

	PipeOut(Pipe * pip): pipe(pip) {	}

	~PipeOut()  {
	}
private:
	bool write(char c);

	Pipe* pipe;
};

class PipeIn : public IOHandle {
public:

	kiv_os::NOS_Error write(const char* buffer, const size_t size, size_t& written) override {
		return kiv_os::NOS_Error::IO_Error;
	};

	kiv_os::NOS_Error read(const size_t size, char* out_buffer, size_t& read) override;


	void close() override;
	

	PipeIn(Pipe* pip) : pipe(pip) {	}

	~PipeIn() {
	}
private:
	Pipe* pipe;
};
