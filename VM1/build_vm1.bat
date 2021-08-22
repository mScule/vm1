call "shared.bat"

set src_vm1=src\vm1\
set bin_vm1=bin\vm1\

cd %bin_vm1%

gcc -std=c99 ..\..\%src_vm1%vm1.c -o vm1.exe

pause