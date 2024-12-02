@echo off

:: Config
set project_root=%~dp0%
set build_folder=%project_root%\build\
set exe_name=make_board.exe

:: Build executable
if not exist %build_folder% (mkdir %build_folder%)

pushd %build_folder%
  cl /MD -DDEBUG=1 /O2 -nologo -Zo -Z7 %project_root%\make_board.c /link -subsystem:console -incremental:no -opt:ref -OUT:%exe_name%

  IF %errorlevel% NEQ 0 (popd && goto end)

  .\%exe_name% data/16Star.txt -n 200000
popd

:end

exit /B %errorlevel%
