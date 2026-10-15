/* Shared Small-C file primitives. All persistent offsets are two words. */
int livefd[8], cleanfn[4], nclean;
int iopos[2], poshi, poslo;
char iobuf[512], argline[128], renline[80];
char backup[32];

die(msg)
char *msg;
{
 int i;
 puts(prog); puts(msg);
 i=0; while (i<8) {
  if (livefd[i]) fclose(livefd[i]);
  livefd[i++]=0;
 }
 i=0; while (i<nclean) unlink(cleanfn[i++]);
 exit(1);
}

ult(a,b)
int a,b;
{
 return (a^32768)<(b^32768);
}

add16(a,b)
int a,b;
{
 int n;
 n=a+b;
 if (ult(n,a)) die("16-bit size overflow");
 return n;
}

be16(p)
char *p;
{
 return ((p[0]&255)<<8)|(p[1]&255);
}

set16(p,v)
char *p;
int v;
{
 p[0]=(v>>8)&255; p[1]=v&255;
}

ioopen(name,mode)
char *name,*mode;
{
 int fd,i;
 fd=fopen(name,mode);
 if (fd==0) { puts(name); die("Cannot open file"); }
 i=0; while (i<8) {
  if (livefd[i]==0) { livefd[i]=fd; return fd; }
  i++;
 }
 fclose(fd); die("Too many simultaneous streams");
}

ioclose(fd)
int fd;
{
 int i;
 i=0; while (i<8) {
  if (livefd[i]==fd) livefd[i]=0;
  i++;
 }
 if (fclose(fd)) die("File close error");
}

exists(name)
char *name;
{
 int f;
 f=fopen(name,"r");
 if (f==0) return 0;
 fclose(f); return 1;
}

rdblk(fd,p,n)
int fd;
char *p;
int n;
{
 if (n) {
  if (fread(p,1,n,fd)!=n) die("Short read");
 }
}

wrblk(fd,p,n)
int fd;
char *p;
int n;
{
 if (n) {
  if (fwrite(p,1,n,fd)!=n) die("Write error");
 }
}

rb(fd)
int fd;
{
 int c;
 c=getc(fd);
 if (c<0) die("Unexpected end of file");
 return c;
}

rw(fd)
int fd;
{
 int hi;
 hi=rb(fd); return (hi<<8)|rb(fd);
}

wb(fd,v)
int fd,v;
{
 if (putc(v&255,fd)==-1) die("Write error");
}

ww(fd,v)
int fd,v;
{
 wb(fd,(v>>8)&255); wb(fd,v);
}

seekabs(fd,hi,lo)
int fd,hi,lo;
{
 iopos[0]=hi; iopos[1]=lo;
 if (fseek(fd,iopos,0)) die("Seek error");
}

getpos(fd)
int fd;
{
 ftell(fd,iopos);
 poshi=iopos[0]; poslo=iopos[1];
 if (poshi==-1) die("File position error");
}

getsize(fd)
int fd;
{
 iopos[0]=0; iopos[1]=0;
 if (fseek(fd,iopos,2)) die("File size error");
 getpos(fd); seekabs(fd,0,0);
}

posadd(hi,lo,n)
int hi,lo,n;
{
 poslo=lo+n; poshi=hi;
 if (ult(poslo,lo)) poshi++;
 if (poshi<0) die("File too large");
}

copyblk(src,dst,n)
int src,dst,n;
{
 int k;
 while (n) {
  k=512; if (ult(n,k)) k=n;
  rdblk(src,iobuf,k); wrblk(dst,iobuf,k); n=n-k;
 }
}

zeroblk(dst,n)
int dst,n;
{
 int i,k;
 i=0; while (i<512) iobuf[i++]=0;
 while (n) {
  k=512; if (ult(n,k)) k=n;
  wrblk(dst,iobuf,k); n=n-k;
 }
}

pathcpy(dst,src)
char *dst,*src;
{
 if (strlen(src)>31) die("Path exceeds 31 characters");
 strcpy(dst,src);
}

extension(dst,src,ext)
char *dst,*src,*ext;
{
 char *p,*dot;
 pathcpy(dst,src); p=dst; dot=0;
 while (*p) {
  if (*p=='.') dot=p;
  if (*p=='/' | *p==92 | *p==':') dot=0;
  p++;
 }
 if (dot) *dot=0;
 if (strlen(dst)+strlen(ext)>31) die("Path too long");
 strcat(dst,ext);
}

defaultx(dst,src,ext)
char *dst,*src,*ext;
{
 char *p;
 int dot;
 pathcpy(dst,src); p=dst; dot=0;
 while (*p) {
  if (*p=='.') dot=1;
  if (*p=='/' | *p==92 | *p==':') dot=0;
  p++;
 }
 if (dot==0) {
  if (strlen(dst)+strlen(ext)>31) die("Path too long");
  strcat(dst,ext);
 }
}

tmpname(dst,src,letter)
char *dst,*src;
int letter;
{
 int n,len;
 extension(dst,src,".T00"); len=strlen(dst);
 dst[len-3]=letter; n=0;
 while (n<100) {
  dst[len-2]='0'+n/10; dst[len-1]='0'+n%10;
  if (exists(dst)==0) return;
  n++;
 }
 die("No unused temporary name");
}

cleanup(name)
char *name;
{
 if (nclean==4) die("Too many temporary files");
 cleanfn[nclean++]=name;
}

rename_(from,to)
char *from,*to;
{
 strcpy(renline,"REN "); strcat(renline,from);
 strcat(renline," "); strcat(renline,to);
 return system(renline);
}

publish(tmp,dest)
char *tmp,*dest;
{
 int old;
 old=exists(dest);
 if (old) {
  tmpname(backup,dest,'B');
  if (rename_(dest,backup)) die("Cannot retain old output");
 }
 if (rename_(tmp,dest)) {
  if (old) {
   if (rename_(backup,dest)) {
    puts(backup); die("Old output retained under backup name");
   }
  }
  die("Cannot publish output");
 }
 if (old) unlink(backup);
 nclean=0;
}

argsfile(name)
char *name;
{
 int f;
 char *p,*start;
 f=ioopen(name,"r");
 while (fgets(argline,128,f)) {
  p=argline;
  while (*p) {
   while (*p==' ' | *p==9 | *p==10 | *p==13) p++;
   if (*p==0 | *p==';' | *p=='#') break;
   start=p;
   while (*p!=0 & *p!=' ' & *p!=9 & *p!=10 & *p!=13) p++;
   if (*p) *p++=0;
   if (*start=='@') die("Nested response file");
   addinput(start);
  }
 }
 if (feof(f)==0) die("Response file read error");
 ioclose(f);
}

membernm(path,name)
char *path,*name;
{
 char *p;
 int n;
 p=path;
 while (*p) {
  if (*p=='/' | *p==92 | *p==':') path=p+1;
  p++;
 }
 n=0;
 while (*path!=0 & *path!='.') {
  if (n==8) die("Library member name exceeds eight characters");
  name[n++]=toupper(*path++);
 }
 if (n==0) die("Empty member name");
 while (n<8) name[n++]=' ';
}

numout(v)
int v;
{
 char b[6];
 int n,d;
 n=0;
 while (1) {
  d=v&1; v=(v>>1)&32767;
  b[n++]='0'+(v%5)*2+d; v=v/5;
  if (v==0) break;
 }
 while (n) putchar(b[--n]);
}
