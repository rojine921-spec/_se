@echo off
setlocal
echo ============================================
echo   mycurl Build Script for Windows
echo ============================================
echo.

REM Locate a MinGW-w64 gcc (MSYS2 / standalone / PATH)
set "GCC_CMD=gcc"
where gcc >nul 2>nul
if %errorlevel% neq 0 (
    if exist "C:\msys64\mingw64\bin\gcc.exe" set "GCC_CMD=C:\msys64\mingw64\bin\gcc.exe"
    if exist "C:\msys64\ucrt64\bin\gcc.exe"  set "GCC_CMD=C:\msys64\ucrt64\bin\gcc.exe"
)
%GCC_CMD% --version >nul 2>nul
if %errorlevel% neq 0 (
    echo Compiler not found.
    echo Install MSYS2 and the mingw-w64-x86_64-gcc package, or add gcc to PATH.
    exit /b 1
)
echo Using compiler: %GCC_CMD%
echo.

if not exist "obj" mkdir obj
if not exist "bin" mkdir bin

echo [1/4] Compiling url_parser.c ...
%GCC_CMD% -Wall -Wextra -Iinclude -std=c99 -c src/url_parser.c -o obj/url_parser.o
if %errorlevel% neq 0 goto :error

echo [2/4] Compiling utils.c ...
%GCC_CMD% -Wall -Wextra -Iinclude -std=c99 -c src/utils.c -o obj/utils.o
if %errorlevel% neq 0 goto :error

echo [3/4] Compiling http_client.c ...
%GCC_CMD% -Wall -Wextra -Iinclude -std=c99 -c src/http_client.c -o obj/http_client.o
if %errorlevel% neq 0 goto :error

echo [4/4] Compiling main.c ...
%GCC_CMD% -Wall -Wextra -Iinclude -std=c99 -c src/main.c -o obj/main.o
if %errorlevel% neq 0 goto :error

echo.
echo Linking mycurl.exe (static) ...
%GCC_CMD% obj/url_parser.o obj/utils.o obj/http_client.o obj/main.o -o bin/mycurl.exe -lws2_32 -static
if %errorlevel% neq 0 goto :error

echo.
echo ============================================
echo   Build successful!
echo   Output: bin\mycurl.exe
echo ============================================
echo.
echo Usage examples:
echo   bin\mycurl.exe http://example.com
echo   bin\mycurl.exe -v http://httpbin.org/get
echo   bin\mycurl.exe -X POST -d "name=test" http://httpbin.org/post
echo   bin\mycurl.exe -H "X-Custom: value" http://httpbin.org/get
echo   bin\mycurl.exe -o output.txt http://example.com
echo   bin\mycurl.exe -L http://example.com/redirect
echo.
goto :end

:error
echo.
echo ============================================
echo   BUILD FAILED!
echo ============================================
exit /b 1

:end