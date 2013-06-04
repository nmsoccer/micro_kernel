#######################
# Makefile for Leming's OS
######################


# Entry point of kernel
ENTRYPOINT = 0x1400

# Program flags , etc

NASM = nasm
NASMBHF = -I boot/include/
NASMKHF = -I include/
NASMOBJ = -o
NASMELF = -f elf

CC = gcc
CFLAGS = -I include/ -c -fno-builtin
COBJ = -o

LD = ld
LDELF = -s -Ttext $(ENTRYPOINT) -o

# Program target
IMG = b.img

BOOT = boot/boot.bin
LOADER = boot/loader.bin
KERNELOBJ = kernel/kernel.o
KERNEL = kernel/kernel.bin

#This could be expanded
KLIBOBJ=kernel/start.o kernel/i8259.o kernel/global.o kernel/protect.o kernel/initidt.o kernel/main.o kernel/exam.o \
kernel/initpcb.o kernel/clock.o kernel/syscall.o kernel/process.o kernel/schedule.o kernel/cglobal.o kernel/keyboard.o kernel/tty.o \
kernel/console.o kernel/message.o kernel/hd.o kernel/fs/fs.o kernel/command.o 
LIBOBJ = lib/klibc.o lib/string.o lib/dispstr.o lib/memopt.o lib/dispint.o lib/assist.o lib/stdio.o lib/stdlib.o lib/signal.o \
lib/unistd.o lib/fcntl.o lib/stat.o

# Action below
.PHONY: everything build copy clean kmake kcopy kbuild kclean
everything: $(BOOT) $(LOADER) $(KERNEL)

copy:
	dd if=$(BOOT) of=$(IMG) bs=512 count=1 conv=notrunc
	sudo mount -o loop $(IMG) /mnt/floppy
	sudo cp -fv $(LOADER) /mnt/floppy
	sudo cp -fv $(KERNEL) /mnt/floppy
	sudo umount /mnt/floppy

clean:
	rm -rf $(BOOT) boot/*~
	rm -rf $(LOADER)
	rm -rf $(KERNEL) kernel/*~
	rm -rf $(KERNELOBJ)
	rm -rf $(KLIBOBJ) $(LIBOBJ) lib/*~
	rm -rf include/*~

build: everything copy clean
# Only to kernel
kmake: $(KERNEL)
kcopy:
	sudo mount -o loop $(IMG) /mnt/floppy
	sudo cp -fv $(KERNEL) /mnt/floppy
	sudo umount /mnt/floppy		
	 
kclean:
	rm -rf $(KERNEL)  $(KERNELOBJ) kernel/*~
	rm -rf $(KLIBOBJ) $(LIBOBJ) lib/*~
	rm -rf include/*~	

kbuild: kmake kcopy kclean

$(BOOT):boot/boot.asm
	$(NASM) $(NASMBHF) $< $(NASMOBJ) $@
$(LOADER):boot/loader.asm
	$(NASM) $(NASMBHF) $< $(NASMOBJ) $@
$(KERNELOBJ):kernel/kernel.asm
	$(NASM) $(NASMKHF) $(NASMELF) $< $(NASMOBJ) $@
$(KERNEL):$(KERNELOBJ) $(KLIBOBJ) $(LIBOBJ)
	$(LD) $(LDELF) $(KERNEL) $(KERNELOBJ) $(KLIBOBJ) $(LIBOBJ)

kernel/start.o:kernel/start.asm
	$(NASM) $(NASMKHF) $(NASMELF) $< $(NASMOBJ) $@
kernel/i8259.o:kernel/i8259.asm
	$(NASM) $(NASMKHF) $(NASMELF) $< $(NASMOBJ) $@
kernel/global.o:kernel/global.asm
	$(NASM) $(NASMKHF) $(NASMELF) $< $(NASMOBJ) $@
kernel/protect.o:kernel/protect.asm
	$(NASM) $(NASMKHF) $(NASMELF) $< $(NASMOBJ) $@
kernel/initidt.o:kernel/initidt.asm
	$(NASM) $(NASMKHF) $(NASMELF) $< $(NASMOBJ) $@
kernel/initpcb.o:kernel/initpcb.asm
	$(NASM) $(NASMKHF) $(NASMELF) $< $(NASMOBJ) $@
kernel/clock.o:kernel/clock.asm
	$(NASM) $(NASMKHF) $(NASMELF) $< $(NASMOBJ) $@
kernel/syscall.o:kernel/syscall.asm
	$(NASM) $(NASMKHF) $(NASMELF) $< $(NASMOBJ) $@
kernel/exam.o:kernel/exam.c
	$(CC) $(CFLAGS) $< $(COBJ) $@
kernel/main.o:kernel/main.c
	$(CC) $(CFLAGS) $< $(COBJ) $@	
kernel/process.o:kernel/process.c
	$(CC) $(CFLAGS) $< $(COBJ) $@
kernel/schedule.o:kernel/schedule.c
	$(CC) $(CFLAGS) $< $(COBJ) $@	
kernel/cglobal.o:kernel/cglobal.c
	$(CC) $(CFLAGS) $< $(COBJ) $@	
kernel/keyboard.o:kernel/keyboard.c
	$(CC) $(CFLAGS) $< $(COBJ) $@
kernel/tty.o:kernel/tty.c
	$(CC) $(CFLAGS) $< $(COBJ) $@
kernel/console.o:kernel/console.c
	$(CC) $(CFLAGS) $< $(COBJ) $@
kernel/message.o:kernel/message.c
	$(CC) $(CFLAGS) $< $(COBJ) $@
kernel/hd.o:kernel/hd.c
	$(CC) $(CFLAGS) $< $(COBJ) $@
kernel/fs/fs.o:kernel/fs/fs.c
	$(CC) $(CFLAGS) $< $(COBJ) $@
kernel/command.o:kernel/command.c
	$(CC) $(CFLAGS) $< $(COBJ) $@			
lib/klibc.o:lib/klibc.c
	$(CC) $(CFLAGS) $< $(COBJ) $@
lib/string.o:lib/string.c
	$(CC) $(CFLAGS) $< $(COBJ) $@		
lib/stdio.o:lib/stdio.c
	$(CC) $(CFLAGS) $< $(COBJ) $@
lib/stdlib.o:lib/stdlib.c
	$(CC) $(CFLAGS) $< $(COBJ) $@
lib/signal.o:lib/signal.c
	$(CC) $(CFLAGS) $< $(COBJ) $@
lib/unistd.o:lib/unistd.c
	$(CC) $(CFLAGS) $< $(COBJ) $@
lib/fcntl.o:lib/fcntl.c
	$(CC) $(CFLAGS) $< $(COBJ) $@
lib/stat.o:lib/stat.c
	$(CC) $(CFLAGS) $< $(COBJ) $@						
lib/dispstr.o:lib/dispstr.asm
	$(NASM) $(NASMKHF) $(NASMELF) $< $(NASMOBJ) $@
lib/memopt.o:lib/memopt.asm
	$(NASM) $(NASMKHF) $(NASMELF) $< $(NASMOBJ) $@
lib/dispint.o:lib/dispint.asm
	$(NASM) $(NASMKHF) $(NASMELF) $< $(NASMOBJ) $@
lib/assist.o:lib/assist.asm
	$(NASM) $(NASMKHF) $(NASMELF) $< $(NASMOBJ) $@


