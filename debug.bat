if "%VCINSTALLDIR%"=="" call "C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\vcvarsall" x64
clang-cl -I/mpir -Wno-deprecated-declarations *.cc
if errorlevel 1 goto :eof
cl /DDEBUG /Feayane /I\mpir /MP /MTd /Zi *.cc \mpir\lib\x64\Debug\mpir.lib dbghelp.lib
if errorlevel 1 goto :eof
