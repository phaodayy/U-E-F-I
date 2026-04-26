.code
    launch_raw_hypercall proc
        push rbx
        cpuid         ; Hypervisor VMEXIT trigger
        pop rbx
        ret
    launch_raw_hypercall endp
END
