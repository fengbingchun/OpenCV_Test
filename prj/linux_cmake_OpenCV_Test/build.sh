#! /bin/bash

build_mode=release
if [ $# == 1 ]; then
	build_mode=debug
fi
echo "build mode: ${build_mode}"

real_path=$(realpath $0)
dir_name=`dirname "${real_path}"`
echo "real_path: ${real_path}, dir_name: ${dir_name}"

data_dir="test_images"
if [ -d ${dir_name}/${data_dir} ]; then
	rm -rf ${dir_name}/${data_dir}
fi

ln -s ${dir_name}/./../../${data_dir} ${dir_name}

new_dir_name=${dir_name}/build
mkdir -p ${new_dir_name}

rc=$?
if [[ ${rc} != 0 ]]; then
	echo "##### Error: some of thess commands have errors above, please check"
	exit ${rc}
fi

cd ${new_dir_name}
if [ $# == 1 ]; then
	cmake -DOpenCV_DIR=/opt/opencv/4.8.1/debug/lib/cmake/opencv4/ -DBUILD_MODE=${build_mode} ..
else
	cmake -DOpenCV_DIR=/opt/opencv/4.8.1/release/lib/cmake/opencv4/ -DBUILD_MODE=${build_mode} ..
fi
make

cd -
