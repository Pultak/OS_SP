#include"dir.h"

bool Set_Working_Dir(kiv_hal::TRegisters& regs) {
    auto path = reinterpret_cast<char*>(regs.rdx.r);
    
    if (path != nullptr) {
        std::filesystem::path f_path = path;
       
        std::filesystem::path resolved_path_relative_to_fs;
        std::filesystem::path absolute_path;
        
        if (filesystems::Filesystem_exists(f_path) != nullptr) {
            return true;
        }
    }
    return false;
}

bool Get_Working_Dir(kiv_hal::TRegisters& regs) {
    char* out_buffer = reinterpret_cast<char*>(regs.rdx.r);
    size_t buffer_size = static_cast<size_t>(regs.rcx.r);
    size_t written;

    if (Get_Working_Dir(out_buffer, buffer_size, written)) {
        
        regs.rax.r = written;
        if (written > 0) {
            return true;
        }
    }

    return false;
}

bool Get_Working_Dir(char* out_buffer, size_t buffer_size, size_t& written) {
    return true;
    
}
    
