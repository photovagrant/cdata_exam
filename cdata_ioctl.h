#ifndef _CDATA_IOCTL_H_
#define _CDATA_IOCTL_H_

#include <linux/ioctl.h>

#define IOCTL_EMPTY	_IO(0xCE,0)
#define IOCTL_SYNC	_IO(0xCE,2)
#define IOCTL_WRITE	_IO(0xCE,1)

#endif
