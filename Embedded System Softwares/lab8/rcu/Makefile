obj-m := simple_rcu.o

KDIR := ~/linux-rpi/
ARM := ARCH=arm CROSS_COMPILE=/usr/bin/arm-linux-gnueabi-
PWD := $(shell pwd)

default:
		$(MAKE) -C $(KDIR) M=$(PWD) $(ARM) modules
		arm-linux-gnueabi-gcc $(obj-m:.o=_app.c) -o $(obj-m:.o=_app)
clean:
		$(MAKE) -C $(KDIR) M=$(PWD) $(ARM) clean
		rm $(obj-m:.o=_app)
scp:
		scp $(obj-m:.o=.ko) mknod.sh $(obj-m:.o=_app) pi@10.10.10.12:~/
