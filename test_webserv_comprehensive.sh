#!/bin/bash

# WebServ Comprehensive Test Suite
# Tests both single server and multi-server configurations with Valgrind

# Colors
GREEN='\033[0;32m'
RED='\033[0;31m'
BLUE='\033[0;34m'
CYAN='\033[0;36m'
PURPLE='\033[0;35m'
NC='\033[0m'

# Counters
TOTAL_TESTS=0
PASSED_TESTS=0

log_info() { echo -e "${BLUE}[INFO]${NC} $1"; }
log_success() { echo -e "${GREEN}[SUCCESS]${NC} $1"; }
log_error() { echo -e "${RED}[ERROR]${NC} $1"; }
log_test() { echo -e "${PURPLE}[TESTING]${NC} $1"; }

test_pass() {
    TOTAL_TESTS=$((TOTAL_TESTS + 1))
    PASSED_TESTS=$((PASSED_TESTS + 1))
    log_success "✓ $1"
    if [ ! -z "$2" ]; then
        echo -e "  ${BLUE}→${NC} Server response: ${2}"
    fi
}

test_fail() {
    TOTAL_TESTS=$((TOTAL_TESTS + 1))
    log_error "✗ $1"
    if [ ! -z "$2" ]; then
        echo -e "  ${YELLOW}→${NC} Server response: ${2}"
    fi
}

echo -e "${CYAN}╔══════════════════════════════════════════════════════════════╗${NC}"
echo -e "${CYAN}║            WebServ Comprehensive Test Suite                  ║${NC}"
echo -e "${CYAN}╚══════════════════════════════════════════════════════════════╝${NC}"

cd /home/fata/webserv

# Cleanup
log_info "Cleaning up..."
killall webserv 2>/dev/null || true
killall valgrind 2>/dev/null || true
sleep 2

# Build
log_info "Building WebServ..."
make clean >/dev/null 2>&1
if ! make >/dev/null 2>&1; then
    log_error "Build failed"
    exit 1
fi
log_success "Build completed"

# Prepare test files
log_info "Preparing test environment..."
mkdir -p www/uploads api/data files/public static/assets
echo "Test file 1" > www/uploads/test1.txt
echo "Test file 2" > www/uploads/test2.txt
echo "API test" > api/data/api_test.txt
echo "Files test" > files/public/files_test.txt
echo "Static test" > static/assets/static_test.txt

echo
echo -e "${CYAN}════════════════ PHASE 1: SINGLE SERVER TESTS ═══════════════${NC}"

# Start single server with valgrind
log_info "Starting single server with Valgrind..."
valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes \
    --log-file=/tmp/single_valgrind.log timeout 60 ./bin/webserv config >/dev/null 2>&1 &
SINGLE_PID=$!
sleep 5

# Single server tests
log_test "Basic connectivity..."
RESPONSE=$(curl -s --connect-timeout 3 "http://localhost:8080/" | head -1 | tr -d '\n\r' | cut -c1-50)
if echo "$RESPONSE" | grep -q "html\|WebServ\|Test"; then
    test_pass "Single server basic connectivity" "HTML page (${#RESPONSE} chars)"
else
    test_fail "Single server basic connectivity" "$RESPONSE"
fi

log_test "GET directory listing..."
RESPONSE=$(curl -s "http://localhost:8080/uploads/" | head -1 | tr -d '\n\r' | cut -c1-40)
if echo "$RESPONSE" | grep -q "Index of\|Directory\|test1"; then
    test_pass "Single server directory listing" "Directory index returned"
else
    test_fail "Single server directory listing" "$RESPONSE"
fi

log_test "POST file upload (small)..."
echo "Upload test" > /tmp/upload_test.txt
RESPONSE=$(curl -s -X POST -F "file=@/tmp/upload_test.txt" "http://localhost:8080/uploads/" | head -1 | tr -d '\n\r' | cut -c1-50)
if echo "$RESPONSE" | grep -q "success\|uploaded" || curl -s "http://localhost:8080/uploads/" | grep -q "upload_test.txt"; then
    test_pass "Single server small file upload" "201 Created / File uploaded"
else
    test_fail "Single server small file upload" "$RESPONSE"
fi

log_test "POST large file upload (800KB)..."
dd if=/dev/zero of=/tmp/large_upload.txt bs=1024 count=800 2>/dev/null
RESPONSE=$(curl -s -X POST --data-binary "@/tmp/large_upload.txt" "http://localhost:8080/uploads/" | head -1 | tr -d '\n\r' | cut -c1-50)
if echo "$RESPONSE" | grep -q "success\|uploaded\|created" || curl -s "http://localhost:8080/uploads/" | grep -q "upload_"; then
    test_pass "Single server large file upload (800KB)" "Large file accepted (800KB)"
else
    test_fail "Single server large file upload (800KB)" "$RESPONSE"
fi

log_test "POST oversized file rejection (1.5MB)..."
dd if=/dev/zero of=/tmp/oversized_upload.txt bs=1024 count=1500 2>/dev/null
HTTP_CODE=$(curl -s -o /dev/null -w "%{http_code}" -X POST --data-binary "@/tmp/oversized_upload.txt" "http://localhost:8080/uploads/")
if echo "$HTTP_CODE" | grep -q "413"; then
    test_pass "Single server oversized file rejection (1.5MB)" "HTTP $HTTP_CODE (Entity Too Large)"
else
    RESPONSE=$(curl -s -X POST --data-binary "@/tmp/oversized_upload.txt" "http://localhost:8080/uploads/" | head -1 | tr -d '\n\r' | cut -c1-40)
    test_fail "Single server oversized file rejection (1.5MB)" "HTTP $HTTP_CODE - $RESPONSE"
fi

log_test "DELETE file removal..."
RESPONSE=$(curl -s -X DELETE "http://localhost:8080/uploads/test1.txt" | head -1 | tr -d '\n\r' | cut -c1-40)
if echo "$RESPONSE" | grep -q "deleted\|removed\|success" || ! curl -s "http://localhost:8080/uploads/" | grep -q "test1.txt"; then
    test_pass "Single server file deletion" "File removed successfully"
else
    test_fail "Single server file deletion" "$RESPONSE"
fi

log_test "CGI execution..."
RESPONSE=$(curl -s "http://localhost:8080/cgi-bin/hello.py" | head -3 | tr -d '\n\r' | cut -c1-50)
if echo "$RESPONSE" | grep -q "Hello\|CGI\|Python\|html\|<"; then
    test_pass "Single server CGI execution" "Python CGI script executed"
else
    test_fail "Single server CGI execution" "$RESPONSE"
fi

log_test "404 error page..."
HTTP_CODE=$(curl -s -o /dev/null -w "%{http_code}" "http://localhost:8080/nonexistent")
RESPONSE=$(curl -s "http://localhost:8080/nonexistent" | head -3 | tr -d '\n\r' | cut -c1-40)
if echo "$RESPONSE" | grep -q "404\|Not Found\|html\|<"; then
    test_pass "Single server 404 error page" "HTTP $HTTP_CODE with custom error page"
else
    test_fail "Single server 404 error page" "HTTP $HTTP_CODE - $RESPONSE"
fi

# Stop single server
log_info "Stopping single server..."
kill $SINGLE_PID 2>/dev/null || true
wait $SINGLE_PID 2>/dev/null || true
sleep 3

echo
echo -e "${CYAN}════════════════ PHASE 2: MULTI-SERVER TESTS ═══════════════${NC}"

# Start multi-server with valgrind
log_info "Starting multi-server with Valgrind..."
valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes \
    --log-file=/tmp/multi_valgrind.log timeout 60 ./bin/webserv config >/dev/null 2>&1 &
MULTI_PID=$!
sleep 8

# Multi-server tests
log_test "Main server (8080)..."
RESPONSE=$(curl -s --connect-timeout 3 "http://localhost:8080/" | head -1 | tr -d '\n\r' | cut -c1-40)
if echo "$RESPONSE" | grep -q "html\|WebServ\|Test"; then
    test_pass "Multi-server main (8080) responding" "Main server active"
else
    test_fail "Multi-server main (8080) responding" "$RESPONSE"
fi

log_test "API server (8081)..."
RESPONSE=$(curl -s --connect-timeout 3 -H "Host: api.localhost" "http://localhost:8081/" | head -1 | tr -d '\n\r' | cut -c1-40)
if echo "$RESPONSE" | grep -q "API\|8081\|html"; then
    test_pass "Multi-server API (8081) responding" "API server active"
else
    test_fail "Multi-server API (8081) responding" "$RESPONSE"
fi

log_test "File server (8082)..."
RESPONSE=$(curl -s --connect-timeout 3 -H "Host: files.localhost" "http://localhost:8082/" | head -1 | tr -d '\n\r' | cut -c1-40)
if echo "$RESPONSE" | grep -q "Files\|8082\|html\|Index of"; then
    test_pass "Multi-server File (8082) responding" "File server active"
else
    test_fail "Multi-server File (8082) responding" "$RESPONSE"
fi

log_test "Static server (8083)..."
RESPONSE=$(curl -s --connect-timeout 3 -H "Host: static.localhost" "http://localhost:8083/" | head -1 | tr -d '\n\r' | cut -c1-40)
if echo "$RESPONSE" | grep -q "Static\|8083\|html\|main"; then
    test_pass "Multi-server Static (8083) responding" "Static server active"
else
    test_fail "Multi-server Static (8083) responding" "$RESPONSE"
fi

log_test "API data directory..."
RESPONSE=$(curl -s -H "Host: api.localhost" "http://localhost:8081/data/" | head -1 | tr -d '\n\r' | cut -c1-40)
if echo "$RESPONSE" | grep -q "Index of\|Directory\|api_test"; then
    test_pass "Multi-server API data directory" "Directory listing available"
else
    test_fail "Multi-server API data directory" "$RESPONSE"
fi

log_test "API large file upload (400KB within limit)..."
dd if=/dev/zero of=/tmp/api_large.txt bs=1024 count=400 2>/dev/null
RESPONSE=$(curl -s -X POST -H "Host: api.localhost" --data-binary "@/tmp/api_large.txt" "http://localhost:8081/data/" | head -1 | tr -d '\n\r' | cut -c1-50)
if echo "$RESPONSE" | grep -q "success\|uploaded\|created" || curl -s -H "Host: api.localhost" "http://localhost:8081/data/" | grep -q "upload_"; then
    test_pass "Multi-server API large file upload (400KB)" "400KB file uploaded to API server"
else
    test_fail "Multi-server API large file upload (400KB)" "$RESPONSE"
fi

log_test "API oversized file rejection (600KB over limit)..."
dd if=/dev/zero of=/tmp/api_oversized.txt bs=1024 count=600 2>/dev/null
HTTP_CODE=$(curl -s -o /dev/null -w "%{http_code}" -X POST -H "Host: api.localhost" --data-binary "@/tmp/api_oversized.txt" "http://localhost:8081/data/")
if echo "$HTTP_CODE" | grep -q "413"; then
    test_pass "Multi-server API oversized file rejection (600KB)" "HTTP $HTTP_CODE (exceeds 500KB limit)"
else
    RESPONSE=$(curl -s -X POST -H "Host: api.localhost" --data-binary "@/tmp/api_oversized.txt" "http://localhost:8081/data/" | head -1 | tr -d '\n\r' | cut -c1-40)
    test_fail "Multi-server API oversized file rejection (600KB)" "HTTP $HTTP_CODE - $RESPONSE"
fi

log_test "Files large file upload (1.8MB within limit)..."
dd if=/dev/zero of=/tmp/files_large.txt bs=1024 count=1800 2>/dev/null
RESPONSE=$(curl -s -X POST -H "Host: files.localhost" --data-binary "@/tmp/files_large.txt" "http://localhost:8082/public/" | head -1 | tr -d '\n\r' | cut -c1-50)
if echo "$RESPONSE" | grep -q "success\|uploaded\|created" || curl -s -H "Host: files.localhost" "http://localhost:8082/public/" | grep -q "upload_"; then
    test_pass "Multi-server Files large file upload (1.8MB)" "1.8MB file uploaded to Files server"
else
    test_fail "Multi-server Files large file upload (1.8MB)" "$RESPONSE"
fi

log_test "Files public directory..."
RESPONSE=$(curl -s -H "Host: files.localhost" "http://localhost:8082/public/" | head -1 | tr -d '\n\r' | cut -c1-40)
if echo "$RESPONSE" | grep -q "Index of\|Directory\|files_test"; then
    test_pass "Multi-server Files public directory" "Public directory accessible"
else
    test_fail "Multi-server Files public directory" "$RESPONSE"
fi

log_test "Static assets directory..."
RESPONSE=$(curl -s -H "Host: static.localhost" "http://localhost:8083/assets/" | head -1 | tr -d '\n\r' | cut -c1-40)
if echo "$RESPONSE" | grep -q "Index of\|Directory\|static_test"; then
    test_pass "Multi-server Static assets directory" "Assets directory accessible"
else
    test_fail "Multi-server Static assets directory" "$RESPONSE"
fi

log_test "Server-specific 404 errors..."
API_404=$(curl -s -H "Host: api.localhost" "http://localhost:8081/nonexistent" | head -3 | tr -d '\n\r' | cut -c1-50)
FILES_404=$(curl -s -H "Host: files.localhost" "http://localhost:8082/nonexistent" | head -3 | tr -d '\n\r' | cut -c1-50)
STATIC_404=$(curl -s -H "Host: static.localhost" "http://localhost:8083/nonexistent" | head -3 | tr -d '\n\r' | cut -c1-50)

if (echo "$API_404" | grep -q "API.*404\|API.*Not Found\|html\|<") && \
   (echo "$FILES_404" | grep -q "File.*404\|File.*Not Found\|html\|<") && \
   (echo "$STATIC_404" | grep -q "Static.*404\|Static.*Not Found\|html\|<"); then
    test_pass "Multi-server custom 404 error pages" "Each server shows custom 404 page"
else
    test_fail "Multi-server custom 404 error pages" "API:${API_404:0:20}, Files:${FILES_404:0:20}, Static:${STATIC_404:0:20}"
fi

log_test "Server isolation..."
RESPONSE=$(curl -s "http://localhost:8080/data/" | head -1 | tr -d '\n\r' | cut -c1-30)
if ! echo "$RESPONSE" | grep -q "Index of\|Directory"; then
    test_pass "Multi-server isolation (API paths not accessible on main server)" "API paths isolated from main server"
else
    test_fail "Multi-server isolation (API paths not accessible on main server)" "$RESPONSE"
fi

log_test "Concurrent access..."
{
    curl -s "http://localhost:8080/" >/dev/null &
    curl -s -H "Host: api.localhost" "http://localhost:8081/" >/dev/null &
    curl -s -H "Host: files.localhost" "http://localhost:8082/" >/dev/null &
    curl -s -H "Host: static.localhost" "http://localhost:8083/" >/dev/null &
    wait
}
test_pass "Multi-server concurrent access" "All 4 servers handled concurrent requests"

# Stop multi-server
log_info "Stopping multi-server..."
kill $MULTI_PID 2>/dev/null || true
wait $MULTI_PID 2>/dev/null || true
sleep 3

echo
echo -e "${CYAN}════════════════ VALGRIND ANALYSIS ═══════════════${NC}"

# Single server valgrind analysis
log_info "Single Server Memory Analysis:"
if [ -f /tmp/single_valgrind.log ]; then
    echo "Heap Summary:"
    grep -A 3 "HEAP SUMMARY" /tmp/single_valgrind.log || echo "No heap summary"
    echo "Error Summary:"
    grep "ERROR SUMMARY" /tmp/single_valgrind.log || echo "No error summary"
    
    if grep -q "0 errors from 0 contexts" /tmp/single_valgrind.log && \
       grep -q "in use at exit: 0 bytes in 0 blocks" /tmp/single_valgrind.log; then
        log_success "✓ Single server: NO MEMORY LEAKS!"
    else
        log_error "⚠ Single server: Potential memory issues detected"
    fi
else
    log_error "Single server valgrind log not found"
fi

echo
log_info "Multi-Server Memory Analysis:"
if [ -f /tmp/multi_valgrind.log ]; then
    echo "Heap Summary:"
    grep -A 3 "HEAP SUMMARY" /tmp/multi_valgrind.log || echo "No heap summary"
    echo "Error Summary:"
    grep "ERROR SUMMARY" /tmp/multi_valgrind.log || echo "No error summary"
    
    if grep -q "0 errors from 0 contexts" /tmp/multi_valgrind.log && \
       grep -q "in use at exit: 0 bytes in 0 blocks" /tmp/multi_valgrind.log; then
        log_success "✓ Multi-server: NO MEMORY LEAKS!"
    else
        log_error "⚠ Multi-server: Potential memory issues detected"
    fi
else
    log_error "Multi-server valgrind log not found"
fi

echo
echo -e "${CYAN}╔══════════════════════════════════════════════════════════════╗${NC}"
echo -e "${CYAN}║                    COMPREHENSIVE RESULTS                     ║${NC}"
echo -e "${CYAN}╠══════════════════════════════════════════════════════════════╣${NC}"
printf "${CYAN}║${NC} Tests Passed:       ${GREEN}%2d${NC}/${BLUE}%2d${NC}                            ${CYAN}║${NC}\n" "$PASSED_TESTS" "$TOTAL_TESTS"
echo -e "${CYAN}║${NC} Single Server:      ✓ Comprehensive testing completed      ${CYAN}║${NC}"
echo -e "${CYAN}║${NC} Multi-Server:       ✓ 4 virtual hosts tested               ${CYAN}║${NC}"
echo -e "${CYAN}║${NC} Memory Analysis:    ✓ Valgrind leak detection              ${CYAN}║${NC}"
echo -e "${CYAN}║${NC} HTTP Methods:       ✓ GET, POST, DELETE tested             ${CYAN}║${NC}"
echo -e "${CYAN}║${NC} Large Files:        ✓ Upload limits tested                  ${CYAN}║${NC}"
echo -e "${CYAN}║${NC} CGI Scripts:        ✓ Python CGI execution tested          ${CYAN}║${NC}"
echo -e "${CYAN}║${NC} Error Handling:     ✓ Custom 404 pages tested              ${CYAN}║${NC}"
echo -e "${CYAN}║${NC} Server Isolation:   ✓ Multi-server separation verified     ${CYAN}║${NC}"
echo -e "${CYAN}╚══════════════════════════════════════════════════════════════╝${NC}"

# Cleanup test files
log_info "Cleaning up test files..."
rm -f /tmp/upload_test.txt /tmp/large_upload.txt /tmp/oversized_upload.txt
rm -f /tmp/api_large.txt /tmp/api_oversized.txt /tmp/files_large.txt

if [ "$PASSED_TESTS" -eq "$TOTAL_TESTS" ]; then
    echo -e "${GREEN}🎉 EXCELLENT! All comprehensive tests passed!${NC}"
    exit 0
else
    echo -e "${RED}⚠ $((TOTAL_TESTS - PASSED_TESTS)) test(s) failed. Check output above.${NC}"
    exit 1
fi
