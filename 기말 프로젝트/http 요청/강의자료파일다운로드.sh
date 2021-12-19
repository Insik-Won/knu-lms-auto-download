#!/usr/bin/env bash
curl -v -L --referer ";auto" -X GET --cookie cookie.txt --cookie-jar cookie.txt "https://lms.knu.ac.kr/ilos/pf/course/lecture_material_list_zip_download2.acl?CONTENT_SEQ=6339205&ky=A20213ELEC462003&ud=2021113585&pf_st_flag=2" \
-H "Connection: keep-alive" \
-H "Accept: */*" \
-H "Accept-Encoding: gzip" \
-H "Accept-Language: ko-KR,ko;q=0.9" \
-H "Host: lms.knu.ac.kr" \
-H "Origin: https://lms.knu.ac.kr" \
-H "Referer: https://lms.knu.ac.kr/ilos/st/course/lecture_material_view_form.acl?ARTL_NUM=6339205&SCH_KEY=&SCH_VALUE=&display=1&start=1" \
-H '"sec-ch-ua: " Not A;Brand";v="99", "Chromium";v="96", "Google Chrome";v="96"' \
-H "sec-ch-ua-mobile: ?0" \
-H "sec-ch-ua-platfrom: \"Windows\"" \
-H "Sec-Fetch-Dest: iframe" \
-H "Sec-Fetch-Mode: navigate" \
-H "Sec-Fetch-Site: same-origin" \
-H "Sec-Fetch-User: ?1" \
-H "Upgrade-Insecure-Requests: 1" \
-H "User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/96.0.4664.93 Safari/537.36" \
-o 강의자료파일다운로드.zip
unzip -o 강의자료파일다운로드.zip -d "강의자료"