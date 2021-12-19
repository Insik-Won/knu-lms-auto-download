#!/usr/bin/env bash
curl -v -L --referer ";auto" -X POST --cookie cookie.txt --cookie-jar cookie.txt "https://lms.knu.ac.kr/ilos/co/list_file_list2.acl" > 강의자료파일목록.txt.gz \
-d "ud=2021113585&ky=A20213ELEC462003&pf_st_flag=2&CONTENT_SEQ=4F7ILQWDNWTIA&encoding=utf-8" \
-H "Connection: keep-alive" \
-H "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/avif,image/webp,image/apng,*/*;q=0.8,application/signed-exchange;v=b3;q=0.9" \
-H "Accept-Encoding: gzip" \
-H "Accept-Language: ko-KR,ko;q=0.9" \
-H "Host: lms.knu.ac.kr" \
-H "Origin: https://lms.knu.ac.kr" \
-H "Referer: https://lms.knu.ac.kr/ilos/st/course/lecture_material_list_form.acl" \
-H '"sec-ch-ua: " Not A;Brand";v="99", "Chromium";v="96", "Google Chrome";v="96"' \
-H "sec-ch-ua-mobile: ?0" \
-H "sec-ch-ua-platfrom: \"Windows\"" \
-H "Sec-Fetch-Dest: empty" \
-H "Sec-Fetch-Mode: cors" \
-H "Sec-Fetch-Site: same-origin" \
-H "User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/96.0.4664.93 Safari/537.36" \
-H "X-Requested-With: XMLHttpRequest" \
--output 강의자료파일목록.txt.gz
gzip -df 강의자료파일목록.txt.gz