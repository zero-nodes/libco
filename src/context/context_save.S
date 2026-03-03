.section .text
.code64
.intel_syntax noprefix

.global context_save
context_save:
    push r15
    push r14
    push r13
    push r12
    push rbp
    push rbx
    
    push r9
    push r8
    push rcx
    push rdx
    push rsi
    push rdi
    
    mov [rdi], rsp
    
    add rsp, 48
    
    pop rbx
    pop rbp
    pop r12
    pop r13
    pop r14
    pop r15
    
    ret

.section .note.GNU-stack,"",@progbits
