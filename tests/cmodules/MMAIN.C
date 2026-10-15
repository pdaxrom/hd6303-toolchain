extern int shared;
extern char bytes[];
int triple();
int fact();
int even();
static int local;
static int check(ok)
int ok;
{
 if (ok==0) { puts("C MODULE FAIL"); exit(1); }
}
int main(argc,argv)
int argc;
char *argv[];
{
 int *fp;
 check(argc>0); check(argv!=0);
 check(shared==0); check(bytes[0]==0); check(local==0);
 local=11; shared=9; bytes[2]=42;
 check(triple(7)==21); fp=triple; check(fp(5)==15);
 check(shared==21); check(bytes[2]==42); check(local==11);
 check(fact(6)==720); check(even(10)==1); check(even(9)==0);
 puts("C MODULE PASS");
 return 0;
}
