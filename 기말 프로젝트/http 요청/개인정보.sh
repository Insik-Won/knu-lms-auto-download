#!/usr/bin/env bash
curl -v -L --referer ";auto" -X GET --cookie cookie.txt --cookie-jar cookie.txt "https://lms.knu.ac.kr/ilos/mp/myinfo_form.acl" > 개인정보.txt \
-H "Connection: keep-alive" \
-H "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/avif,image/webp,image/apng,*/*;q=0.8,application/signed-exchange;v=b3;q=0.9" \
-H "Accept-Encoding: gzip" \
-H "Accept-Language: ko-KR,ko;q=0.9" \
-H "Host: lms.knu.ac.kr" \
-H "Origin: https://lms.knu.ac.kr" \
-H "Referer: https://lms.knu.ac.kr/ilos/mp/myinfo_form.acl" \
-H '"sec-ch-ua: " Not A;Brand";v="99", "Chromium";v="96", "Google Chrome";v="96"' \
-H "sec-ch-ua-mobile: ?0" \
-H "sec-ch-ua-platfrom: \"Windows\"" \
-H "Sec-Fetch-Dest: document" \
-H "Sec-Fetch-Mode: navigate" \
-H "Sec-Fetch-Site: same-origin" \
-H "Sec-Fetch-User: ?1" \
-H "Upgrade-Insecure-Requests: 1" \
-H "User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/96.0.4664.93 Safari/537.36" \
--output 개인정보.txt.gz
gzip -df 개인정보.txt.gz

# 페이지 이동