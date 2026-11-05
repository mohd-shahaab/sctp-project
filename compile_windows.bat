@echo off
REM ============================================================================
REM SCTP Lab: Windows Compilation Script
REM Compiles server_windows.c and client_windows.c using GCC (MinGW) or MSVC
REM ============================================================================

echo [BUILD] Checking for GCC (MinGW)...
where gcc >nul 2>nul
if %ERRORLEVEL% EQU 0 (
    echo [BUILD] Compiling with MinGW GCC...
    gcc -Wall server_windows.c -o server_windows.exe -lws2_32
    gcc -Wall client_windows.c -o client_windows.exe -lws2_32
    echo [SUCCESS] server_windows.exe and client_windows.exe built successfully!
    goto done
)

echo [BUILD] Checking for Microsoft Visual Studio cl.exe...
where cl >nul 2>nul
if %ERRORLEVEL% EQU 0 (
    echo [BUILD] Compiling with MSVC cl.exe...
    cl server_windows.c ws2_32.lib
    cl client_windows.c ws2_32.lib
    echo [SUCCESS] Binaries built with MSVC!
    goto done
)

echo [WARNING] Neither MinGW gcc nor MSVC cl was found in PATH.
echo You can run the included Python implementation:
echo   python sctp_server_windows.py
echo   python sctp_client_windows.py

:done
pause
