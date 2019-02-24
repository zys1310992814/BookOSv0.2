#Makefile for boot
#auther: Hu Zicheng
#time: 2019/1/24


#The tools name
NASM 		= nasm
CC			= gcc
LD 			= ld
DD 			= dd

#The file path
INCLUDE_DIR = ../../src/include/
LIB_DIR = ../../src/lib/

BIN = bin

BIN_OFF = 500
BIN_CNTS = 30

#img dir
IMG_DIR = ../../img/

FLAPPY_IMG = $(IMG_DIR)a.img

#flags
ASM_FLAGS	= -I $(INCLUDE_DIR) -f elf
C_FLAGS	= -I $(INCLUDE_DIR) -c -fno-builtin -Wall -Wunused

LDFLAGS		= -e _start -Ttext 0x80000000 --oformat binary

OBJS =  _start.o\
		main.o

LD_OBJS = $(LIB_DIR)printf.o\
		$(LIB_DIR)malloc.o\
		$(LIB_DIR)free.o\
		$(LIB_DIR)exit.o\
		$(LIB_DIR)getchar.o\
		$(LIB_DIR)putchar.o\
		$(LIB_DIR)write.o\
		$(LIB_DIR)vsprintf.o\
		$(LIB_DIR)string.o\
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
		$(LIB_DIR)wait.o\
		$(LIB_DIR)execv.o\
		$(LIB_DIR)clear.o\
		$(LIB_DIR)ps.o\
		$(LIB_DIR)reboot.o\
		$(LIB_DIR)access.o\
		$(LIB_DIR)gettime.o\
		$(LIB_DIR)mm.o\
		
#First read here
.PHONY: all

all: compile link disk

#Compile files
compile: $(OBJS)

link: $(BIN)
$(BIN): $(OBJS) $(LD_OBJS)
	$(LD) $(LDFLAGS) -o $(BIN) $(OBJS) $(LD_OBJS)

disk:
	$(DD) if=$(BIN) of=$(FLAPPY_IMG) bs=512 seek=$(BIN_OFF) count=$(BIN_CNTS) conv=notrunc
	

	
#Clean temporary files
clean:
	-rm *.o
	-rm bin

%.o: %.asm
	$(NASM) $(ASM_FLAGS) -o $@ $<
	
%.o: %.c
	$(CC) $(C_FLAGS) -o $@ $<
	