#!/usr/bin/env bash
BUILD_PATH="$(dirname $0)/build"

apt update -y
apt upgrade -y
apt install -y sed gzip zip unzip
apt install -y gcc g++ make cmake
apt install -y curl libcurl4-openssl-dev libxml2-dev libcmocka-dev html-xml-utils

cd $BUILD_PATH
cmake ..
make
cp "./bin/knudown" "/usr/local/bin"