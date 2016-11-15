#./configure --host=i386-pc-elf MACH=pc ARCH=i386
make
dd if=boot/boot.bin of=$HOME/big_disk.img bs=446 count=1 conv=notrunc
#sudo dd if=kern/vmaim.elf of=/dev/loop2p2 bs=512 count=100 conv=notrunc 
dd if=kern/vmaim.elf of=$HOME/big_disk.img bs=512 count=100 conv=notrunc seek=204800
