// A. Sheaff 2023/01/05
// Syscalls to make gcc be happy
//  We are not actually using any direct syscalls
#include <sys/types.h>

int _close_r(int fd)
{
	return 0;
}

off_t _lseek(int fd, off_t offset, int whence)
{
	return 0;
}
