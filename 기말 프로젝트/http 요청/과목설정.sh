#!/usr/bin/env bash
curl -L --referer ";auto" -v -X POST --cookie cookie.txt --cookie-jar cookie.txt https://lms.knu.ac.kr/ilos/st/course/eclass_room2.acl > 과목설정.txt \
-d "KJKEY=A20213ELEC462003&returnData=json&returnURI=%252Filos%252Fst%252Fcourse%252Fsubmain_form.acl&encoding=utf-8" \
-H "Connection: keep-alive" \
-H "Content-Type: application/x-www-form-urlencoded; charset=UTF-8" \
-H "Accept: */*" \
-H "Accept-Encoding: gzip" \
-H "Accept-Language: ko-KR,ko;q=0.9" \
-H "Host: lms.knu.ac.kr" \
-H "Origin: https://lms.knu.ac.kr" \
-H "Referer: https://lms.knu.ac.kr/ilos/main/member/login_form.acl" \
-H '"sec-ch-ua: " Not A;Brand";v="99", "Chromium";v="96", "Google Chrome";v="96"' \
-H "sec-ch-ua-mobile: ?0" \
-H "sec-ch-ua-platfrom: \"Windows\"" \
-H "Sec-Fetch-Dest: empty" \
-H "Sec-Fetch-Mode: cors" \
-H "Sec-Fetch-Site: same-origin" \
-H "User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/96.0.4664.93 Safari/537.36" \
-H "X-Requested-With: XMLHttpRequest"

# API 호출