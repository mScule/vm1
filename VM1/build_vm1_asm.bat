call "shared.bat"

set src_vm1_asm=src\vm1_asm\
set bin_vm1_asm=bin\vm1_asm\

cd %bin_vm1_asm%

gcc -std=c99 ..\..\%src_vm1_asm%vm1_asm.c ..\..\%src_shared%str.c -o vm1_asm.exe

pause