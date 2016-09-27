#./configure --host=i386-pc-elf MACH=pc ARCH=i386
make
dd if=boot/boot.bin of=$HOME/big_disk.img bs=446 count=1 conv=notrunc
dd if=kern/vmaim.elf of=$HOME/big_disk.img bs=512 count=1 conv=notrunc seek=204801

#qemu-system-i386 big_disk.img -gdb tcp::1234 -S