obj-m := simple_block_1.o

KERNELDIR := /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)

default:
		$(MAKE) -C $(KERNELDIR) M=$(PWD) modules
		gcc -o $(obj-m:.o=_reader) $(obj-m:.o=_reader.c)
		gcc -o $(obj-m:.o=_writer) $(obj-m:.o=_writer.c)
clean:
		$(MAKE) -C $(KERNELDIR) M=$(PWD) clean
		rm $(obj-m:.o=_reader) 
		rm $(obj-m:.o=_writer)
		rm $(obj-m).ur-safe
