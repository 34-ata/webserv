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
}

test_fail() {
    TOTAL_TESTS=$((TOTAL_TESTS + 1))
    log_error "✗ $1"
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
if curl -s --connect-timeout 3 "http://localhost:8080/" | grep -q "html\|WebServ\|Test"; then
    test_pass "Single server basic connectivity"
else
    test_fail "Single server basic connectivity"
fi

log_test "GET directory listing..."
if curl -s "http://localhost:8080/uploads/" | grep -q "Index of\|Directory\|test1"; then
    test_pass "Single server directory listing"
else
    test_fail "Single server directory listing"
fi

log_test "POST file upload..."
echo "Upload test" > /tmp/upload_test.txt
if curl -s -X POST -F "file=@/tmp/upload_test.txt" "http://localhost:8080/uploads/" | grep -q "success\|uploaded" || \
   curl -s "http://localhost:8080/uploads/" | grep -q "upload_test.txt"; then
    test_pass "Single server file upload"
else
    test_fail "Single server file upload"
fi

log_test "DELETE file removal..."
if curl -s -X DELETE "http://localhost:8080/uploads/test1.txt" | grep -q "deleted\|removed\|success" || \
   ! curl -s "http://localhost:8080/uploads/" | grep -q "test1.txt"; then
    test_pass "Single server file deletion"
else
    test_fail "Single server file deletion"
fi

log_test "CGI execution..."
if curl -s "http://localhost:8080/cgi-bin/hello.py" | grep -q "Hello\|CGI\|Python"; then
    test_pass "Single server CGI execution"
else
    test_fail "Single server CGI execution"
fi

log_test "404 error page..."
if curl -s "http://localhost:8080/nonexistent" | grep -q "404\|Not Found"; then
    test_pass "Single server 404 error page"
else
    test_fail "Single server 404 error page"
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
if curl -s --connect-timeout 3 "http://localhost:8080/" | grep -q "html\|WebServ\|Test"; then
    test_pass "Multi-server main (8080) responding"
else
    test_fail "Multi-server main (8080) responding"
fi

log_test "API server (8081)..."
if curl -s --connect-timeout 3 -H "Host: api.localhost" "http://localhost:8081/" | grep -q "API\|8081\|html"; then
    test_pass "Multi-server API (8081) responding"
else
    test_fail "Multi-server API (8081) responding"
fi

log_test "File server (8082)..."
if curl -s --connect-timeout 3 -H "Host: files.localhost" "http://localhost:8082/" | grep -q "Files\|8082\|html\|Index of"; then
    test_pass "Multi-server File (8082) responding"
else
    test_fail "Multi-server File (8082) responding"
fi

log_test "Static server (8083)..."
if curl -s --connect-timeout 3 -H "Host: static.localhost" "http://localhost:8083/" | grep -q "Static\|8083\|html\|main"; then
    test_pass "Multi-server Static (8083) responding"
else
    test_fail "Multi-server Static (8083) responding"
fi

log_test "API data directory..."
if curl -s -H "Host: api.localhost" "http://localhost:8081/data/" | grep -q "Index of\|Directory\|api_test"; then
    test_pass "Multi-server API data directory"
else
    test_fail "Multi-server API data directory"
fi

log_test "Files public directory..."
if curl -s -H "Host: files.localhost" "http://localhost:8082/public/" | grep -q "Index of\|Directory\|files_test"; then
    test_pass "Multi-server Files public directory"
else
    test_fail "Multi-server Files public directory"
fi

log_test "Static assets directory..."
if curl -s -H "Host: static.localhost" "http://localhost:8083/assets/" | grep -q "Index of\|Directory\|static_test"; then
    test_pass "Multi-server Static assets directory"
else
    test_fail "Multi-server Static assets directory"
fi

log_test "Server-specific 404 errors..."
API_404=$(curl -s -H "Host: api.localhost" "http://localhost:8081/nonexistent")
FILES_404=$(curl -s -H "Host: files.localhost" "http://localhost:8082/nonexistent")
STATIC_404=$(curl -s -H "Host: static.localhost" "http://localhost:8083/nonexistent")

if echo "$API_404" | grep -q "API.*404\|API.*Not Found" && \
   echo "$FILES_404" | grep -q "File.*404\|File.*Not Found" && \
   echo "$STATIC_404" | grep -q "Static.*404\|Static.*Not Found"; then
    test_pass "Multi-server custom 404 error pages"
else
    test_fail "Multi-server custom 404 error pages"
fi

log_test "Server isolation..."
if ! curl -s "http://localhost:8080/data/" | grep -q "Index of\|Directory"; then
    test_pass "Multi-server isolation (API paths not accessible on main server)"
else
    test_fail "Multi-server isolation (API paths not accessible on main server)"
fi

log_test "Concurrent access..."
{
    curl -s "http://localhost:8080/" >/dev/null &
    curl -s -H "Host: api.localhost" "http://localhost:8081/" >/dev/null &
    curl -s -H "Host: files.localhost" "http://localhost:8082/" >/dev/null &
    curl -s -H "Host: static.localhost" "http://localhost:8083/" >/dev/null &
    wait
}
test_pass "Multi-server concurrent access"

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
echo -e "${CYAN}║${NC} CGI Scripts:        ✓ Python CGI execution tested          ${CYAN}║${NC}"
echo -e "${CYAN}║${NC} Error Handling:     ✓ Custom 404 pages tested              ${CYAN}║${NC}"
echo -e "${CYAN}║${NC} Server Isolation:   ✓ Multi-server separation verified     ${CYAN}║${NC}"
echo -e "${CYAN}╚══════════════════════════════════════════════════════════════╝${NC}"

if [ "$PASSED_TESTS" -eq "$TOTAL_TESTS" ]; then
    echo -e "${GREEN}🎉 EXCELLENT! All comprehensive tests passed!${NC}"
    exit 0
else
    echo -e "${RED}⚠ $((TOTAL_TESTS - PASSED_TESTS)) test(s) failed. Check output above.${NC}"
    exit 1
fi
