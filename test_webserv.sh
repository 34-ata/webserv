#!/bin/bash

PASS_COUNT=0
FAIL_COUNT=0

print_header() {
	echo -e "\n\033[1;34m=== $2 $3 (expected $4) ===\033[0m"
}

run_test() {
	IP_PORT=$1
	URL=$2
	METHOD=$3
	DATA=$4
	EXPECTED=$5

	SERVER="http://$IP_PORT"

	print_header "$SERVER" "$METHOD" "$URL" "$EXPECTED"

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

# 🧹 Hazırlık
mkdir -p www/images
mkdir -p www/uploads
touch www/images/deletable.txt

# ✅ TESTLER

# 🌐 Port 9090 (localhost) - `.php` CGI kaldırıldı
run_test "127.0.0.1:9090" "/images/logo.png" "GET" "" "200"
run_test "127.0.0.1:9090" "/images/" "GET" "" "404"
run_test "127.0.0.1:9090" "/unknown" "GET" "" "404"
run_test "127.0.0.1:9090" "/old-page" "GET" "" "301"
run_test "127.0.0.1:9090" "/cgi-bin/script.py" "GET" "" "404"  # .py burada çalışmaz
run_test "127.0.0.1:9090" "/upload" "POST" "testdata" "201"
run_test "127.0.0.1:9090" "/images" "POST" "deneme" "403"
run_test "127.0.0.1:9090" "/images/deletable.txt" "DELETE" "" "200"
run_test "127.0.0.1:9090" "/images/nope.txt" "DELETE" "" "404"
run_test "127.0.0.1:9090" "/cgi-bin/script.py" "DELETE" "" "405"

# 🌐 Port 8080 (api.example.com) - `.py` CGI aktif
run_test "127.0.0.1:8080" "/" "GET" "" "404"
run_test "127.0.0.1:8080" "/index.py" "GET" "" "200"
run_test "127.0.0.1:8080" "/index.py" "POST" "sample data" "200"

# 🌐 Port 7090 (api.example.com - alternate)
run_test "127.0.0.1:7090" "/" "GET" "" "404"

# 🧼 Temizlik
rm -f www/images/deletable.txt
rm -f www/uploads/upload_*

# 🔚 Test Özeti
echo -e "\n\033[1;36mTest Summary:\033[0m"
echo -e "  ✅ Passed: $PASS_COUNT"
echo -e "  ❌ Failed: $FAIL_COUNT"

if [ $FAIL_COUNT -eq 0 ]; then
	echo -e "\n\033[1;32mAll tests passed! ✅\033[0m"
else
	echo -e "\n\033[1;31mSome tests failed. ❌\033[0m"
fi
