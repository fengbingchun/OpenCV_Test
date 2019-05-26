#! /bin/bash

real_path=$(realpath $0)
dir_name=`dirname "${real_path}"`
echo "real_path: ${real_path}"
echo "dir_name: ${dir_name}"

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

# build five555
echo "########## start build live555 ##########"
live555_path=${dir_name}/../../src/live555
if [ -f ${live555_path}/liveMedia/libliveMedia.a ]; then
	echo "live555 has been builded"
else
	echo "live555 has not been builded yet, now start build"
	cd ${live555_path}
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


cd ${new_dir_name}
cmake ..
make

cd -

