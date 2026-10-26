#!/bin/sh
#
# CS631 APUE Group Project: sws Automated Test Suite
# Member 4: Quality Assurance & Functional Verification
#

PORT=8089
DOCROOT="./test_docroot"
CGIDIR="./test_cgidir"
LOGFILE="./test_sws.log"

echo "=== Setting up test environment ==="
rm -rf "$DOCROOT" "$CGIDIR" "$LOGFILE"
mkdir -p "$DOCROOT/subdir"
mkdir -p "$CGIDIR"

echo "Hello from sws index" > "$DOCROOT/index.html"
echo "Subdir file A" > "$DOCROOT/subdir/fileA.txt"
echo "Subdir file B" > "$DOCROOT/subdir/fileB.txt"

cat << 'EOF' > "$CGIDIR/hello.cgi"
#!/bin/sh
echo "Content-Type: text/plain"
echo ""
echo "Hello from CGI! Method: $REQUEST_METHOD Query: $QUERY_STRING"
EOF
chmod +x "$CGIDIR/hello.cgi"

echo "=== Starting sws in background ==="
./sws -p $PORT -c "$CGIDIR" -l "$LOGFILE" "$DOCROOT" &
SWS_PID=$!
sleep 1

cleanup() {
    echo "=== Cleaning up ==="
    kill $SWS_PID 2>/dev/null
    rm -rf "$DOCROOT" "$CGIDIR" "$LOGFILE"
}
trap cleanup EXIT INT TERM

echo "--- Test 1: GET / (index.html) ---"
curl -s -i "http://127.0.0.1:$PORT/" | grep "200 OK" && echo "PASS: GET /"

echo "--- Test 2: HEAD /index.html ---"
curl -s -I "http://127.0.0.1:$PORT/index.html" | grep "200 OK" && echo "PASS: HEAD"

echo "--- Test 3: 404 Not Found ---"
curl -s -i "http://127.0.0.1:$PORT/nonexistent.html" | grep "404 Not Found" && echo "PASS: 404"

echo "--- Test 4: Path Traversal Attack Prevention ---"
curl -s -i --path-as-is "http://127.0.0.1:$PORT/../../../etc/passwd" | grep "403 Forbidden" && echo "PASS: 403 Path Traversal"

echo "--- Test 5: Directory Indexing ---"
curl -s "http://127.0.0.1:$PORT/subdir/" | grep "fileA.txt" && echo "PASS: Directory Index"

echo "--- Test 6: CGI Execution ---"
curl -s "http://127.0.0.1:$PORT/cgi-bin/hello.cgi?param=test" | grep "Hello from CGI" && echo "PASS: CGI"

echo "--- Test 7: Verify Log File ---"
if [ -f "$LOGFILE" ]; then
    echo "PASS: Log file created successfully"
    cat "$LOGFILE"
else
    echo "FAIL: Log file missing"
fi

echo "=== All tests finished ==="
