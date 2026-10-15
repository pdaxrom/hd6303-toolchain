int mixed;
extern int nativefn();
extern int inlinen();
extern int backcall();
extern int badcall();
int addone(n)
int n;
{
 return n+1;
}
int reenter(n)
int n;
{
 if (n==0) return 1;
 return backcall(reenter,n-1)+n;
}
int cadd(a,b)
int a,b;
{
 return a+b;
}
int retneg()
{
 return -1234;
}
int noargs()
{
 return 0x1234;
}
int explode(n)
int n;
{
 return 1/n;
}
int main()
{
 if (inlinen(11)!=11) return 9;
 if (mixed!=0) return 7;
 if (nativefn(7,8)!=15) { puts("NATIVE FAIL"); return 1; }
 if (nativefn(-123,23)!=-100) return 2;
 if (mixed!=2) return 8;
 if (backcall(addone,41)!=42) { puts("CALLBACK FAIL"); return 3; }
 if (backcall(reenter,6)!=22) { puts("REENTER FAIL"); return 4; }
 if (badcall(addone,41)!=255) { puts("BAD CALL FAIL"); return 5; }
 if (backcall(addone,99)!=100) return 6;
 puts("C MIXED PASS");
 return 0;
}
