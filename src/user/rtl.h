#pragma once

#include "..\api\api.h"
#include <atomic>

namespace kiv_os_rtl{

	extern std::atomic<kiv_os::NOS_Error> Last_Error;

	bool Read_File(const kiv_os::THandle file_handle, char* const buffer, const size_t buffer_size, size_t& read);
	//zapise do souboru identifikovaneho deskriptor data z buffer o velikosti buffer_size a vrati pocet zapsanych dat ve written
	//vraci true, kdyz vse OK
	//vraci true, kdyz vse OK

	bool Write_File(const kiv_os::THandle file_handle, const char* buffer, const size_t buffer_size, size_t& written);
	//zapise do souboru identifikovaneho deskriptor data z buffer o velikosti buffer_size a vrati pocet zapsanych dat ve written
	//vraci true, kdyz vse OK
	//vraci true, kdyz vse OK

	bool Set_Working_Dir(const char* dir);

	bool Get_Working_Dir(char* buffer, const size_t buffer_size, size_t& chars_written);

	bool Open_File(const char* file_name, kiv_os::NOpen_File file_open, kiv_os::NFile_Attributes file_attribute, kiv_os::THandle& file_handle_ret);

	bool Seek(kiv_os::THandle file_handle, const uint16_t position, kiv_os::NFile_Seek file_seek_pos, kiv_os::NFile_Seek file_seek_op, uint64_t & position_ret);
	
	bool Close_Handle(kiv_os::THandle handle);

	bool Delete_File(const char* file_name);

	bool Set_File_Attribute(char* file_name, kiv_os::NFile_Attributes file_attribute);

	bool Get_File_Attribute(char* file_name, uint64_t& file_attribute_ret);

	bool Create_Pipe(kiv_os::THandle * file_handles);

	bool Create_Process(const char* process_name,const char* argument, kiv_os::THandle stdin_handle, kiv_os::THandle stdout_handle, kiv_os::THandle& process_handle_ret);
	
	bool Create_Thread(kiv_os::TThread_Proc thread_name, const char* argument, kiv_os::THandle stdin_handle, kiv_os::THandle stdout_handle, kiv_os::THandle& thread_handle_ret);
	
	bool Wait_For(kiv_os::THandle* handles_to_wait, uint16_t num_of_handles, kiv_os::THandle& handle_signal_ret);
				
	bool Read_Exit_Code(kiv_os::THandle process_handle, kiv_os::NOS_Error& exit_code_ret);

	bool Exit(kiv_os::NOS_Error exit_code);
	
	bool Shutdown();
	
	bool Register_Signal_Handler(kiv_os::NSignal_Id signal_id, kiv_os::THandle process_handle);
}