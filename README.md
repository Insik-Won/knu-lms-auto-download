# knu-lms-auto-download

2021년 1학기 시스템 프로그래밍(ELEC462003) 기말 프로젝트  

제목: 경북대학교 lms 강의자료/과제 자동 다운로드 프로그램

# 요구사항

- 운영체제: Linux Ubuntu 20.04 or WSL2 Ubuntu 20.04

## 사용 스택

- vscode : 통합 개발 환경
- make, cmake : 프로젝트 빌드 툴
- libcurl : html 요청
- json-c : json 문서 처리
- libxml2 : xml 문서 처리
- cmocka : 단위 테스트
- html-xml-utils 패키지
  - hxnormalize 명령어: html을 xml로 정규화
  - hxselect 명령어: css 선택자로 html 요소 찾기
- sed, tr 명령어: html 정규화 과정에서 예외 처리
- gunzip, unzip 명령어: 압축파일 해제


## 디렉토리 구조

```shell
$ tree
.  
├── CMakeLists.txt      # 최상위 CMakeLists.txt
├── README.md           # github README 마크다운 문서
├── app                 
│   ├── CMakeLists.txt  # 실행파일 CMakeLists.txt
│   └── main.c          # main.c: 실행파일 진입점 소스코드
├── build               # 빌드 디렉토리. ./bin에 app과 test가, ./lib에 정적 라이브러리 파일이 들어가 있다.
├── includes            
│   ├── knuapi.h        # knuapi.h: lms 크롤링의 각 단계를 래핑한 라이브러리
│   ├── knudef.h        # knudef.h: 라이브러리 전체에 쓰일 구조체/함수/상수 선언
│   ├── knulms.h        # knulms.h: lms 크롤링을 대폭 추상화해 쉽게 사용할 수 있도록 만든 라이브러리
│   ├── knustring.h     # knustring.h: 라이브러리 전체에 쓰일 문자열 구조체, KnuString를 정의하는 라이브러리
│   ├── knuutils.h      # knuutils.h: 여러 유용한 함수를 모아놓은 라이브러리.
│   └── test_util.h     # test_util.h: 단위 테스트할 때 쓸 유용한 함수를 모아놓은 라이브러리
├── lib
│   ├── CMakeLists.txt  # 라이브러리 CMakeLists.txt
│   ├── knuapi.c        # knuapi.h 정의
│   ├── knudef.c        # knudef.h 정의
│   ├── knulms.c        # knulms.h 정의
│   ├── knustring.c     # knustring.h 정의
│   └── knuutils.c      # knuutils.h 정의
├── tests
│   ├── CMakeLists.txt  # 단위 테스트 CmakeLists.txt
│   ├── resources       # 단위 테스트에 사용할 리소스 파일
│   │   └── ...
│   ├── test_knuapi.c   # knuapi 라이브러리를 단위 테스트하는 파일
│   ├── test_knustring.c  # knustring 라이브러리를 단위 테스트하는 파일
│   ├── test_knuutils.c # knuutils 라이브러리를 단위 테스트하는 파일
│   └── test_util.c     # test_util.h 정의
└── 기말 프로젝트
    ├── 2021113585 원인식 프로젝트 계획서.txt # 계획서
    ├── http 요청
    │   └── ...           # 경북대 lms 크롤링을 위해 만든 curl 명령 쉘 파일
    ├── 프로젝트 계획.txt  # 프로젝트 초안 및 기획
    ├── readme.txt        # 사용법
    └── 프로젝트 보고서.docx  # 프로젝트 보고서 - 프로젝트 개요, 상세구조 및 동작 흐름도, 결과, 참고자료
```

## 설치방법
```shell
git clone https://github.com/Insik-Won/knu-lms-auto-download.git
chmod +x ./install.sh
sudo ./install.sh

knudown [options] [subject_name] [output_directory]
```

## 삭제방법
```shell
chmod +x ./uninstall.sh
sudo ./uninstall.sh
```

## 빌드방법

```shell
sudo apt update -y
sudo apt upgrade -y
sudo apt install -y sed tr gzip gunzip zip unzip
sudo apt install -y gcc g++ make cmake
sudo apt install -y curl libcurl4-openssl-dev libxml2-dev libcmocka-dev html-xml-utils

cwd=$(pwd)
git clone https://github.com/Insik-Won/knu-lms-auto-download.git
cd knu-lms-auto-download/build
cmake ..
make
cd $cwd

unset cwd
```

## 실행방법 : 실행파일

```shell
./knu-lms-auto-download/build/bin/knudown [options] [subject_name] [output_directory]
```

## 실행방법 : 테스트

1. tests/resources 디렉토리에 .env 파일을 만들고 다음과 같이 쓴다.
```text
USER_ID={시스템프로그래밍 003을 수강하는 학생의 통합 로그인 ID}
PASSWORD={해당 학생의 통합 로그인 비밀번호}
STUDENT_NUMBER={해당 학생의 경북대 학번}
```

2. 다음 명령어를 입력한다.
```shell
./knu-lms-auto-download/build/bin/test
```
