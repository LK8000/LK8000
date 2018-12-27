#!/bin/sh

set -ex

script_dir=$(dirname $(readlink -f $0))

docker build --rm -f "$script_dir/dockerfile" -t lk8000/lk8000:build $script_dir
docker run --rm -i -v $script_dir/../:/home/compiler -t lk8000/lk8000:build
