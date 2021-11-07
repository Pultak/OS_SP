
#include <vector>
#include "../api/api.h"
#include <string>

class VFS {
public:
	virtual kiv_os::NOS_Error mkdir(const char* name, uint8_t attr) = 0;

	virtual int rmdir(const char* name) = 0;

};