BUILD_DIR?=/home/jimchan/Desktop/workingOS/build
ASM?=nasm


.PHONY: all clean

all: kernel

kernel: $(BUILD_DIR)/kernel.bin

$(BUILD_DIR)/kernel.bin:
	$(ASM) main.asm -f bin -o $(BUILD_DIR)/kernel.bin


clean:
	rm -f $(BUILD_DIR)/kernel.bin

