MINEMU ?= minemu

.PHONY: all bootloader bootloader-check kernel kernel-examples user image clean

all: bootloader-check kernel user kernel-examples

bootloader:
	$(MAKE) -C bootloader all

bootloader-check:
	$(MAKE) -C bootloader check

kernel:
	$(MAKE) -C kernel all

kernel-examples:
	$(MAKE) -C kernel examples

user:
	$(MAKE) -C user all

image: kernel user
	$(MAKE) -C image MINEMU="$(MINEMU)" all

clean:
	$(MAKE) -C bootloader clean
	$(MAKE) -C kernel clean
	$(MAKE) -C user clean
	$(MAKE) -C image clean
