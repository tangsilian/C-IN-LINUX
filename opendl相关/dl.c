int add(int a,int b)
{
    return (a + b);
}

int sub(int a, int b)
{
    return (a - b);
}

int mul(int a, int b)
{
    return (a * b);
}

int div(int a, int b)
{
    return (a / b);
}

//ʹ��gcc -fPIC -shared dl.c -o libdl.so 