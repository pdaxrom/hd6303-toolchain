/* UniAR: original UniLIB containers, opaque and byte-identical OBJ members. */
char libname[32], archive[32], objname[32], extract[32];
char names[2048], files[8192], requests[2048];
char active[256], source[256], reqhit[256], arhead[32], aname[8];
int ah[256], al[256], sizes[256], nitems,nreq,action;
char *prog;
#include TOOLIO.C

main(argc,argv)
int argc,*argv[];
{
 int i,f,out,count,src,j;
 char *p;
 prog="UniAR";
 if (argc<3) die("uniar r/a/d/t/x library [objects] [@list]");
 p=argv[1]; if (*p=='-') p++;
 action=*p++;
 if (*p) die("Expected one archive operation");
 if (action!='r' & action!='a' & action!='d' & action!='t' &
     action!='x') die("Unknown archive operation");
 defaultx(libname,argv[2],".LIB");
 if (exists(libname)) loadlib();
 else if (action!='r' & action!='a') die("Library does not exist");
 i=3;
 while (i<argc) {
  p=argv[i++];
  if (*p=='@') argsfile(p+1);
  else addinput(p);
 }
 if (action=='t' | action=='x') {
  i=0;
  while (i<nitems) {
   if (wanted(names+i*8)) {
    if (action=='t') {
     j=0; while (j<8) putchar(names[i*8+j++]);
     putchar(' '); numout(sizes[i]); puts(" bytes");
    } else unpack(i);
   }
   i++;
  }
  missing(); exit(0);
 }
 if (action=='d') {
  i=0; while (i<nitems) {
   if (nreq!=0 & wanted(names+i*8)) active[i]=0;
   i++;
  }
  missing();
 }
 count=0; i=0;
 while (i<nitems) { if (active[i++]) count++; }
 tmpname(archive,libname,'A'); cleanup(archive);
 out=ioopen(archive,"w+");
 i=0; while (i<32) arhead[i++]=0;
 set16(arhead,2992); set16(arhead+4,count);
 wrblk(out,arhead,32);
 i=0;
 while (i<nitems) {
  if (active[i]) {
   wrblk(out,names+i*8,8); ww(out,0); ww(out,sizes[i]);
   if (source[i]) {
    src=ioopen(files+i*32,"r");
   } else {
    src=ioopen(libname,"r"); seekabs(src,ah[i],al[i]);
   }
   copyblk(src,out,sizes[i]); ioclose(src);
  }
  i++;
 }
 ioclose(out); publish(archive,libname);
 puts(libname); numout(count); puts(" modules");
 exit(0);
}

same8(a,b)
char *a,*b;
{
 int i;
 i=0; while (i<8) {
  if (toupper(a[i])!=toupper(b[i])) return 0;
  i++;
 }
 return 1;
}

checkobj(f,hi,lo,len)
int f,hi,lo,len;
{
 int off,code,n,ver;
 if (ult(len,35)) die("OBJ shorter than its header");
 seekabs(f,hi,lo); rdblk(f,arhead,32);
 if (be16(arhead)!=23205) die("Invalid OBJ magic");
 ver=be16(arhead+2);
 if (ver!=0 & ver!=1) die("Unsupported OBJ version");
 if (be16(arhead+10)) die("OBJ offset exceeds 65535");
 off=be16(arhead+12); code=be16(arhead+8);
 if (ult(off,32)) die("Invalid OBJ code offset");
 n=add16(off,code);
 if (ult(len,add16(n,3))) die("Truncated OBJ");
 posadd(hi,lo,n); seekabs(f,poshi,poslo); n=rw(f);
 if (n<0 | n>13100) die("Invalid OBJ relocation count");
 if (add16(add16(add16(off,code),3),n*5)!=len)
  die("OBJ length does not match header");
 posadd(hi,lo,len-1); seekabs(f,poshi,poslo);
 if (rb(f)) die("Invalid OBJ end marker");
}

loadlib()
{
 int f,i,j,n,hi,lo,next,nh,nl,bytes;
 f=ioopen(libname,"r"); getsize(f); nh=poshi; nl=poslo;
 rdblk(f,arhead,32);
 if (be16(arhead)!=2992) die("Invalid LIB magic");
 n=be16(arhead+4);
 if (n<0 | n>256) die("Invalid LIB member count");
 hi=0; lo=32; i=0;
 while (i<n) {
  seekabs(f,hi,lo); rdblk(f,aname,8);
  if (rw(f)) die("OBJ member exceeds 65535 bytes");
  bytes=rw(f); getpos(f); hi=poshi; lo=poslo;
  posadd(hi,lo,bytes);
  if (ult(nh,poshi) | (nh==poshi & ult(nl,poslo)))
   die("Truncated library");
  ah[i]=hi; al[i]=lo; sizes[i]=bytes; active[i]=1;
  j=0; while (j<i) {
   if (same8(names+j*8,aname)) die("Duplicate library member");
   j++;
  }
  j=0; while (j<8) { names[i*8+j]=aname[j]; j++; }
  checkobj(f,hi,lo,bytes);
  posadd(hi,lo,bytes); hi=poshi; lo=poslo; i++;
 }
 if (hi!=nh | lo!=nl) die("Trailing library bytes");
 nitems=n; ioclose(f);
}

addinput(path)
char *path;
{
 int i,j,f,len;
 membernm(path,aname);
 if (action=='d' | action=='t' | action=='x') {
  if (nreq==256) die("More than 256 member requests");
  j=0; while (j<8) { requests[nreq*8+j]=aname[j]; j++; }
  nreq++; return;
 }
 defaultx(objname,path,".OBJ");
 f=ioopen(objname,"r"); getsize(f);
 if (poshi) die("OBJ exceeds 65535 bytes");
 len=poslo; checkobj(f,0,0,len); ioclose(f);
 i=0;
 while (i<nitems) {
  if (same8(names+i*8,aname)) break;
  i++;
 }
 if (i<nitems & action=='a') die("Member already exists");
 if (i==nitems) {
  if (nitems==256) die("More than 256 library members");
  nitems++;
 }
 j=0; while (j<8) { names[i*8+j]=aname[j]; j++; }
 pathcpy(files+i*32,objname);
 source[i]=1; active[i]=1; sizes[i]=len;
}

wanted(name)
char *name;
{
 int i,found;
 if (nreq==0) return 1;
 i=0; found=0;
 while (i<nreq) {
  if (same8(name,requests+i*8)) { reqhit[i]=1; found=1; }
  i++;
 }
 return found;
}

missing()
{
 int i;
 i=0; while (i<nreq) {
  if (reqhit[i]==0) die("Requested member does not exist");
  i++;
 }
}

unpack(i)
int i;
{
 int f,out,j;
 j=0;
 while (j<8 & names[i*8+j]!=' ') {
  extract[j]=names[i*8+j]; j++;
 }
 extract[j]=0; strcat(extract,".OBJ");
 tmpname(archive,extract,'X'); cleanup(archive);
 f=ioopen(libname,"r"); seekabs(f,ah[i],al[i]);
 out=ioopen(archive,"w+"); copyblk(f,out,sizes[i]);
 ioclose(out); ioclose(f); publish(archive,extract); puts(extract);
}
