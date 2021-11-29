#include "io.h"



std::map<kiv_os::THandle, IOHandle*> openedHandles;
const std::unique_ptr<Synchronization::Spinlock> ioHandleLock = std::make_unique<Synchronization::Spinlock>(0);


size_t io::Read_Line_From_Console(char *buffer, const size_t buffer_size) {
	//todo remove
	kiv_hal::TRegisters registers;
	
	size_t pos = 0;
	while (pos < buffer_size) {
		//read char
		registers.rax.h = static_cast<decltype(registers.rax.l)>(kiv_hal::NKeyboard::Read_Char);
		kiv_hal::Call_Interrupt_Handler(kiv_hal::NInterrupt::Keyboard, registers);
		
		if (!registers.flags.non_zero) break;	//nic jsme neprecetli, 
												//pokud je rax.l EOT, pak byl zrejme vstup korektne ukoncen
												//jinak zrejme doslo k chybe zarizeni

		char ch = registers.rax.l;

		//osetrime zname kody
		switch (static_cast<kiv_hal::NControl_Codes>(ch)) {
			case kiv_hal::NControl_Codes::BS: {
					//mazeme znak z bufferu
					if (pos > 0) pos--;

					registers.rax.h = static_cast<decltype(registers.rax.l)>(kiv_hal::NVGA_BIOS::Write_Control_Char);
					registers.rdx.l = ch;
					kiv_hal::Call_Interrupt_Handler(kiv_hal::NInterrupt::VGA_BIOS, registers);
				}
				break;

			case kiv_hal::NControl_Codes::LF:  break;	//jenom pohltime, ale necteme
			case kiv_hal::NControl_Codes::NUL:			//chyba cteni?
			case kiv_hal::NControl_Codes::EOT:			//konec textu
			case kiv_hal::NControl_Codes::CR:  return pos;	//docetli jsme az po Enter


			default: buffer[pos] = ch;
					 pos++;	
					 registers.rax.h = static_cast<decltype(registers.rax.l)>(kiv_hal::NVGA_BIOS::Write_String);
					 registers.rdx.r = reinterpret_cast<decltype(registers.rdx.r)>(&ch);
					 registers.rcx.r = 1;
					 kiv_hal::Call_Interrupt_Handler(kiv_hal::NInterrupt::VGA_BIOS, registers);
					 break;
		}
	}

	return pos;

}

kiv_os::THandle io::addIoHandle(IOHandle* handle) {
	ioHandleLock->lock();
	kiv_os::THandle result = 0;
	if (!openedHandles.empty()) {
		//auto it = openedHandles.begin();
		//iterate though handles until you find free handle (map is ordered)

		for (const auto& it : openedHandles)
		{
			if (it.first != result) {
				break;
			}
			result++;
		}

	/*	while (it->first == result) {
			//todo does it work though?
			++it;
			++result;
		}*/
	}

	openedHandles.emplace(std::make_pair(result, handle));

	ioHandleLock->unlock();
	return result;
}

IOHandle* io::getIoHandle(kiv_os::THandle handle) {
	ioHandleLock->lock();
	auto it = openedHandles.find(handle);
	IOHandle* result = nullptr;
	if (it != openedHandles.end()) {
		result = it->second;
	}
	ioHandleLock->unlock();
	return result;
}

void io::removeIoHandle(kiv_os::THandle handle) {
	ioHandleLock->lock();
	auto it = openedHandles.find(handle);
	if (it != openedHandles.end()) {
		it->second->close();
		delete it->second;
		openedHandles.erase(it);
	}
	ioHandleLock->unlock();
}

void io::removeAllIoHandles(){
	ioHandleLock->lock();
	auto it = openedHandles.begin();
	
	if (it != openedHandles.end()) {
		it->second->close();
		delete it->second;
		openedHandles.erase(it);
	}
	ioHandleLock->unlock();
}


void io::Handle_IO(kiv_hal::TRegisters &regs) {
	switch (static_cast<kiv_os::NOS_File_System>(regs.rax.l)) {
		case kiv_os::NOS_File_System::Open_File: {
			io::OpenIOHandle(regs);
			break;
		}
		case kiv_os::NOS_File_System::Write_File: {
			io::WriteIOHandle(regs);
			break;
		}
		case kiv_os::NOS_File_System::Read_File: {
			io::ReadIOHandle(regs);
			break;
		}
		case kiv_os::NOS_File_System::Seek: {
			io::SeekFsFile(regs);
			break;
		}
		case kiv_os::NOS_File_System::Close_Handle: {
			io::CloseIOHandle(regs);
			break;
		}
		case kiv_os::NOS_File_System::Delete_File: {
			io::DeleteFsFile(regs);
			break;
		}
		
		case kiv_os::NOS_File_System::Set_Working_Dir: {
			io::SetWorkingDirectory(regs);
			break;
		}
		case kiv_os::NOS_File_System::Get_Working_Dir: {
			io::GetWorkingDirectory(regs);
			break;
		}
		case kiv_os::NOS_File_System::Set_File_Attribute: {
			io::SetFileAttribute(regs);
			break;
		}
		case kiv_os::NOS_File_System::Get_File_Attribute: {
			io::GetFileAttribute(regs);
			break;
		}
		case kiv_os::NOS_File_System::Create_Pipe: {
			io::CreatePipe(regs);
			break;
		}
	}
}

void io::OpenIOHandle(kiv_hal::TRegisters& regs){
	//todo needed synch?
	char* file_name = reinterpret_cast<char*>(regs.rdx.r);
	
	auto flags = static_cast<kiv_os::NOpen_File>(regs.rcx.l);
	auto attributes = static_cast<uint8_t>(regs.rdi.i);
	/*auto processHandle = handles::getTHandleById(std::this_thread::get_id());
	/*auto process = processHandle == kiv_os::Invalid_Handle ? nullptr :
		ProcessUtils::pcb->getProcess(processHandle);
	*/kiv_os::NOS_Error returnCode = kiv_os::NOS_Error::Success;
	IOHandle* ioHandle = nullptr;
	//opening stdout?
	if (strcmp(file_name, "\\stdout\\") == 0) {
		ioHandle = new VGAHandle();
	}
	//opening stdin?
	else if (strcmp(file_name, "\\stdin\\") == 0) {
		ioHandle = new KeyboardHandle();
	}
	//opening file from fs
	else {
		std::filesystem::path inputPath = file_name;
		resolvePath(inputPath, file_name);
		ioHandle = filesystems::Open_File(inputPath.string().c_str(), flags, attributes, returnCode);
	}
	auto resultHandle = ioHandle == nullptr ? kiv_os::Invalid_Handle : io::addIoHandle(ioHandle);
	if (returnCode == kiv_os::NOS_Error::Success) {
		regs.rax.x = resultHandle;
	}
	else {
		regs.flags.carry = 1;
		regs.rax.x = static_cast<uint16_t>(returnCode);
	}
}

void io::WriteIOHandle(kiv_hal::TRegisters& regs){
	//get passed handle and its counterpart IOHandle
	kiv_os::THandle handle = regs.rdx.x;
	IOHandle* iohandle = io::getIoHandle(handle);
	if (iohandle != nullptr) {
		//get the passed arguments
		size_t size = static_cast<size_t>(regs.rcx.r);
		char* buffer = reinterpret_cast<char*>(regs.rdi.r);
		size_t writeCount;
		auto returnCode = iohandle->write(buffer, size, writeCount);
		
		if (returnCode == kiv_os::NOS_Error::Success) {
			regs.rax.r = writeCount;
		}else{
			regs.flags.carry = 1;
			regs.rax.r = static_cast<decltype(regs.rax.r)>(returnCode);
		}
	}
	else {
		//handle not opened
		regs.rax.r = static_cast<decltype(regs.rax.r)>(kiv_os::NOS_Error::File_Not_Found);
		regs.flags.carry = 1;
	}

}

void io::ReadIOHandle(kiv_hal::TRegisters& regs){
	//get passed handle and its counterpart IOHandle
	kiv_os::THandle handle = regs.rdx.x;
	IOHandle* ioHandle = io::getIoHandle(handle);
	if (ioHandle != nullptr) {
		//get the passed arguments
		char* buffer = reinterpret_cast<char*>(regs.rdi.r);
		auto size = static_cast<size_t>(regs.rcx.r);
		size_t readCount = 0;
		auto returnCode = ioHandle->read(size, buffer, readCount);
		if (returnCode == kiv_os::NOS_Error::Success) {
			regs.rax.r = readCount;
		}else{
			regs.rax.r = static_cast<decltype(regs.rax.r)>(returnCode);
			regs.flags.carry = 1;
		}
	}
	else {
		//handle not opened
		regs.rax.r = static_cast<uint64_t>(kiv_os::NOS_Error::File_Not_Found);
		regs.flags.carry = 1;
	}
}

void io::SeekFsFile(kiv_hal::TRegisters& regs){
	//get passed handle and its counterpart IOHandle
	kiv_os::THandle handle = regs.rdx.x;
	IOHandle* ioHandle = io::getIoHandle(handle);

	//error if everything fails
	kiv_os::NOS_Error errorCode = kiv_os::NOS_Error::Unknown_Error;

	//todo instanceof working?
	//non null and is handle of file
	if (ioHandle != nullptr && instanceof<FileHandle>(ioHandle)) {
		FileHandle* fileHandle = dynamic_cast<FileHandle*>(ioHandle);
		auto type = static_cast<kiv_os::NFile_Seek>(regs.rcx.l);
		auto operation = static_cast<kiv_os::NFile_Seek>(regs.rcx.h);
		auto position = static_cast<size_t>(regs.rdi.r);

		size_t resultPosition;

		errorCode = fileHandle->seek(position, type, operation, resultPosition);
		//is everything ok?
		if (errorCode == kiv_os::NOS_Error::Success) {
			if (operation == kiv_os::NFile_Seek::Get_Position) {
				regs.rax.r = resultPosition;
			}
			return;
		}
	}else {
		//handle not opened
		errorCode = kiv_os::NOS_Error::Invalid_Argument;
	}

	regs.rax.r = static_cast<uint64_t>(errorCode);
	regs.flags.carry = 1;
}

void io::CloseIOHandle(kiv_hal::TRegisters& regs){
	//get passed handle and its counterpart IOHandle
	kiv_os::THandle handle = regs.rdx.x;
	IOHandle* ioHandle = io::getIoHandle(handle);
	if (ioHandle) {
		// close and remove opened handle
		//ioHandle->close();
		//freeing of memory is done in the function
		io::removeIoHandle(handle);
	}
	else {
		regs.flags.carry = 1;
		regs.rax.r = static_cast<decltype(regs.rax.r)>(kiv_os::NOS_Error::File_Not_Found);
	}

}

void io::DeleteFsFile(kiv_hal::TRegisters& regs){
	char* fileName = reinterpret_cast<char*>(regs.rdx.r);
	std::filesystem::path inputPath = fileName;
	resolvePath(inputPath, fileName);
	auto fs = filesystems::Filesystem_exists(inputPath);
	//error if everything fails
	kiv_os::NOS_Error errorCode = kiv_os::NOS_Error::Unknown_Error;
	//todo handle relative path?
	if (fs) {
		if (fs->file_exist(inputPath.relative_path().string().c_str())) {
			errorCode = fs->rmdir(inputPath.relative_path().string().c_str());
			if (errorCode == kiv_os::NOS_Error::Success)return;
		}
		else {
			errorCode = kiv_os::NOS_Error::File_Not_Found;
		}
	}
	else {
		errorCode = kiv_os::NOS_Error::Unknown_Filesystem;
	}
	regs.rax.r = static_cast<uint64_t>(errorCode);
	regs.flags.carry = 1;
}

void io::SetWorkingDirectory(kiv_hal::TRegisters& regs){
	//todo synch
	auto path = reinterpret_cast<char*>(regs.rdx.r);
	std::filesystem::path inputPath = path;
	resolvePath(inputPath, path);
	auto fs = filesystems::Filesystem_exists(inputPath);
	//error if everything fails
	kiv_os::NOS_Error errorCode = kiv_os::NOS_Error::Unknown_Error;
	//todo handle relative path?
	if (fs) {
		if (inputPath.relative_path().empty() || fs->file_exist(inputPath.relative_path().string().c_str())) {
			auto processHandle = handles::getTHandleById(std::this_thread::get_id());
			auto process = processHandle == kiv_os::Invalid_Handle ? nullptr : 
				ProcessUtils::pcb->getProcess(processHandle);
			if (process) {
				process->workingDirectory = inputPath;
				//dir set -> everything ok
				return;
			}else {
				errorCode = kiv_os::NOS_Error::Unknown_Error;
			}
		}else {
			errorCode = kiv_os::NOS_Error::File_Not_Found;
		}
	}else {
		errorCode = kiv_os::NOS_Error::Unknown_Filesystem;
	}
	regs.rax.r = static_cast<uint64_t>(errorCode);
	regs.flags.carry = 1;
}
void io::GetWorkingDirectory(kiv_hal::TRegisters& regs) {
	//todo synch
	size_t size = static_cast<size_t>(regs.rcx.r);
	char* buffer = reinterpret_cast<char*>(regs.rdx.r);
	size_t writeCount;
	auto processHandle = handles::getTHandleById(std::this_thread::get_id());
	auto process = processHandle == kiv_os::Invalid_Handle ? nullptr :
		ProcessUtils::pcb->getProcess(processHandle);
	//error if everything fails
	kiv_os::NOS_Error errorCode = kiv_os::NOS_Error::Unknown_Error;
	if (process) {
		std::string wd = process->workingDirectory.string().c_str();
		if (size < wd.length()) {
			//something horrible happened -> small buffer
			errorCode = kiv_os::NOS_Error::Invalid_Argument;
		}else {
			auto error = strcpy_s(buffer, size, wd.c_str());
			if (error) {
				//error during copying
				errorCode = kiv_os::NOS_Error::IO_Error;
			}else {
				regs.rax.r = wd.length();
				//all written => all ok
				return;
			}
		}
	}

	regs.rax.r = static_cast<uint64_t>(errorCode);
	regs.flags.carry = 1;
}
void io::SetFileAttribute(kiv_hal::TRegisters& regs){
	char* file = reinterpret_cast<char*>(regs.rdx.r);
	auto attributes = static_cast<uint8_t>(regs.rdi.i);

	std::filesystem::path inputPath = file;
	resolvePath(inputPath, file);
	std::string fileName = inputPath.filename().string();
	auto fs = filesystems::Filesystem_exists(inputPath);
	//error if everything fails
	kiv_os::NOS_Error errorCode = kiv_os::NOS_Error::Unknown_Error;
	if (fs) {
		//toodo some kind of relative path
		errorCode = fs->set_file_attribute(inputPath.relative_path().string().c_str(), attributes);
		if (errorCode == kiv_os::NOS_Error::Success) {

			return;
		}
	}else {
		errorCode = kiv_os::NOS_Error::Unknown_Filesystem;
	}
	regs.rax.r = static_cast<uint64_t>(errorCode);
	regs.flags.carry = 1;

}

void io::GetFileAttribute(kiv_hal::TRegisters& regs){
	char* file = reinterpret_cast<char*>(regs.rdx.r);

	std::filesystem::path inputPath = file;
	resolvePath(inputPath, file);
	std::string fileName = inputPath.filename().string();
	auto fs = filesystems::Filesystem_exists(inputPath);
	//error if everything fails
	kiv_os::NOS_Error errorCode = kiv_os::NOS_Error::Unknown_Error;
	if (fs) {
		uint8_t attr;
		errorCode = fs->get_file_attribute(inputPath.relative_path().string().c_str(), attr);
		if (errorCode == kiv_os::NOS_Error::Success) {
			regs.rdi.i = static_cast<uint16_t>(attr);
			return;
		}
	}else {
		errorCode = kiv_os::NOS_Error::Unknown_Filesystem;
	}
	regs.rax.r = static_cast<uint64_t>(errorCode);
	regs.flags.carry = 1;
}

void io::CreatePipe(kiv_hal::TRegisters& regs){
	Pipe* pipe = new Pipe(1024);

	IOHandle* in = new PipeIn(pipe);
	IOHandle* out = new PipeOut(pipe);
	auto* pipeHandles = reinterpret_cast<kiv_os::THandle*>(regs.rdx.r);
	pipeHandles[0] = addIoHandle(in);
	pipeHandles[1] = addIoHandle(out);
}

void io::resolvePath(std::filesystem::path& resultPath, char* fileName){
	if (resultPath.is_relative()) {
		auto processHandle = handles::getTHandleById(std::this_thread::get_id());
		auto process = processHandle == kiv_os::Invalid_Handle ? nullptr :
			ProcessUtils::pcb->getProcess(processHandle);
		if (process) {
			auto absPath = process->workingDirectory;
			std::string result;
			filesystems::parse_path(absPath.string().c_str(), fileName, result);
			resultPath = result;
		}
	}

}
