#!/bin/bash

SERVER="http://127.0.0.1:8080"
PASS_COUNT=0
FAIL_COUNT=0

print_header() {
	echo -e "\n\033[1;34m=== $1 (expected $2) ===\033[0m"
}

run_test() {
	URL=$1
	METHOD=$2
	DATA=$3
	EXPECTED=$4

	print_header "$METHOD $URL" "$EXPECTED"

	if [ "$METHOD" = "GET" ]; then
		HTTP_CODE=$(curl -s -o /dev/null -w "%{http_code}" "$SERVER$URL")
	elif [ "$METHOD" = "POST" ]; then
		HTTP_CODE=$(curl -s -o /dev/null -w "%{http_code}" -X POST -d "$DATA" "$SERVER$URL")
	elif [ "$METHOD" = "DELETE" ]; then
		HTTP_CODE=$(curl -s -o /dev/null -w "%{http_code}" -X DELETE "$SERVER$URL")
	fi

	if [ "$HTTP_CODE" = "$EXPECTED" ]; then
		echo -e "[$HTTP_CODE] \033[1;32m✔ Test passed\033[0m"
		((PASS_COUNT++))
	else
		echo -e "[$HTTP_CODE] \033[1;31m✘ Test failed\033[0m (expected $EXPECTED)"
		((FAIL_COUNT++))
	fi
}

# 🧹 Test dosyaları hazırlığı
touch www/images/deletable.txt

### ✅ TESTLER

# GET
run_test "/images/logo.png" "GET" "" "200"
run_test "/images/" "GET" "" "200"
run_test "/unknown" "GET" "" "404"
run_test "/old-page" "GET" "" "301"
run_test "/cgi-bin/script.py" "GET" "" "200"

# POST
run_test "/upload" "POST" "this is a test upload" "201"
run_test "/images" "POST" "deneme" "403"

# DELETE
run_test "/images/deletable.txt" "DELETE" "" "200"
run_test "/images/nope.txt" "DELETE" "" "404"
run_test "/cgi-bin/script.py" "DELETE" "" "405"

# 🧼 Temizlik
rm -f www/images/deletable.txt
rm -f www/uploads/upload_*

### 🔚 Test özeti
echo -e "\n\033[1;36mTest Summary:\033[0m"
echo -e "  ✅ Passed: $PASS_COUNT"
echo -e "  ❌ Failed: $FAIL_COUNT"

if [ $FAIL_COUNT -eq 0 ]; then
	echo -e "\n\033[1;32mAll tests passed! ✅\033[0m"
else
	echo -e "\n\033[1;31mSome tests failed. ❌\033[0m"
fi
