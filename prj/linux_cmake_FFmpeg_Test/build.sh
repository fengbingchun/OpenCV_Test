#! /bin/bash

real_path=$(realpath $0)
dir_name=`dirname "${real_path}"`
echo "real_path: ${real_path}"
echo "dir_name: ${dir_name}"

echo "##### manually install dependent libraries #####"
echo "sudo apt install nasm "
echo "sudo apt-get install libsdl2-2.0-0"
echo "sudo apt-get install libsdl2-dev"
echo "sudo apt-get install autoconf"
echo "sudo apt-get install liblzma-dev"
echo "sudo apt install libxcb-shm0-dev"
sleep 2

data_dir="test_images"
if [[ ! -d ${dir_name}/${data_dir} ]]; then
	echo "data directory does not exist: ${data_dir}"
	ln -s ${dir_name}/./../../${data_dir} ${dir_name}
else
	echo "data directory already exists: ${data_dir}"
fi

new_dir_name=${dir_name}/build
mkdir -p ${new_dir_name}

# build ffmpeg
echo "########## start build ffmpeg ##########"
ffmpeg_path=${dir_name}/../../src/ffmpeg
if [ -d ${ffmpeg_path}/build/install/lib ]; then
	echo "ffmpeg has been builded"
else
	echo "ffmpeg has not been builded yet, now start build"
	mkdir -p ${ffmpeg_path}/build
	cd ${ffmpeg_path}/build
	.././configure --prefix=./install
	make -j4
	make install
	cd -
fi
echo "########## finish build ffmpeg ##########"

cp -a ${ffmpeg_path}/build/install/lib/libavdevice.a ${new_dir_name}
cp -a ${ffmpeg_path}/build/install/lib/libavformat.a ${new_dir_name}
cp -a ${ffmpeg_path}/build/install/lib/libavutil.a ${new_dir_name}
cp -a ${ffmpeg_path}/build/install/lib/libavfilter.a ${new_dir_name}
cp -a ${ffmpeg_path}/build/install/lib/libswscale.a ${new_dir_name}
cp -a ${ffmpeg_path}/build/install/lib/libavcodec.a ${new_dir_name}
cp -a ${ffmpeg_path}/build/install/lib/libswresample.a ${new_dir_name}
cp -a ${ffmpeg_path}/build/install/bin/ffmpeg ${new_dir_name}
cp -a ${ffmpeg_path}/build/install/bin/ffprobe ${new_dir_name}
cp -a ${ffmpeg_path}/build/install/bin/ffplay ${new_dir_name}

# build five555
echo "########## start build live555 ##########"
live555_path=${dir_name}/../../src/live555
if [ -f ${live555_path}/liveMedia/libliveMedia.a ]; then
	echo "live555 has been builded"
else
	echo "live555 has not been builded yet, now start build"
	cd ${live555_path}
	chmod +x genMakefiles
	./genMakefiles linux-64bit
	make
	cd -
fi

cp -a ${live555_path}/BasicUsageEnvironment/libBasicUsageEnvironment.a ${new_dir_name}
cp -a ${live555_path}/UsageEnvironment/libUsageEnvironment.a ${new_dir_name}
cp -a ${live555_path}/liveMedia/libliveMedia.a ${new_dir_name}
cp -a ${live555_path}/groupsock/libgroupsock.a ${new_dir_name}
cp -a ${live555_path}/mediaServer/live555MediaServer ${new_dir_name}
cp -a ${live555_path}/proxyServer/live555ProxyServer ${new_dir_name}

# build libusb
echo "########## start build libusb ##########"
libusb_path=${dir_name}/../../src/libusb
if [ -f ${libusb_path}/libusb/.libs/libusb-1.0.a ]; then
	echo "libusb has been builded"
else
	echo "libusb has not been builded yet, now start build"
	cd ${libusb_path}
	./autogen.sh
	make
	cd -
fi

cp -a ${libusb_path}/libusb/.libs/libusb-1.0.a ${new_dir_name}

# build libuvc
echo "########## start build libuvc ##########"
libuvc_path=${dir_name}/../../src/libuvc
if [ -f ${libuvc_path}/build/libuvc.a ]; then
	echo "libuvc has been builded"
else
	echo "libuvc has not been builded yet, now start build"
	mkdir -p ${libuvc_path}/build
	cd ${libuvc_path}/build
	cmake ..
	make
	cd -
fi

cp -a ${libuvc_path}/build/libuvc.a ${new_dir_name}

rc=$?
if [[ ${rc} != 0 ]]; then
	echo "##### Error: some of thess commands have errors above, please check"
	exit ${rc}
fi

cd ${new_dir_name}
cmake -DOpenCV_DIR=/opt/opencv/4.8.1/release/lib/cmake/opencv4/ ..
make

cd -
