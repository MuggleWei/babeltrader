::vcpkg.exe install openssl:x64-windows zlib:x64-windows
md build
cd build
cmake .. -DBUILD_TESTING=OFF -DCMAKE_TOOLCHAIN_FILE=%VCPKG_ROOT%\scripts\buildsystems\vcpkg.cmake -G "Visual Studio 14 2015 Win64"