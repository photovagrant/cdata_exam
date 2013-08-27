#
# Author: jollen
# 
obj-m := cdata.o

#KDIR := /lib/modules/2.6.24-26-generic/build
KDIR := /lib/modules/3.8.0-19-generic/build
PWD := $(shell pwd)

default:
	$(MAKE) -C $(KDIR) SUBDIRS=$(PWD) modules

clean:
	rm -rf *.o *.ko .*cmd modules.* Module.* .tmp_versions *.mod.c
