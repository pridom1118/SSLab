obj-m := ku_binder.o

KERNELDIR := /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)

default:
		$(MAKE) -C $(KERNELDIR) M=$(PWD) modules
		gcc -o $(obj-m:.o=_client) $(obj-m:.o=_client.c)
		gcc -o $(obj-m:.o=_server_vol) $(obj-m:.o=_server_vol.c)
		gcc -o $(obj-m:.o=_server_led) $(obj-m:.o=_server_led.c)
		sudo insmod ku_binder.ko
		sudo sh mknod.sh
clean:
		$(MAKE) -C $(KERNELDIR) M=$(PWD) clean
		rm $(obj-m:.o=_client) $(obj-m:.o=_server_vol) $(obj-m:.o=_server_led)
	    rm $(obj-m).ur-safe
	    sudo rmmod ku_binder
		sudo rm /dev/ku_binder_dev
