int shared;
char bytes[8];
static int local;
extern int odd();
int triple(n)
int n;
{
 local=local+1; shared=shared+n;
 return n*3;
}
int fact(n)
int n;
{
 if (n==0) return 1;
 return n*fact(n-1);
}
int even(n)
int n;
{
 if (n==0) return 1;
 return odd(n-1);
}
