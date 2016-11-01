gnome-terminal -x zsh -c 'cd ~/AIM-public; sh write_disk.sh; qemu-system-i386 $HOME/big_disk.img -serial stdio;' &''
