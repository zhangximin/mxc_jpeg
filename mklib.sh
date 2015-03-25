#/bin/sh

if [ -f Makefile ];then

if [ -f Makefile.simon ];then

export POKY_ROOT=/opt/poky/1.7.1

source ${POKY_ROOT}/environment-setup-cortexa9hf-vfp-neon-poky-linux-gnueabi

mv Makefile Makefile.liyuan
mv Makefile.simon Makefile

make

if [ -f libmxc_jpeg.so ];then
${POKY_ROOT}/sysroots/x86_64-pokysdk-linux/usr/bin/arm-poky-linux-gnueabi/arm-poky-linux-gnueabi-strip libmxc_jpeg.so

if [ -d "../rootfs" ];then
sudo cp mxc_jpeg.h ../rootfs/usr/include
sudo cp libmxc_jpeg.so ../rootfs/usr/lib
cp mxc_jpeg.h ~/fsl-community-bsp/nbuild/tmp/sysroots/imx6qsabrelite/usr/include/
cp libmxc_jpeg.so ~/fsl-community-bsp/nbuild/tmp/sysroots/imx6qsabrelite/usr/lib/
else
echo ""
echo ""
echo "#####################################"
echo "Remember following two steps:"
echo "  1.copy mxc_jpeg.h to /usr/include\n"
echo "  2.copy libmxc_jpeg.so to /usr/lib\n"
echo ""
fi

if [ -d "~/fsl-community-bsp/nbuild/tmp/sysroots/imx6qsabrelite/usr/" ];then
cp mxc_jpeg.h ~/fsl-community-bsp/nbuild/tmp/sysroots/imx6qsabrelite/usr/include/
cp libmxc_jpeg.so ~/fsl-community-bsp/nbuild/tmp/sysroots/imx6qsabrelite/usr/lib/
fi

fi

mv Makefile Makefile.simon
mv Makefile.liyuan Makefile

else
echo "Error:Makefile.simon not found!"
fi

else
echo "Error:Makefile not found!"
fi

echo "Done!"

