gnome-terminal -x zsh -c 'cd ~/AIM-public; sh write_disk.sh; qemu-system-i386 $HOME/big_disk.img -gdb tcp::1234 -S;' &''
gnome-terminal -x bash -c "cd ~/AIM-public; $CROSS_GDB kern/vmaim.elf"
