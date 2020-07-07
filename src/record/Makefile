obj-m += record.o
record-objs := main.o syscall_wrapper.o copy_to_user_wrapper.o utils.o syscall_dumper.o

ccflags-y += -I $(PWD)/include/

all:
	make -C $(BUILD_DIR) M=$(CURDIR) modules

clean:
	make -C  $(PWD)/../linux-5.4.1 M=$(CURDIR) clean