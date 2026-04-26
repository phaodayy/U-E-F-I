.code
    launch_raw_hypercall proc
        push rbx
        cpuid
        pop rbx
        ret
    launch_raw_hypercall endp
END
