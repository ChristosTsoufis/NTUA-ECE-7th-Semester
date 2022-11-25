.globl main #

.equ N, 10

.data

A: .word 0, 1, 2, 7, -8, 4, 5, -12, 11, -2
B: .word 0, 1, 2, 7, -8, 4, 5, 12, -11, -2

.bss
C: .space 4*N

.text
main:
la t0, A                // load address of A in reg t0
la t1, B                // load address of B in reg t1
addi t1, t1, 4*(N-1)    // the value 4*(N-1) is added to the contents 
                        // of t1 and the result is placed in t1

// Rest of the program follows...

la t2, C                // load address of C in reg t2
li t3, N                // load the immediate value N and copy it in t3
                        // this will be the counter of the loop

loop:
lw t4, 0(t0)            // load word A[i] in reg t4
lw t5, 0(t1)            // load word B[N-i-1] in reg t5 
add t4, t4, t5          // the contents of t5 is added to the contents of t4
                        // and the result is stored in t4
bge t4, zero, absolute  // the contents of t4 is compared to zero
                        // if t4 is greater than or equal to zero
                        // control jumps to absolute
not t4, t4              // the contents of t4 is fetched and each of the bits 
                        // is flipped and the result is copied into t4
addi t4, t4, 0x00000001 // the value 0x00000001 is added to the contents of t4
                        // and the result is placed in t4
                        // and then continue

absolute:
sw t4, 0(t2)            // 0 is copied from reg t4 to memory. The memory address
                        // is formed by adding the offset to the contents of t2
addi t0, t0, 4          // the value 4 is added to the contents of t0
                        // and the result is placed in t0
                        // in order to get the next value of A
addi t1, t1, -4         // the value -4 is added to the contents of t1
                        // and the result is placed in t1
                        // in order to get the next value of B
addi t2, t2, 4          // the value 4 is added to the contents of t2
                        // and the result is placed in t2
                        // in order to get the next value of C
addi t3, t3, -1         // the value -1 is added to the contents of t3
                        // and the result is placed in t3
                        // in order to decrease the counter
bne t3, zero, loop      // the contents of t3 is compared to the contents of zero
                        // if not equal, control jumps to loop

.end