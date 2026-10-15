extern int even();
int odd(n)
int n;
{
 if (n==0) return 0;
 return even(n-1);
}
