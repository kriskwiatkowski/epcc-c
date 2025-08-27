// Number of rounds in the kaccak. Change to 12-1 for TurboShake
#define KECCAK_ROUNDS (24-1)
// Location of a round counter on a stack
#define COUNT_LOC #(8*1)
#define IOTA_LOC #(8*2)
#define TMP_LOC #(8*3)
#define COUNT_REG x28
#define IOTA_REG x27

#ifdef __APPLE__
IOTA24:
    .quad   0x0000000000000001
    .quad   0x0000000000008082
    .quad   0x800000000000808a
    .quad   0x8000000080008000
    .quad   0x000000000000808b
    .quad   0x0000000080000001
    .quad   0x8000000080008081
    .quad   0x8000000000008009
    .quad   0x000000000000008a
    .quad   0x0000000000000088
    .quad   0x0000000080008009
    .quad   0x000000008000000a
IOTA12:
    .quad   0x000000008000808b
    .quad   0x800000000000008b
    .quad   0x8000000000008089
    .quad   0x8000000000008003
    .quad   0x8000000000008002
    .quad   0x8000000000000080
    .quad   0x000000000000800a
    .quad   0x800000008000000a
    .quad   0x8000000080008081
    .quad   0x8000000000008080
    .quad   0x0000000080000001
    .quad   0x8000000080008008
#else /* !__APPLE__ */
.align 8
.type IOTA24,%object
IOTA24:
    .quad   0x0000000000000001
    .quad   0x0000000000008082
    .quad   0x800000000000808a
    .quad   0x8000000080008000
    .quad   0x000000000000808b
    .quad   0x0000000080000001
    .quad   0x8000000080008081
    .quad   0x8000000000008009
    .quad   0x000000000000008a
    .quad   0x0000000000000088
    .quad   0x0000000080008009
    .quad   0x000000008000000a
IOTA12:
    .quad   0x000000008000808b
    .quad   0x800000000000008b
    .quad   0x8000000000008089
    .quad   0x8000000000008003
    .quad   0x8000000000008002
    .quad   0x8000000000000080
    .quad   0x000000000000800a
    .quad   0x800000008000000a
    .quad   0x8000000080008081
    .quad   0x8000000000008080
    .quad   0x0000000080000001
    .quad   0x8000000080008008
.size   IOTA24,.-IOTA24
#endif

#ifdef __APPLE__
.macro ldr_iota_ptr
    adrp IOTA_REG, IOTA24@PAGE
    add  IOTA_REG, IOTA_REG, :lo12:IOTA24@PAGEOFF
.endm
#else
.macro ldr_iota_ptr
    adrp IOTA_REG, IOTA24
    add  IOTA_REG, IOTA_REG, :lo12:IOTA24
.endm
#endif

.macro ldr_iota_val
    // OZAPTF: just save current address?
    ldr IOTA_REG, [sp, IOTA_LOC]
    ldr IOTA_REG, [IOTA_REG, COUNT_REG, lsl #3]
.endm

// Stores callee-saved registers
.macro STR_PROC_REGS
    stp x19, x20, [sp, #-16]!
    stp x21, x22, [sp, #-16]!
    stp x23, x24, [sp, #-16]!
    stp x25, x26, [sp, #-16]!
    stp x27, x28, [sp, #-16]!
    stp x29, x30, [sp, #-16]! // LR and SP
    // OZAPTF: Why 32 on MAC not 16?
    sub sp, sp, #48           // Leave a space to store round counter
                              // Note, sp address must be 16-byte aligned
.endm

// Load callee-saved registers
.macro LDR_PROC_REGS
    add sp, sp, #48
    ldp x29, x30, [sp], #16
    ldp x27, x28, [sp], #16
    ldp x25, x26, [sp], #16
    ldp x23, x24, [sp], #16
    ldp x21, x22, [sp], #16
    ldp x19, x20, [sp], #16
.endm

.macro STRST
    stp x0,x1,  [x26,#16*0]
    stp x2,x3,  [x26,#16*1]
    stp x4,x5,  [x26,#16*2]
    stp x6,x7,  [x26,#16*3]
    stp x8,x9,  [x26,#16*4]
    stp x10,x11,[x26,#16*5]
    stp x12,x13,[x26,#16*6]
    stp x14,x15,[x26,#16*7]
    stp x16,x17,[x26,#16*8]
    stp x25,x19,[x26,#16*9]
    stp x20,x21,[x26,#16*10]
    stp x22,x23,[x26,#16*11]
    str x24,    [x26,#16*12]
.endm

.macro CHI aa1,aa2,aa3,aa4,aa5,c1,c2,c3,c4
    bic  \c1,\aa3,\aa2
    bic  \c2,\aa4,\aa3
    bic  \c3,\aa5,\aa4
    bic  \c4,\aa1,\aa5
        eor \aa3,\aa3, \c3
    bic  \c3,\aa2,\aa1
    eor \aa1,\aa1, \c1
    eor \aa2,\aa2, \c2
    eor \aa4,\aa4, \c4
    eor \aa5,\aa5, \c3
.endm

.macro ldr_count
    ldr COUNT_REG, [sp, COUNT_LOC]
.endm

.macro str_count
    str COUNT_REG, [sp, COUNT_LOC]
.endm

.macro stp_tmp aa1,aa2
    stp \aa1,\aa2,[sp, TMP_LOC]
.endm

.macro ldp_tmp aa1,aa2
    ldp \aa1,\aa2,[sp, TMP_LOC]
.endm

.macro do_Full_Theta
    // Calculate C0-C4 (mixed to relax contention on registers).
    eor x27, x0, x5  // C0
    eor x29, x1, x6  // C1
    eor x28,x10,x15  // C0
    eor x30,x11,x16  // C1
        stp_tmp x19,x24 // Offload pair, use x19, x24 as temporary registers for C4
    eor x28,x28,x20  // C0
    eor x30,x30,x21  // C1
    eor x27,x27,x28  // C0 -> x27
    eor x29,x29,x30  // C1 -> x29
    eor x28, x2, x7  // C2
    eor x30, x3, x8  // C3
    eor x19,x19,x24  // C4
    eor x28,x28,x12  // C2 // TODO you can use x24 to speed up something
    eor x30,x30,x13  // C3
    eor x19,x19, x4  // C4
    eor x28,x28,x17  // C2
    eor x30,x30,x25  // C3
    eor x19,x19, x9  // C4
    eor x28,x28,x22  // C2 -> x28
    eor x30,x30,x23  // C3 -> x30
    eor x19,x19,x14  // C4 -> x19

    // Now we want to release x19 and x24 as soon as possible
    // Start with calculating D0
    eor x24,x19,x29, ror #63
    eor  x0, x0,x24
    eor  x5, x5,x24
    eor x10,x10,x24
    eor x15,x15,x24
    eor x20,x20,x24

    // D3
    eor x19,x28,x19, ror #63
    eor  x3, x3,x19
    eor  x8, x8,x19
    eor x13,x13,x19
    eor x25,x25,x19
    eor x23,x23,x19

    // x19 released now
    eor x24,x27,x28, ror #63 // D1
    eor x19,x29,x30, ror #63 // D2
    eor  x1, x1,x24
    eor  x2, x2,x19
    eor  x6, x6,x24
    eor  x7, x7,x19
    eor x11,x11,x24
    eor x12,x12,x19
    eor x16,x16,x24
    eor x17,x17,x19
    eor x21,x21,x24
    eor x22,x22,x19

    // D4
    eor x28,x30,x27, ror #63
    eor  x4, x4,x28
        ldp_tmp x19,x24
    eor  x9, x9,x28
    eor x14, x14,x28
    eor x19, x19,x28
    eor x24, x24,x28
.endm

.macro do_Load_State_and_Init_Theta
    // x26 keeps the pointer to the array
    // Calculate C0-C4 (mixed to relax contention on registers).
        ldp x0,x1,  [x26,#16*0]
        ldp x4,x5,  [x26,#16*2]
        ldp x6,x7,  [x26,#16*3]
        ldp x10,x11,[x26,#16*5]
    eor x27, x0, x5  // C0
        ldp x14,x15,[x26,#16*7]
    eor x29, x1, x6  // C1
        ldp x16,x17,[x26,#16*8]
        ldp x20,x21,[x26,#16*10]
    eor x28,x10,x15  // C0
    eor x30,x11,x16  // C1
        ldp x2,x3,  [x26,#16*1]
    eor x28,x28,x20  // C0
    eor x30,x30,x21  // C1
        ldp x8,x9,  [x26,#16*4]
    eor x27,x27,x28  // C0 -> x27
    eor x29,x29,x30  // C1 -> x29
        ldp x23,x24,[x26,#(16*11)+8]
    eor x28, x2, x7  // C2
        ldp x25,x19,[x26,#16*9] // don't use x18 (platform reg.)
        ldp x12,x13,[x26,#16*6]
    eor x30, x3, x8  // C3
    eor x19,x19,x24  // C4
    eor x28,x28,x12  // C2
    eor x30,x30,x13  // C3
        ldr x22,[x26,#16*11]
    eor x19,x19, x4  // C4
    eor x28,x28,x17  // C2
    eor x30,x30,x25  // C3
    eor x19,x19, x9  // C4
    eor x28,x28,x22  // C2 -> x28
    eor x30,x30,x23  // C3 -> x30
    eor x19,x19,x14  // C4 -> x19
.endm

.macro do_finish_Theta
    // Now we want to release x19 and x24 as soon as possible
    // Start with calculating D0
    eor x24,x19,x29, ror #63
    eor  x0, x0,x24
    eor  x5, x5,x24
    eor x10,x10,x24
    eor x15,x15,x24
    eor x20,x20,x24

    // D3
    eor x19,x28,x19, ror #63
    eor  x3, x3,x19
    eor  x8, x8,x19
    eor x13,x13,x19
    eor x25,x25,x19
    eor x23,x23,x19

    // x19 released now
    eor x24,x27,x28, ror #63 // D1
    eor x19,x29,x30, ror #63 // D2
    eor  x1, x1,x24
    eor  x2, x2,x19
    eor  x6, x6,x24
    eor  x7, x7,x19
    eor x11,x11,x24
    eor x12,x12,x19
    eor x16,x16,x24
    eor x17,x17,x19
    eor x21,x21,x24
    eor x22,x22,x19

    // D4
    eor x28,x30,x27, ror #63
        ldr x24,[x26,#16*12]
    eor  x4, x4,x28
        ldr x19,[x26,#(16*9)+8]
    eor  x9, x9,x28
    eor x14, x14,x28
    eor x19, x19,x28
    eor x24, x24,x28
.endm

.macro do_Rho_Pi
    mov x27, x1
    mov x28, x2
    mov x29, x3
    mov x30, x4

    ror  x1,  x6, #(64 - 44)
    ror  x2, x12, #(64 - 43)
    ror  x3, x25, #(64 - 21)
    ror  x4, x24, #(64 - 14)

    ror  x6,  x9, #(64 - 20)
    ror x12, x13, #(64 - 25)
    ror x25, x17, #(64 - 15)
    ror x24, x21, #(64 -  2)

    ror  x9, x22, #(64 - 61)
    ror x13, x19, #(64 -  8)
    ror x17, x11, #(64 - 10)
    ror x21,  x8, #(64 - 55)

    ror x22, x14, #(64 - 39)
    ror x19, x23, #(64 - 56)
    ror x11,  x7, #(64 -  6)
    ror  x8, x16, #(64 - 45)

    ror x14, x20, #(64 - 18)
    ror x23, x15, #(64 - 41)
    ror  x7, x10, #(64 -  3)
    ror x16,  x5, #(64 - 36)

    ror x20, x28, #(64 - 62)
    ror x15, x30, #(64 - 27)
    ror x10, x27, #(64 -  1)
    ror  x5, x29, #(64 - 28)
.endm

.macro do_Full_Chi
    CHI  x0, x1, x2, x3, x4,x27,x28,x29,x30
    CHI  x5, x6, x7, x8, x9,x27,x28,x29,x30
    CHI x10,x11,x12,x13,x14,x27,x28,x29,x30
    CHI x15,x16,x17,x25,x19,x27,x28,x29,x30
    CHI x20,x21,x22,x23,x24,x27,x28,x29,x30
.endm

.macro do_Init_Chi
    CHI  x0, x1, x2, x3, x4,x27,x28,x29,x30
    CHI  x5, x6, x7, x8, x9,x27,x28,x29,x30
.endm

.macro do_Store_State_and_Finish_Chi
        stp x0,x1,  [x26,#16*0]
    bic x27,x12,x11
    bic x28,x13,x12
        stp x2,x3,  [x26,#16*1]
    bic x29,x14,x13
    bic x30,x10,x14
        stp x4,x5,  [x26,#16*2]
    eor x12,x12,x29
    bic x29,x11,x10
        stp x6,x7,  [x26,#16*3]
    eor x10,x10,x27
    eor x11,x11,x28
        stp x8,x9,  [x26,#16*4]
    eor x13,x13,x30
    eor x14,x14,x29
        stp x10,x11,[x26,#16*5]

    bic x27,x17,x16
    bic x28,x25,x17
    bic x29,x19,x25
    bic x30,x15,x19
        stp x12,x13,[x26,#16*6]
    eor x17,x17,x29
    bic x29,x16,x15
    eor x15,x15,x27
    eor x16,x16,x28
    eor x25,x25,x30
    eor x19,x19,x29
        stp x14,x15,[x26,#16*7]

    bic x27,x22,x21
    bic x28,x23,x22
        stp x16,x17,[x26,#16*8]
    bic x29,x24,x23
    bic x30,x20,x24
    eor x22,x22,x29
        stp x25,x19,[x26,#16*9]
    bic x29,x21,x20
    eor x20,x20,x27
    eor x21,x21,x28
    eor x23,x23,x30
    eor x24,x24,x29

    stp x20,x21,[x26,#16*10]
    stp x22,x23,[x26,#16*11]
    str x24,    [x26,#16*12]
.endm

// Performs Iota. Loads COUNT_REG
.macro do_Iota
    ldr_count
    ldr_iota_val
    eor x0, x0, IOTA_REG
.endm

#ifdef __APPLE__
.text
.p2align 4,,15
.globl _keccakf1600_asm
_keccakf1600_asm:
#else
.text
.p2align 4,,15
.globl keccakf1600_asm
.hidden keccakf1600_asm
.type keccakf1600_asm, @function
keccakf1600_asm:
#endif
    STR_PROC_REGS
    // Move pointer to the array storing state to x26
    mov x26,x0
    // Store pointer to IOTA's on the stack
    ldr_iota_ptr
    str IOTA_REG, [sp, IOTA_LOC]

    do_Load_State_and_Init_Theta
    do_finish_Theta
    do_Rho_Pi
    do_Full_Chi

    // Process Iota separately. First one, hence round counter is 0
    mov COUNT_REG, #0
    ldr_iota_val
    eor x0, x0, IOTA_REG

    // Round counter is 1
    mov COUNT_REG, #1

.Loop:
    str_count

    do_Full_Theta
    do_Rho_Pi
    do_Full_Chi
    do_Iota

    // Increase round counter, loaded by do_Iota
    add COUNT_REG, COUNT_REG, #1
    cmp COUNT_REG, KECCAK_ROUNDS
    b.ne .Loop

    do_Full_Theta
    do_Rho_Pi
    do_Init_Chi
    // Iota - Init_Chi calculates X0, so now we can apply iota
    mov COUNT_REG, KECCAK_ROUNDS
    ldr_iota_val
    eor x0, x0, IOTA_REG
    //
    do_Store_State_and_Finish_Chi

    LDR_PROC_REGS
    blr lr
