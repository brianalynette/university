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
	
	.globl main
	.text	
main:
	addi $a0, $zero, 0
	addi $a1, $zero, 0
	addi $a2, $zero, 0x00ff0000
	jal draw_bitmap_box
	
	addi $a0, $zero, 11
	addi $a1, $zero, 6
	addi $a2, $zero, 0x00ffff00
	jal draw_bitmap_box
	
	addi $a0, $zero, 8
	addi $a1, $zero, 8
	addi $a2, $zero, 0x0099ff33
	jal draw_bitmap_box
	
	addi $a0, $zero, 2
	addi $a1, $zero, 3
	addi $a2, $zero, 0x00000000
	jal draw_bitmap_box

	addi $v0, $zero, 10
	syscall
	
# STUDENTS MAY MODIFY CODE BELOW
# vvvvvvvvvvvvvvvvvvvvvvvvvvvvvv

# Name: Briana Johnson
# Student Number: V00929120

# Draws a 4x4 pixel box in the "Bitmap Display" tool
# $a0: row of box's upper-left corner
# $a1: column of box's upper-left corner
# $a2: colour of box
# $t0: counter

draw_bitmap_box:
	addi $sp, $sp, -4
	sw $ra, 0($sp)
	addi $t7, $zero, 4	# row counter
	addi $t6, $zero, 4	# column counter
	or $t3, $a0, $zero
	
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
	lw $ra, 0($sp)
	addi $sp, $sp, 4
	jr $ra

# ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
# STUDENTS MAY MODIFY CODE ABOVE
