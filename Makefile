obj-m += swat.o

all:
	make -C ../linux-5.4.1/ M=$(PWD) modules

clean:
	make -C ../linux-5.4.1/ M=$(PWD) clean
