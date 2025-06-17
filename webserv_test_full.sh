#!/bin/bash

# -------------------------------
# Webserv Full Automated Test Suite
# -------------------------------

#set -e

CONFIG_FILE=config_test.conf
WEBSERV_BINARY=./bin/webserv

# Step 1: Create Config File
cat > $CONFIG_FILE << EOF
server {
    listen 9090;
    server_name localhost www.example.com;
    error_page 404 /errors/404.html;
    error_page 500 /errors/500.html;
    index index.html;
    client_max_body_size 1000000;

    location / {
        root /www;
        index index.html;
        autoindex off;
        methods GET POST;
    }

    location /upload {
        index index.html;
        root /www/uploads;
        methods POST;
        upload_store ./www/uploads;
    }

    location /old-page {
        return 301 http://www.example.com/new-page;
    }

    location /cgi-bin {
        root /www/cgi-bin;
        cgi_extension .php;
        methods GET POST;
        cgi_path /usr/bin/php-cgi;
    }

    location /files {
        root /www/files;
        autoindex on;
        index index.html;
        methods GET;
    }
}

server {
    listen 8080;
    server_name api.example.com;

    location / {
        root /www/api;
        index index.py;
        methods GET POST;
        cgi_extension .py;
        cgi_path /usr/bin/python3;
    }
}

server {
    server_name api.example.com;
    listen 7090;

    location / {
        root /www/alternate;
        index index.html;
        autoindex off;
        methods GET;
    }
}
EOF

# Step 2: Prepare Web Content
mkdir -p www/cgi-bin www/uploads www/files www/errors www/private www/alternate www/api www/public

echo '<html><body>Index Page</body></html>' > www/index.html
echo '#!/usr/bin/env python3
print("Content-Type: text/html\n\nHello from CGI")' > www/cgi-bin/script.py
chmod +x www/cgi-bin/script.py

echo 'Test File 1' > www/files/deneme.txt
echo 'Test File 2' > www/files/deneme2.txt

echo '<html><body><h1>404 Not Found</h1></body></html>' > www/errors/404.html
echo '<html><body><h1>500 Internal Server Error</h1></body></html>' > www/errors/500.html
echo '<html><body><h1>403 Forbidden</h1></body></html>' > www/errors/403.html
echo '<html><body><h1>400 Bad Request</h1></body></html>' > www/errors/400.html

echo '<html><body>Alternate Server</body></html>' > www/alternate/index.html
echo '<html><body>API Server</body></html>' > www/api/index.py

# Step 3: Launch Webserv
$WEBSERV_BINARY $CONFIG_FILE &
WS_PID=$!
sleep 1

# Step 4: Run Tests
total_tests=0
passed_tests=0

run_test() {
    desc=$1
    expected=$2
    url=$3
    method=${4:-GET}
    data=${5:-}

    ((total_tests++))
    if [ "$method" = "POST" ]; then
        code=$(curl -s -o /dev/null -w "%{http_code}" -X POST -d "$data" "$url")
    else
        code=$(curl -s -o /dev/null -w "%{http_code}" "$url")
    fi

    if [ "$code" = "$expected" ]; then
        echo "✅ PASSED: $desc (Expected $expected)"
        ((passed_tests++))
    else
        echo "❌ FAILED: $desc (Expected $expected, Got $code)"
    fi
}

check_body_contains() {
    desc=$1
    url=$2
    expected_string=$3

    ((total_tests++))
    body=$(curl -s "$url")
    if [[ "$body" == *"$expected_string"* ]]; then
        echo "✅ PASSED: $desc (Content matched)"
        ((passed_tests++))
    else
        echo "❌ FAILED: $desc (Expected content not found)"
    fi
}

echo "### BEGINNING TEST SUITE ###"

run_test "GET /" 200 http://localhost:9090/
run_test "GET /files/ (autoindex)" 200 http://localhost:9090/files/
run_test "GET /private/ (404 Not Found)" 404 http://localhost:9090/private/
run_test "GET /non-existent page" 404 http://localhost:9090/this-page-does-not-exist
run_test "POST /upload (success)" 201 http://localhost:9090/upload POST "data here"
run_test "POST /upload2 (404 Not Found)" 404 http://localhost:9090/upload2 POST "trigger"
run_test "POST / (Method Not Allowed)" 403 http://localhost:9090/ POST
run_test "GET /upload (Method Not Allowed)" 405 http://localhost:9090/upload GET
run_test "GET /old-page (Redirect)" 301 http://localhost:9090/old-page

check_body_contains "CGI script execution" http://localhost:9090/cgi-bin/script.py "Hello from CGI"
check_body_contains "Custom 404 error page content" http://localhost:9090/non-existent-page "404 Not Found"

echo ""
echo "### TEST SUMMARY ###"
echo "Total tests run: $total_tests"
echo "Tests passed:    $passed_tests"
echo "Tests failed:    $((total_tests - passed_tests))"

# Step 5: Cleanup
echo ""
echo "Cleaning up test environment..."
kill $WS_PID
rm -rf www $CONFIG_FILE
echo "Done."
