# knu-lms-auto-download

2021년 1학기 시스템 프로그래밍(ELEC462003) 기말 프로젝트  

경북대학교 lms 강의자료/과제 자동 다운로드 프로그램

## 요구사항
- 운영체제: Linux Ubuntu 20.04 or WSL2

## 빌드방법

```shell
sudo apt update -y
sudo apt upgrade -y
sudo apt install -y gcc g++ make cmake
sudo apt install -y curl html-xml-utils

cwd=$(pwd)
cd /tmp
git clone https://github.com/cgreen-devs/cgreen.git ./cgreen-build
cd cgreen-build
make
make test
make install
sudo ldconfig
cd $cwd

git clone https://github.com/Insik-Won/knu-lms-auto-download.git
cd knu-lms-auto-download/build
cmake ..
make
cd $cwd

unset cwd
```

## 실행방법

```shell
./knu-lms-auto-download/build/bin/program
```
