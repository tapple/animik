mkdir _build
cd _build

cmake .. -DCMAKE_INSTALL_PREFIX=../Animik -G "MinGW Makefiles" %*
cmake --build . --target install
