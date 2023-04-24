obj-m := ch2_mod_201811244.o

KERNELDIR := /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)

default:
		$(MAKE) -C $(KERNELDIR) M=$(PWD) modules
		gcc -o ch2_app_201811244 ch2_app_201811244.c
clean:
		$(MAKE) -C $(KERNELDIR) M=$(PWD) clean
		rm ch2_app_201811244
