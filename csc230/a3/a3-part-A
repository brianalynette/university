
	.data
ARRAY_A:
	.word	21, 210, 49, 4
ARRAY_B:
	.word	21, -314159, 0x1000, 0x7fffffff, 3, 1, 4, 1, 5, 9, 2
ARRAY_Z:
	.space	28
NEWLINE:
	.asciiz "\n"
SPACE:
	.asciiz " "
		
	
	.text  
main:	
	la $a0, ARRAY_A
	addi $a1, $zero, 4
	jal dump_array
	
	la $a0, ARRAY_B
	addi $a1, $zero, 11
	jal dump_array
	
	la $a0, ARRAY_Z
	lw $t0, 0($a0)
	addi $t0, $t0, 1
	sw $t0, 0($a0)
	addi $a1, $zero, 9
	jal dump_array
		
	addi $v0, $zero, 10
	syscall

# STUDENTS MAY MODIFY CODE BELOW
# vvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
	
# Name: Briana Johnson
# Student Number: V00929120

# $t1: copy of $a0 (address)
# $a0: actual value at $a0
	
dump_array:
	addi $sp, $sp, -4
	sw $ra, 0($sp)
	or $t1, $a0, $zero
	
dump_loop:
	lw $a0, 0($t1)
	beq $a1, $zero, done_dump 

	addi $v0, $zero, 1
	syscall
	
	la $a0, SPACE
	addi $v0, $zero, 4
	syscall
	
	addi $a1, $a1, -1
	addi $t1, $t1, 4
	or $a0, $t1, $zero
	j dump_loop
	
done_dump:
	la $a0, NEWLINE
	addi $v0, $zero, 4
	syscall
	
	lw $ra, 0($sp)
	addi $sp, $sp, 4
	jr $ra
	
	
	
# ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
# STUDENTS MAY MODIFY CODE ABOVE
