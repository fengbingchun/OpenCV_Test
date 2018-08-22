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
cd ${new_dir_name}
cmake .. -DBUILD_MODE=${build_mode}
make

cd -

