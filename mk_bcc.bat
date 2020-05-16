@echo off
bcc -mt -lt -O -G -Z -f- -d -w+            -ecrc_c.com    crc_test.c crc.c
if errorlevel 1 goto err
bcc -mt -lt -O -G -Z -f- -d -w+            -ecrc_asm.com  crc_test.c crc.asm
if errorlevel 1 goto err
bcc -mt -lt -O -G -Z -f- -d -w+ -DCRC_TINY -ecrc_tiny.com crc_test.c crc_tiny.c
if errorlevel 1 goto err
goto end
:err
echo Errors!
:end
if exist crc_test.obj del crc_test.obj
if exist crc.obj      del crc.obj
if exist crc_tiny.obj del crc_tiny.obj
