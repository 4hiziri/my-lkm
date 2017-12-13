obj-m+=my-logger-mod.o

all:
	make -C /lib/modules/$(shell uname -r)/build/ M=$(PWD) modules
tester:
	gcc -o test-logger test-logger.c
clean:
	make -C /lib/modules/$(shell uname -r)/build/ M=$(PWD) clean; rm test-logger
