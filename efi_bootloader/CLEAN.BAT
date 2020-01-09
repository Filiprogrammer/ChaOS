@echo off
cd %~d0%~p0
del obj\*.obj >nul 2>&1
del BOOTIA32.EFI >nul 2>&1
del efiboot.lib >nul 2>&1
del efiboot.map >nul 2>&1
del obj\object_files.lst >nul 2>&1
del efiboot.dll >nul 2>&1
del efiboot.debug >nul 2>&1
del efiboot.txt >nul 2>&1
