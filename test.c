#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/mman.h>
#include "cdata_ioctl.h"

int main(void)
{
	int fd;
	int i;
	char chardat[4]={0x30,0x31,0x32,0x33};
	fd=open("/dev/cdata",O_RDWR);
	ioctl(fd,IOCTL_EMPTY,NULL);
	//ioctl(fd,IOCTL_WRITE,&chardat[0]);
	//ioctl(fd,IOCTL_WRITE,&chardat[1]);
	//ioctl(fd,IOCTL_WRITE,&chardat[2]);
	//ioctl(fd,IOCTL_WRITE,&chardat[3]);
	ioctl(fd,IOCTL_SYNC,NULL);
	close(fd);
}
	
