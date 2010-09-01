#!/bin/sh
# Copy extra files to the Mac OS/X application bundle
## Christine Montgomery, August 2006
## Ralin Redgrave, September 2008

# Bundle the data files in the right place
cd ../bin
cp -R ../other/osx/QAvimator.app .
cp -R data QAvimator.app/Contents/Resources/

# Bundle needed Qt4 frameworks
cp -R /Library/Frameworks/QtCore.framework/Versions/4/QtCore QAvimator.app/Contents/Frameworks/
install_name_tool -id @executable_path/../Frameworks/QtCore QAvimator.app/Contents/Frameworks/QtCore
install_name_tool -change  QtCore.framework/Versions/4/QtCore @executable_path/../Frameworks/QtCore QAvimator.app/Contents/MacOS/qavimator

cp -R /Library/Frameworks/QtGui.framework/Versions/4/QtGui QAvimator.app/Contents/Frameworks/
install_name_tool -id @executable_path/../Frameworks/QtGui QAvimator.app/Contents/Frameworks/QtGui
install_name_tool -change  QtCore.framework/Versions/4/QtCore @executable_path/../Frameworks/QtCore QAvimator.app/Contents/Frameworks/QtGui
install_name_tool -change  QtGui.framework/Versions/4/QtGui @executable_path/../Frameworks/QtGui QAvimator.app/Contents/MacOS/qavimator

cp -R /Library/Frameworks/QtOpenGL.framework/Versions/4/QtOpenGL QAvimator.app/Contents/Frameworks/
install_name_tool -id @executable_path/../Frameworks/QtOpenGL QAvimator.app/Contents/Frameworks/QtOpenGL
install_name_tool -change  QtCore.framework/Versions/4/QtCore @executable_path/../Frameworks/QtCore QAvimator.app/Contents/Frameworks/QtOpenGL
install_name_tool -change  QtGui.framework/Versions/4/QtGui @executable_path/../Frameworks/QtGui QAvimator.app/Contents/Frameworks//QtOpenGL
install_name_tool -change  QtOpenGL.framework/Versions/4/QtOpenGL @executable_path/../Frameworks/QtOpenGL QAvimator.app/Contents/MacOS/qavimator
