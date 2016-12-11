@echo off

cd "%~dp0"
if errorlevel 1 goto error

if not exist _build.emscripten mkdir _build.emscripten
if errorlevel 1 goto error

cd _build.emscripten
cmake -G "MinGW Makefiles" ^
    -DCMAKE_BUILD_TYPE=Release ^
    -DCMAKE_MODULE_PATH:PATH="%~dp0cmake" ^
    -DCMAKE_TOOLCHAIN_FILE="%~dp0cmake/Emscripten.cmake" ^
    ..
if errorlevel 1 goto error

mingw32-make -j 4
if errorlevel 1 goto error

:end
exit /b 0

:error
echo *** ERROR ***
exit /b 1
