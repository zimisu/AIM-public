dd if=/dev/zero of=$HOME/big_disk.img bs=512 count=1024000
sudo losetup /dev/loop0 $HOME/big_disk.img
#sudo kpartx -av /dev/loop1
