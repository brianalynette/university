	.data
KEYBOARD_EVENT_PENDING:
	.word	0x0
KEYBOARD_EVENT:
	.word   0x0
KEYBOARD_COUNTS:
	.space  128
NEWLINE:
	.asciiz "\n"
SPACE:
	.asciiz " "
	
	
	.eqv 	LETTER_a 97
	.eqv	LETTER_b 98
	.eqv	LETTER_c 99
	.eqv 	LETTER_D 100
	.eqv 	LETTER_space 32
	
	
	.text  
main:
# STUDENTS MAY MODIFY CODE BELOW
# vvvvvvvvvvvvvvvvvvvvvvvvvvvvvv

# Name: Briana Johnson
# Student Number: V00929120

# address of storage for the list
# load the first char
# subtract 'a' to get the offset
# add that to the storage address to get current part to increment
# increment counter at that byte
# return in proper register
	
	la $s0, 0xffff0000	# ctrl reg for MMIO simulator receiver
	lb $s1, 0($s0)		
	ori $s1, $s1, 0x02	# set bit 1 to enable receiver interrupts
	sb $s1, 0($s0)		
	
check_for_event:
	# check if a character has been entered
	lbu $s0, KEYBOARD_EVENT_PENDING
	beq $s0, $zero, check_for_event
	
	#if so, $s1 shall be the character's ascii value
	# check for space key - if so, we'll quit
	lbu $s1, KEYBOARD_EVENT
	beq $s1, ' ', print_and_exit

	#otherwise, check the letter, and increment it's count
inc_counter:
	la $s0, KEYBOARD_COUNTS
	sub $s1, $s1, 'a' 
	add $s0, $s0, $s1
	lb $s2, 0($s0)
	addi $s2, $s2, 1
	sb $s2, 0($s0)
	j finished_processing_char
	
finished_processing_char:
	# set "event pending" flag back to zero
	sb $zero, KEYBOARD_EVENT_PENDING
	j check_for_event
	
print_and_exit:
	la $s0, KEYBOARD_COUNTS
	lb $a0, 0($s0)		# print count of a
	addi $v0, $zero, 1
	syscall
	
	la $a0, SPACE
	addi $v0, $zero, 4
	syscall
	
	lb $a0, 1($s0)		# print count of b
	addi $v0, $zero, 1
	syscall
	
	la $a0, SPACE
	addi $v0, $zero, 4
	syscall
	
	lb $a0, 2($s0)		# print count of c
	addi $v0, $zero, 1
	syscall
	
	la $a0, SPACE
	addi $v0, $zero, 4
	syscall
	
	lb $a0, 3($s0)		# print count of d
	addi $v0, $zero, 1
	syscall
	
	la $a0, NEWLINE
	addi $v0, $zero, 4
	syscall
	
	j finished_processing_char
	
	#######################################################	
	
	.kdata

	.ktext 0x80000180
__kernel_entry:
	mfc0 $k0, $13		# $13 is the "cause" register in Coproc0
	andi $k1, $k0, 0x7c	# bits 2 to 6 are the ExcCode field (0 for interrupts)
	srl  $k1, $k1, 2	# shift ExcCode bits for easier comparison
	beq $zero, $k1, __is_interrupt
	
__is_exception:
	# Something of a placeholder...
	# ... just in case we can't escape the need for handling some exceptions.
	beq $zero, $zero, __exit_exception
	
__is_interrupt:
	andi $k1, $k0, 0x0100		  # examine bit 8
	bne $k1, $zero, __is_keyboard_interrupt # if bit 8 set, then we have a keyboard interrupt.
	
	beq $zero, $zero, __exit_exception
	
__is_keyboard_interrupt:
	lb $k0, 0xffff0004
	sb $k0, KEYBOARD_EVENT
	sb $k0, KEYBOARD_EVENT_PENDING	# put any value other than zero into event pending
	addi $k0, $zero, 1
	beq $zero, $zero, __exit_exception
	
__exit_exception:
	eret
	
	
# ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
# STUDENTS MAY MODIFY CODE ABOVE
