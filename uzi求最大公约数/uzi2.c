#include "stdio.h"  
#include "string.h"  
int main()  
{  
    int testCase;  
    char line[20][256];  
    int i=0;  
    int min=256;  
    int max=0;  
    char *maxStr ;  
    int maxLength;  
    int length,minLength;  
    int j=0,count=0;  
    int m,n,k;  
/*  freopen("in2.txt","r",stdin);*/  
    maxStr =(char *)malloc(sizeof(char)*256);  
    scanf("%d",&testCase);  
    getchar();  
    while(testCase!=0)  
    {  
        for(i=0;i<testCase;i++)  
        {  
            gets(line[i]);  
            length = strlen(line[i]);  
            if(length<min)  
            {  
                j=i;  
                min = length;  
            }  
        }  
        max =0;  
        for(m=0;m<min;m++)  
        {  
            for(n=m;n<min;n++)  
            {  
                count =0;  
                for(k=0;k<testCase;k++)  
                {  
                    if(k==j)  
                    continue;  
                    if((length=strMatch(line[k],line[j],m,n))!=0)  
                    {  
                        ++count;  
  
                    }  
                    else  
                    {                     
                        break;  
                    }  
                    if(count==testCase-1)  
                    {  
                        if(length>max)  
                        {  
                            max=length;  
                            for(i=m;i<=n;i++)  
                            {  
                                maxStr[i-m]=line[j][i];  
                            }  
                            maxStr[i-m]='\0';  
                            /*printf("%d : %s\n",length,maxStr);*/  
                        }  
                      
                    }  
                      
                }  
                  
            }  
        }  
        if(strlen(maxStr)==0)  
        {  
            printf("\n");  
        }  
        else  
        {  
        /*  length = strlen(maxStr); 
            for(i=0;i<length;i++) 
            { 
                printf("%2c",maxStr[i]); 
            } 
            putchar('\n');*/  
            printf("%s\n",maxStr);  
        }  
        scanf("%d",&testCase);  
        getchar();  
          
    }  
/*  fclose(stdin);*/  
    return 0;  
}  