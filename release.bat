if "%VCINSTALLDIR%"=="" call "C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\vcvarsall" x64
move *.asm \tmp
cl /Brepro /Fa /Feayane /I\mpir /O2 *.cc \mpir\lib\x64\Release\mpir.lib
