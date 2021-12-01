#include "PipeUtils.h"


bool PipeOut::write(const char c){
    if (pipe->writeClosed || pipe->readClosed) {
        return false;
    }
    pipe->writeLock->unlock(1);

    //check again -> handle could get closed in the sleeptime
    if (pipe->writeClosed || pipe->readClosed) {
        return false;
    }
    pipe->buffer[pipe->writePtr] = c;
    ++(pipe->writePtr);

    if (pipe->writePtr == pipe->pipeSize) pipe->writePtr = 0; // did we reach the end of the buffer?
    //notify the reader that one char is ready
    pipe->readLock->unlock(1);
    return true;
}

kiv_os::NOS_Error PipeOut::write(const char* buffer, const size_t size, size_t& written){
    //write all characters from buffer
    for (written = 0; written < size && write(buffer[written]); ++written);
    if (written > 0) {
        return kiv_os::NOS_Error::Success;
    }else {
        return kiv_os::NOS_Error::IO_Error;
    }

}


void PipeOut::close() {
    //write the EOT(end of transmission) char to pipe
	write((char)(kiv_hal::NControl_Codes::EOT));
	pipe->writeClosed = true;
}


kiv_os::NOS_Error PipeIn::read(const size_t size, char* out_buffer, size_t& read) {
    kiv_os::NOS_Error resultSignal = kiv_os::NOS_Error::Success;
    // write contents of vector<char> to the given buffer
    for (size_t i = 0; i < size; ++i) {
        // check whether out handle has been closed and there is nothing left to read
        if (pipe->readClosed && pipe->readPtr == pipe->writePtr) {
            resultSignal = kiv_os::NOS_Error::IO_Error;
            break;
        }
        pipe->readLock->lock();

        // check whether the out handle has not been closed while we have been waiting
        if (pipe->readClosed) {
            break;
        }

        char val = pipe->buffer[pipe->readPtr];
        ++(pipe->readPtr);
        //are we at the end of the pipe?
        if (pipe->readPtr  >= pipe->pipeSize) pipe->readPtr = 0;
       
        //one char read -> one more char free to write
        pipe->writeLock->unlock(1);
        out_buffer[i] = val;
        ++read;
        if (val == static_cast<char>(kiv_hal::NControl_Codes::EOT)) {
            // wild EOT appeared
            close();
            break;
        }
    }
    return resultSignal;
}

void PipeIn::close() {
	pipe->readClosed = true;
	pipe->writeLock->unlock(1);
	pipe->readLock->unlock(1);
}
