mkdir _build
cd _build

:: Edit path to your Qt5 installation
set QT_DIR=C:\Work\Qt\5.0.1

cmake .. -G "MinGW Makefiles" -DCMAKE_INSTALL_PREFIX=../Animik/ -DCMAKE_PREFIX_PATH=%QT_DIR%\lib\cmake %*
cmake --build . --target install

copy %QT_DIR%\bin\Qt5Core.dll ..\Animik
copy %QT_DIR%\bin\Qt5Gui.dll ..\Animik
copy %QT_DIR%\bin\Qt5OpenGL.dll ..\Animik
copy %QT_DIR%\bin\Qt5Widgets.dll ..\Animik
copy %QT_DIR%\bin\Qt5Xml.dll ..\Animik

cd ..
