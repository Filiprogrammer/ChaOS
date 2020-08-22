target remote localhost:1234
add-symbol-file "kernel/CHAOSKRN.SYS_debug" 0x40000
set substitute-path 'src' 'kernel/src'