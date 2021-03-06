# This code assumes the use of the "Bitmap Display" tool.
#
# Tool settings must be:
#   Unit Width in Pixels: 32
#   Unit Height in Pixels: 32
#   Display Width in Pixels: 512
#   Display Height in Pixels: 512
#   Based Address for display: 0x10010000 (static data)
#
# In effect, this produces a bitmap display of 16x16 pixels.


	.include "bitmap-routines.asm"

	.data
TELL_TALE:
	.word 0x12345678 0x9abcdef0	# Helps us visually detect where our part starts in .data section
KEYBOARD_EVENT_PENDING:
	.word	0x0
KEYBOARD_EVENT:
	.word   0x0
BOX_ROW:
	.word	0x0
BOX_COLUMN:
	.word	0x0

	.eqv LETTER_a 97
	.eqv LETTER_d 100
	.eqv LETTER_w 119
	.eqv LETTER_x 120
	.eqv BOX_COLOUR 0x0099ff33
	
	.globl main
	
	.text	
main:
# STUDENTS MAY MODIFY CODE BELOW
# vvvvvvvvvvvvvvvvvvvvvvvvvvvvvv

# Name: Briana Johnson
# Student Number: V00929120

init_box:
	# set up values
	la $t2, TELL_TALE
	li $a0, 0
	li $a1, 0
	
	# initialize box in white
	add $a2, $zero, BOX_COLOUR
	jal draw_bitmap_box
	

	la $s0, 0xffff0000	# ctrl reg for MMIO simulator receiver
	lb $s1, 0($s0)		
	ori $s1, $s1, 0x02	# set bit 1 to enable receiver interrupts
	sb $s1, 0($s0)		
	
check_for_event:
	lbu $s0, KEYBOARD_EVENT_PENDING
	beq $s0, $zero, check_for_event	
	
	j erase_box
	
	
	# Should never, *ever* arrive at this point
	# in the code.	

	addi $v0, $zero, 10

.data
    .eqv BOX_COLOUR_BLACK 0x00000000
    
.text
	addi $v0, $zero, BOX_COLOUR_BLACK
	syscall

erase_box:
	# store current location in TELL_TALE
	sw $a0, 0($t2)
	sw $a1, 4($t2)
	
	# set box colour to black
	add $a2, $zero, BOX_COLOUR_BLACK
	
	jal draw_bitmap_box
	
find_direction:
	add $a2, $zero, BOX_COLOUR
	# load what the keyboard event was
	lb $s1, KEYBOARD_EVENT
	
	# test to see which key was pressed
	beq $s1, 'd', go_right
	beq $s1, 'a', go_left
	beq $s1, 'w', go_up
	beq $s1, 'x', go_down
	
	# if a random key was pressed, exit
	jal draw_bitmap_box
	j finished_processing_char

go_right:
	addi $a1, $a1, 1
	sw $a1, BOX_COLUMN
	jal draw_bitmap_box
	j finished_processing_char

go_left:
	addi $a1, $a1, -1
	sw $a1, BOX_COLUMN
	jal draw_bitmap_box
	j finished_processing_char
	
go_up:
	addi $a0, $a0, -1
	sw $a0, BOX_ROW
	jal draw_bitmap_box
	j finished_processing_char
	
go_down:
	addi $a0, $a0, 1
	sw $a0, BOX_ROW
	jal draw_bitmap_box
	j finished_processing_char
	
finished_processing_char:
	# set "event pending" flag back to zero
	sb $zero, KEYBOARD_EVENT_PENDING
	j check_for_event

# Draws a 4x4 pixel box in the "Bitmap Display" tool
# $a0: row of box's upper-left corner
# $a1: column of box's upper-left corner
# $a2: colour of box

draw_bitmap_box:
	addi $sp, $sp, -4
	sw $ra, 0($sp)
	addi $t7, $zero, 4	# row counter
	addi $t6, $zero, 4	# column counter
	or $t3, $a0, $zero
	or $t4, $a1, $zero
	
row_loop:
	beq $t7, $zero, next_row
	addi $t7, $t7, -1
	jal set_pixel
	addi $a0, $a0, 1
	j row_loop
	
next_row:
	addi $t7, $t7, 4
	or $a0, $t3, $zero
	addi $a1, $a1, 1
	addi $t6, $t6, -1
	bne $t6, $zero, row_loop

 done: 
 	or $a0, $t3 $zero
 	or $a1, $t4, $zero
	lw $ra, 0($sp)
	addi $sp, $sp, 4
	jr $ra


	.kdata

	.ktext 0x80000180
#
# You can copy-and-paste some of your code from part (a)
# to provide elements of the interrupt handler.
#
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


.data

# Any additional .text area "variables" that you need can
# be added in this spot. The assembler will ensure that whatever
# directives appear here will be placed in memory following the
# data items at the top of this file.

	
# ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
# STUDENTS MAY MODIFY CODE ABOVE


.eqv BOX_COLOUR_WHITE 0x00FFFFFF
	
