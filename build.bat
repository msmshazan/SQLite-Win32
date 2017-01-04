@echo off
set CompilerFlags= /Z7 /FC /nologo /Od -fp:fast -Gm- -GR- -EHa- /Oi -WX -W4 -wd4189 -wd4505 -wd4100 -wd4201
set LinkerFlags=-subsystem:console 
set bits=x86
set LibraryLocation=..\deps\lib\%bits%\

mkdir build > NUL 2> NUL
pushd build
ctime -begin database.ctm
REM Change the compiler location to MSVC location in the PC
call "C:\Program Files (x86)\Microsoft Visual Studio 11.0\VC\vcvarsall.bat" %bits%
del *.pdb > NUL 2> NUL
del *.dll > NUL 2> NUL
xcopy %LibraryLocation%sqlite3.dll > NUL 2> NUL
cl %CompilerFlags% /MD ..\code\main.cpp /I..\deps\include  /link -incremental:no /LIBPATH:%LibraryLocation%  winmm.lib user32.lib gdi32.lib kernel32.lib sqlite3.lib  %LinkerFlags%
set LastError=%ERRORLEVEL%
ctime -end database.ctm %LastError%
popd
