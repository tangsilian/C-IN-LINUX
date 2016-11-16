#include "stdio.h"  
#include "string.h"  
int strMatch(char *line,char *lineMin,int s,int e)  
{  
    int length = strlen(line);  
    int i=0,j=0;  
    int flag =0;  
    int start =s;  
    int matchLength =0;  
    for(i=0;i<length;i++)  
    {  
        if(line[i]==lineMin[s])  
        {  
            start=s+1;  
            for(j=i+1;(j-i<=e-s)&&j<length;j++)  
            {  
                if(line[j]!=lineMin[start])  
                {  
                    break;  
                }  
                start++;  
            }             
        }  
        if(start == e+1)  
        {  
            matchLength=e-s+1;  
            break;  
        }  
    }  
      
    return matchLength;  
}  