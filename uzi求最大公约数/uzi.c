#include "stdio.h"
#include "string.h"
#include "stdlib.h"
#include "omg.h"
void main(){
	char str1[UZI];
	char str2[UZI];
	char str3[UZI];
	gets(str1);
	gets(str2);
	getCommon(str1,str2,str3);
	printf("%s\n",str3);
}
