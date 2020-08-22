@echo off
cd %~d0%~p0

cecho {0A}Building BaseDebugPrintErrorLevelLib...{#}{\n}
cd BaseDebugPrintErrorLevelLib
call BUILD.BAT || goto ERROR
cd ..
cecho {0A}BaseDebugPrintErrorLevelLib was built{#}{\n\n}

cecho {0A}Building BaseLib...{#}{\n}
cd BaseLib
call BUILD.BAT || goto ERROR
cd ..
cecho {0A}BaseLib was built{#}{\n\n}

cecho {0A}Building BaseMemoryLib...{#}{\n}
cd BaseMemoryLib
call BUILD.BAT || goto ERROR
cd ..
cecho {0A}BaseMemoryLib was built{#}{\n\n}

cecho {0A}Building BasePcdLibNull...{#}{\n}
cd BasePcdLibNull
call BUILD.BAT || goto ERROR
cd ..
cecho {0A}BasePcdLibNull was built{#}{\n\n}

cecho {0A}Building BasePrintLib...{#}{\n}
cd BasePrintLib
call BUILD.BAT || goto ERROR
cd ..
cecho {0A}BasePrintLib was built{#}{\n\n}

cecho {0A}Building UefiApplicationEntryPoint...{#}{\n}
cd UefiApplicationEntryPoint
call BUILD.BAT || goto ERROR
cd ..
cecho {0A}UefiApplicationEntryPoint was built{#}{\n\n}

cecho {0A}Building UefiBootServicesTableLib...{#}{\n}
cd UefiBootServicesTableLib
call BUILD.BAT || goto ERROR
cd ..
cecho {0A}UefiBootServicesTableLib was built{#}{\n\n}

cecho {0A}Building UefiDebugLibStdErr...{#}{\n}
cd UefiDebugLibStdErr
call BUILD.BAT || goto ERROR
cd ..
cecho {0A}UefiDebugLibStdErr was built{#}{\n\n}

cecho {0A}Building UefiDevicePathLib...{#}{\n}
cd UefiDevicePathLib
call BUILD.BAT || goto ERROR
cd ..
cecho {0A}UefiDevicePathLib was built{#}{\n\n}

cecho {0A}Building UefiLib...{#}{\n}
cd UefiLib
call BUILD.BAT || goto ERROR
cd ..
cecho {0A}UefiLib was built{#}{\n\n}

cecho {0A}Building UefiMemoryAllocationLib...{#}{\n}
cd UefiMemoryAllocationLib
call BUILD.BAT || goto ERROR
cd ..
cecho {0A}UefiMemoryAllocationLib was built{#}{\n\n}

cecho {0A}Building UefiRuntimeServicesTableLib...{#}{\n}
cd UefiRuntimeServicesTableLib
call BUILD.BAT || goto ERROR
cd ..
cecho {0A}UefiRuntimeServicesTableLib was built{#}{\n\n}

goto EOF

:ERROR
cecho {0C}Error{#}{\n}
EXIT /B 1

:EOF
