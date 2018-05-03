.data
          .align 5
dataName: .asciiz "Joe"
          .align 5
          .asciiz "Jenny"
          .align 5
          .asciiz "Jill"
          .align 5
          .asciiz "John"
          .align 5
          .asciiz "Jeff"
          .align 5
          .asciiz "Joyce"
          .align 5
          .asciiz "Jerry"
          .align 5
          .asciiz "Janice"
          .align 5
          .asciiz "Jake"
          .align 5
          .asciiz "Jonna"
          .align 5
          .asciiz "Jack"
          .align 5
          .asciiz "Jocelyn"
          .align 5
          .asciiz "Jessie"
          .align 5
          .asciiz "Jess"
          .align 5
          .asciiz "Janet"
          .align 5
          .asciiz "Jane"
          .align 2  # addresses should start on a word boundary
dataAddr: .space 64 # 16 pointers to strings: 16*4 = 64
init_str: .asciiz "Initial array:\n"
end_str:  .asciiz "Sorted array:\n"
lbracket: .asciiz "[ "
rbracket: .asciiz "]\n"
space:    .asciiz " "
null_term:.asciiz ""

.text

main: 
	# const char * data[] = {"Joe", "Jenny", "Jill", "John", ...
	la $t0, dataName # t0 holds the address of current string
	la $t1, dataAddr # t1 holds the address of starting string pointer
	addu $t2, $t1, 64 # mark the end of the array
    arrayinit:
	sw $t0, ($t1) # initialize pointer for current string
	addu $t0, $t0, 32 # move to next string
	addu $t1, $t1, 4  # move to next string pointer
	bltu $t1, $t2, arrayinit # loop back if $t1 havn't reach $t2 (which is the end of the array)

	# printf("Initial array:\n")
	li $v0, 4
	la $a0, init_str
	syscall

	# print_array(data, size);
	la $a0 dataAddr
	li $a1, 64
	jal print_array # note that $a1 was set when entering main
	
	# quick_sort(data, size):
	la $a0 dataAddr
	li $a1, 64
	jal quick_sort

	# printf("Sorted Array:\n")
	li $v0, 4
	la $a0, end_str
	syscall
			
	# print_array(data, size);
	la $a0 dataAddr
	li $a1, 64
	jal print_array # note that $a1 was set when entering main
	
	
	# exit(0)
	li $v0, 10 # syscall code for exit
	li $a0, 0  # exit with status code 0 which means no error
	syscall



# int str_lt (const char *x, const char *y)
str_lt:
	# invariant: $a0 holds addr of x, $a1 holds addr of y
	# returns: 1 if x > y else 0 at $v0
	
	# int i = 0
	li $t0, 0
	# char terminator = '\0'
	lb $t5, null_term
	
	compare_char:	
	# char * a = x + i 
	addu $t1, $t0, $a0
	# char * b = y + i
	addu $t2, $t0, $a1
	
	# char char_a = *a
	lb $t3, ($t1)
	# char char_b = *b
	lb $t4, ($t2)
	
	# if ( char_a == terminator ) { goto end_of_x }
	beq $t3, $t5, end_of_x
	# if ( char_a == char_b) { i++, goto compare_char)
	beq $t3, $t4, compare_next_char
	# if (char_a >= char_b) { return 1; }
	bgeu $t3, $t4, x_greater_or_equal_to_y
	# if (char_a < char_b) { return 0; }
	blt $t3, $t4, x_less_than_y
	
    compare_next_char:
	addu $t0, $t0, 1
	j compare_char
	
    x_greater_or_equal_to_y:
	li $v0, 1
	jr $ra
    
    end_of_x:
    	# if (char_b == terminator) { x_greater_or_equal_to_y }
	beq $t4, $t5, x_greater_or_equal_to_y
	# else { x_less_than_y } // falling through 
	
    x_less_than_y:
    	li $v0, 0
    	jr $ra
		
# void swap_str_ptrs(const char **s1, const char **s2)	
swap_str_ptrs:
	# invariant: $a0 holds s1, $a1 holds s2, no return values
	# note that ** indicates ptr of ptr which the address of the pointer to the str
	
	# char *tmp1 = *s1;
	lw $t0, ($a0)
	# char *tmp2 = *s2;
	lw $t1, ($a1)
	
	# *s1 = *s2;
	sw $t1, ($a0)
	# *s2 = *s1;
	sw $t0, ($a1)
	# return;
	jr $ra
	
# void quick_sort(const char *a[], size_t len)
quick_sort:
	# if (len <= 1) { return; }
	bleu $a1, 4, quick_sort_return

    # Prologue 
	subu $sp, $sp, 32 # making space for CF
	sw $ra, 28($sp) # save return address on CF
	sw $fp, 24($sp) # save frame pointer in stack
	sw $s0, 20($sp) # save callee saved a
	sw $s1, 16($sp) # save callee saved len
	sw $s2, 12($sp) # save callee saved pivot
	addu $fp, $sp, 32 # make the frame pointer point to the base of the call frame
	
    # setup local vars 
    	# char * a = a;
	move $s0, $a0 # save argument a
	# int len = len;
	move $s1, $a1 # save argument len
	# int pivot = 0;
	li $s2, 0
	# int i = 0;
	li $s3, 0
	# last = len - 1;
	subu $s4, $a1, 4
	
    partition:
	# int is_greater = str_lt( a[i], a[last] )
	addu $t3, $s3, $s0
	lw $a0, ($t3)
	addu $t4, $s4, $s0
	lw $a1, ($t4)
	jal str_lt # side effect on $s2 and $s3?
	
	# if ( ! is_greater ) { goto i_move_right; }
	beqz $v0, just_increment_i
	
	# swarp_str_ptrs(&a[i], &a[pivot])
	addu $a0, $s3, $s0
	addu $a1, $s2, $s0
	jal swap_str_ptrs
			
    	# pivot++
    	addu $s2, $s2, 4
	
	
    just_increment_i:
	# i++;
	addu $s3, $s3, 4
	# if (i < last) { goto partition; }
	blt $s3, $s4, partition
	# else { ; } // falling through
	
	# swap_str_ptrs(&a[pivot], &a[len - 1]);
	addu $a0, $s2, $s0
	addu $a1, $s4, $s0
	jal swap_str_ptrs
	
	# quick_sort(a, pivot);
	move $a0, $s0
	move $a1, $s2
	jal quick_sort
	
	# quick_sort(a + pivot + 1, len - pivot - 1);
	addu $a0, $s0, $s2
	addu $a0, $a0, 4
	subu $a1, $s1, $s2
	subu $a1, $a1, 4
	jal quick_sort
	
	
 	#Function Epilogue: Tear down the call frame and restore registers.
	lw $ra, 28($sp)
	lw $fp, 24($sp)
	lw $s0, 20($sp)
	lw $s1, 16($sp)
	lw $s2, 12($sp)
	addu $sp, $sp, 32
    quick_sort_return:
	jr $ra # jump to the saved return address


# void print_array(const char * a[], const int size)
print_array:
	# invariant: $a0 has the array addr, $a1 has the size
	move $s0, $a0 # save argument array addr 
	move $s1, $a1 # save argument
	move $s2, $ra # save $ra from syscall interruption
	# printf("[");
	li $v0, 4     # syscall code for print string 
	la $a0, lbracket
	syscall
	# int i = 0
	li $t0, 0
   print_str:
	# printf("%s ", a[i])
	addu $t1, $t0, $s0 # $t1 is a[i]
	lw $a0, ($t1) # print current string
	syscall
	la $a0, space # print a white space
	syscall
	# i++
	addu $t0, $t0, 4 # move string poniter to next string
	# i < size	
	bltu $t0, $s1, print_str # loop back if $t0 havn't reach $s1 (second argument `size`)
	# printf("]\n")
	la $a0, rbracket # print a white bracket
	syscall
	move $ra, $s2 # restore $ra
	jr $ra # return out of the function print_array
