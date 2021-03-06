	.data
	
GEN_A:	.space 256
GEN_B:	.space 256
GEN_Z:	.space 256

TEST_A_16x16:
	.byte  	9 3 8 5 7 0 8 3 9 3 4 7 3 5 7 1
       		4 3 2 1 7 8 5 3 7 5 3 6 6 3 1 6
       		7 4 3 1 9 5 4 6 6 3 6 1 6 6 0 7
       		3 8 1 5 0 5 5 0 4 9 2 0 6 2 4 1
       		2 6 5 9 7 2 7 8 4 2 8 0 1 1 0 9
       		5 8 7 1 9 9 7 2 2 3 8 7 2 1 2 4
       		5 6 1 0 8 8 5 7 0 3 4 5 1 4 2 4
       		7 3 6 1 8 5 3 1 4 2 0 0 6 9 7 9
       		0 5 3 4 7 3 8 9 8 5 5 0 2 4 5 5
       		6 6 0 3 8 1 3 2 1 2 5 1 5 0 7 3
       		5 8 8 3 2 7 8 8 5 4 4 4 3 6 3 7
       		4 0 3 0 9 5 7 7 0 4 8 3 0 7 9 0
       		0 6 7 4 9 2 7 0 0 4 9 1 1 9 7 5
       		8 1 2 7 6 1 4 0 3 5 3 8 1 3 3 2
       		2 9 3 7 2 0 3 8 8 3 1 9 8 0 5 8
       		2 9 7 2 1 1 0 7 9 9 9 9 1 4 6 2
	
	.text
main:

# Students may modify this "main" section temporarily for their testing.
# However, when evaluating your submission, all code from lines 1
# to 61 will be replaced by other testing code (i.e., we will only
# keep code from lines 62 onward). If your solution breaks because
# you have ignored this note, then a mark of zero for Part 1
# of the assignment is possible.
	la $a0, GEN_A
	addi $a1, $zero, 0
	addi $a2, $zero, 0
	addi $a3, $zero, 0x1
	jal set_16x16			# Test 1a
	
	la $a0, GEN_A
	addi $a1, $zero, 15
	addi $a2, $zero, 15
	addi $a3, $zero, 0x1
	jal set_16x16			# Test 1b
	
	la $a0, TEST_A_16x16
	addi $a1, $zero, 13
	addi $a2, $zero, 4
	jal get_16x16			# Test 1c; $v0 should be 6
	
	la $a0, GEN_B
	la $a1, TEST_A_16x16
	jal copy_16x16			# Memory in data area for GEN_B
					# be an exact copy of data area
					# for TEST_A_16x16
	
			
	addi $v0, $zero, 10
	syscall
	

# STUDENTS MAY MODIFY CODE BELOW
# vvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
	
	
	.data
	
# Available for any extra `.eqv` or data needed for your solution.
	
	.text



# set_16x16:
#	
# $a0 is 16x16 byte array
# $a1 is row (0 is topmost)
# $a2 is column (0 is leftmost)
# $a3 is the value to be stored (i.e., rightmost 8 bits)
#
# $t0: row length
# $t1: index
# $t2: counter
# 
# If $a1 or $a2 are outside bounds of array, then
# nothing happens.
set_16x16:
	li $t0, 0x10		
	bge $a1, $t0, out_of_range
	bge $a2, $t0, out_of_range
	bltz $a1, out_of_range
	bltz $a2, out_of_range
	
	li $t1, 0		
	sll $t1,$a1,4
	add $t1,$t1,$a2		

done_set:
	add $a0, $a0, $t1		# add to the address the index num
	sb $a3, 0($a0)
	sub $a0, $a0, $t1
	jr $ra
	
out_of_range:
	jr $ra
	
	
# get_16x16:
#
# $a0 is 16x16 byte array
# $a1 is row (0 is topmost)
# $a2 is column (0 is leftmost)
#
# $t0: row length
# $t1: index
# $t2: counter
# 
# If $a1 or $a2 are outside bounds of array, then
# the value of zero is returned
#
# $v0 holds the value of the byte at that array location
get_16x16:
	li $v0,0
	li $t0, 0x10			
	bge $a1, $t0, out_of_range_get
	bge $a2, $t0, out_of_range_get
	bltz $a1, out_of_range_get
	bltz $a2, out_of_range_get
	
	li $t1, 0			
	sll $t1,$a1,4	 		
	add $t1, $t1, $a2
	
done_get:
	add $a0, $a0, $t1		# add to the address the index num
	lb $v0, 0($a0) 
	sub $a0, $a0, $t1

out_of_range_get:
	jr $ra
	

# copy_16x16:
#
# $a0 is the destination 16x16 byte array
# $a1 is the source 16x16 byte array
# $t0: counter
# $t1: offset of destination
# $t2: offset of source
#
copy_16x16:
	li $t0, 256	
	li $t1, 0	
	li $t2, 0	
	
copy_loop:
	add $t1, $a0, $t0 
	add $t2, $a1, $t0 

	lb $t3, 0($t2)
	sb $t3, 0($t1)

	beq $t0, $zero, done_copy
	addi $t0, $t0, -1
	j copy_loop

done_copy:
	jr $ra

	
# ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
# STUDENTS MAY MODIFY CODE ABOVE
