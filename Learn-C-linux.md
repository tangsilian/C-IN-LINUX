##run the *.c page in linux 
bulid  the test.c
int mian(){
	
	return 0;
}

gcc test.c -o test
./test
/**use the -g to add the debug **/
gcc test.c -g -o test
/**the see the x86....**/
objdump -S -d ./test 


##makefile
--first make the main progarm
add.c
 #include "test.h"  
 #include <stdio.h>  
  
int add(int a, int b)  
{  
    return a + b;  
}  
  
int main()  
{  
    printf(" 2 + 3 = %d\n", add(2, 3));  
    printf(" 2 - 3 = %d\n", sub(2, 3));  
    return 1;  
}  

--then make the sub.c 
 #include "test.h"  
  
int sub(int a, int b)  
{  
    return a - b;  
}  

the last make the program head tesh.h
 #ifndef _TEST_H  
 #define _TEST_H  
  
int add(int a, int b);  
int sub(int a, int b);  
 #endif 


so  the point coming make the makefile page 
/**  make the .c to *.o  **/
test: add.o sub.o  
    gcc -o test add.o sub.o  
/**add the head to .c **/  
add.o: add.c test.h  
    gcc -c add.c  
 /**add the head to .c **/  
sub.o: sub.c test.h  
    gcc -c sub.c      
 /**cleam all **/     
clean:  
    rm -rf test  
    rm -rf *.o  

how to use the dgb  ?
see this
http://blog.csdn.net/feixiaoxing/article/details/7199643
