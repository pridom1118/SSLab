obj-m := ch1_mod2_201811244.o

KBUILD_EXTRA_SYMBOLS := /home/pridom1118/Desktop/ess2023/week1/ex/mod1/Module.symvers
KERNELDIR := /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)

default:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules
clean:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) clean
