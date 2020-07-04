obj-m +=  swat.o
swat-objs := src/swat.o src/syscall_wrapper.o src/copy_to_user_wrapper.o src/utils.o

ccflags-y += -I $(PWD)/include/

all:
	make -C ../linux-5.4.1/ M=$(PWD) modules

clean:
	make -C ../linux-5.4.1/ M=$(PWD) clean
