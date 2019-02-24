######
#Makefile for BookOS
#File: Makefile
#Contains: makefile for all
#Auther: Hu Zicheng
#Time: 2019/1/20
#Copyright:	(C) 2018-2019 by BookOS developers. All rights reserved.
#E-mail: 2323168280@qq.com
#history:
#-v0.1	2019/1/20
#-v0.2	2019/2/20
######

#tool dir
BOCHS_DIR = ../bochs/

NASM 		= nasm
CC			= gcc
LD 			= ld
DD 			= dd
QEMU 		= qemu-system-i386
BOCHS 		= bochs
BXRC 		= $(BOCHS_DIR)bochsrc.bxrc

#img dir
IMG_DIR = ../img/

FLAPPY_IMG 	= $(IMG_DIR)a.img
HDA_IMG		= $(IMG_DIR)c.img


#flags

LDFLAGS		= -m elf_i386 -e _start -Ttext 0x100000
#system disk
LOADER_OFF = 2		
LOADER_CNTS = 8

KERNEL_OFF = 10
KERNEL_CNTS = 400

#src dir
BOOT_DIR = ./boot/
KERNEL_DIR = ./kernel/
INIT_DIR = ./init/
LIB_DIR = ./lib/
DEVICE_DIR = ./device/
GUI_DIR = ./gui/
FS_DIR = ./fs/

BOOT_BIN = $(BOOT_DIR)boot.bin
LOADER_BIN = $(BOOT_DIR)loader.bin
KERNEL_ELF = $(KERNEL_DIR)kernel.elf

#objs
OBJS =  $(KERNEL_DIR)_start.o\
		$(KERNEL_DIR)x86.o\
		$(KERNEL_DIR)descriptor.o\
		$(KERNEL_DIR)8259a.o\
		$(KERNEL_DIR)interrupt.o\
		$(KERNEL_DIR)console.o\
		$(KERNEL_DIR)bitmap.o\
		$(KERNEL_DIR)page.o\
		$(KERNEL_DIR)kernel.o\
		$(KERNEL_DIR)memory.o\
		$(KERNEL_DIR)ards.o\
		$(KERNEL_DIR)cpu.o\
		$(KERNEL_DIR)ioqueue.o\
		$(KERNEL_DIR)debug.o\
		$(KERNEL_DIR)cmos.o\
		$(KERNEL_DIR)syscall.o\
		$(KERNEL_DIR)thread.o\
		$(KERNEL_DIR)sync.o\
		$(KERNEL_DIR)tss.o\
		$(KERNEL_DIR)process.o\
		$(KERNEL_DIR)system.o\
		$(KERNEL_DIR)pipe.o\
		$(FS_DIR)dir.o\
		$(FS_DIR)fatxe.o\
		$(FS_DIR)fat.o\
		$(FS_DIR)file.o\
		$(INIT_DIR)main.o\
		$(GUI_DIR)main.o\
		$(GUI_DIR)graphic.o\
		$(GUI_DIR)font.o\
		$(GUI_DIR)ft_simsun.o\
		$(GUI_DIR)ft_standard.o\
		$(GUI_DIR)image.o\
		$(DEVICE_DIR)vga.o\
		$(DEVICE_DIR)clock.o\
		$(DEVICE_DIR)keyboard.o\
		$(DEVICE_DIR)harddisk.o\
		$(DEVICE_DIR)video.o\
		$(DEVICE_DIR)ramdisk.o\
		$(DEVICE_DIR)mouse.o\
		$(LIB_DIR)string.o\
		$(LIB_DIR)vsprintf.o\
		$(LIB_DIR)math.o\
		$(LIB_DIR)printf.o\
		$(LIB_DIR)malloc.o\
		$(LIB_DIR)free.o\
		$(LIB_DIR)exit.o\
		$(LIB_DIR)getchar.o\
		$(LIB_DIR)putchar.o\
		$(LIB_DIR)write.o\
		$(LIB_DIR)execv.o\
		$(LIB_DIR)wait.o\
		$(LIB_DIR)fopen.o\
		$(LIB_DIR)fclose.o\
		$(LIB_DIR)fread.o\
		$(LIB_DIR)fwrite.o\
		$(LIB_DIR)fstat.o\
		$(LIB_DIR)lseek.o\
		$(LIB_DIR)unlink.o\
		$(LIB_DIR)opendir.o\
		$(LIB_DIR)closedir.o\
		$(LIB_DIR)readdir.o\
		$(LIB_DIR)rewinddir.o\
		$(LIB_DIR)mkdir.o\
		$(LIB_DIR)rmdir.o\
		$(LIB_DIR)rename.o\
		$(LIB_DIR)copy.o\
		$(LIB_DIR)move.o\
		$(LIB_DIR)getcwd.o\
		$(LIB_DIR)chdir.o\
		$(LIB_DIR)clear.o\
		$(LIB_DIR)ps.o\
		$(LIB_DIR)reboot.o\
		$(LIB_DIR)random.o\
		$(LIB_DIR)access.o\
		$(LIB_DIR)gettime.o\
		$(LIB_DIR)graphic.o\
		$(LIB_DIR)mouse.o\
		$(LIB_DIR)igl.o\
		$(LIB_DIR)video.o\
		$(LIB_DIR)mm.o\
		$(LIB_DIR)pipe.o\
		
all: compile link disk qemu

cld: compile link disk


#compile file
compile:
	cd $(BOOT_DIR) && $(MAKE)
	cd $(KERNEL_DIR) && $(MAKE)
	cd $(INIT_DIR) && $(MAKE)
	cd $(LIB_DIR) && $(MAKE)
	cd $(DEVICE_DIR) && $(MAKE)
	cd $(GUI_DIR) && $(MAKE)
	cd $(FS_DIR) && $(MAKE)
	
link: $(KERNEL_ELF)

$(KERNEL_ELF): $(OBJS)
	$(LD) $(LDFLAGS) -o $(KERNEL_ELF) $(OBJS)
	
#write file into disk
disk:
	$(DD) if=$(BOOT_BIN) of=$(FLAPPY_IMG) bs=512 count=1 conv=notrunc
	$(DD) if=$(LOADER_BIN) of=$(FLAPPY_IMG) bs=512 seek=$(LOADER_OFF) count=$(LOADER_CNTS) conv=notrunc
	$(DD) if=$(KERNEL_ELF) of=$(FLAPPY_IMG) bs=512 seek=$(KERNEL_OFF) count=$(KERNEL_CNTS) conv=notrunc

#run kernel in vm
bochs:
	$(BOCHS) -q -f $(BXRC)

qemu:
	$(QEMU) -m 64 -fda $(FLAPPY_IMG) -hda $(HDA_IMG) -boot a
	
#clean temporary files
clean:
	cd $(BOOT_DIR) && $(MAKE) clean
	cd $(KERNEL_DIR) && $(MAKE) clean
	cd $(INIT_DIR) && $(MAKE) clean
	cd $(LIB_DIR) && $(MAKE) clean
	cd $(DEVICE_DIR) && $(MAKE) clean
	cd $(GUI_DIR) && $(MAKE) clean
	cd $(FS_DIR) && $(MAKE) clean
	