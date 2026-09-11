@echo off
echo ============================================
echo   mycurl - Examples
echo ============================================
echo.

echo [1] Basic GET request
echo     bin\mycurl.exe http://httpbin.org/get
echo.

echo [2] Verbose GET request
echo     bin\mycurl.exe -v http://httpbin.org/get
echo.

echo [3] POST request with data
echo     bin\mycurl.exe -X POST -d "username=admin&password=123" http://httpbin.org/post
echo.

echo [4] Custom headers
echo     bin\mycurl.exe -H "Authorization: Bearer mytoken" -H "X-Custom: test" http://httpbin.org/get
echo.

echo [5] Save output to file
echo     bin\mycurl.exe -o response.txt http://httpbin.org/get
echo.

echo [6] HEAD request (headers only)
echo     bin\mycurl.exe -I http://httpbin.org/get
echo.

echo [7] Custom User-Agent
echo     bin\mycurl.exe -A "MyBot/1.0" http://httpbin.org/user-agent
echo.

echo [8] PUT request
echo     bin\mycurl.exe -X PUT -d "updated data" http://httpbin.org/put
echo.

echo [9] DELETE request
echo     bin\mycurl.exe -X DELETE http://httpbin.org/delete
echo.

echo [10] Silent mode (no progress)
echo     bin\mycurl.exe -s -o output.txt http://httpbin.org/get
echo.
