#!/usr/bin/env bash
SCRIPTS=( "개인정보" "메인" "과목설정" "강의자료목록" "강의자료파일목록" "강의자료파일다운로드" "로그아웃")

echo -e "로그인 시작\n"
./로그인.sh
echo -e "로그인 끝\n"

for script in ${SCRIPTS[@]}
do
  echo -e "${script} 시작\n"
  ./$script.sh
  echo -e "끝 : ${script}\n"
done
