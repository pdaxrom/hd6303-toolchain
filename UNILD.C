/* UniLD: streaming UniCROSS OBJ v0/v1 and UniLIB linker, Small-C. */
#define MAXMOD 256
#define MAXSYM 512
#define SYMSIZE 21
char paths[1024], members[2048], globals[10752];
char mf[256], mpub[256], mext[256], selected[256], walked[256];
int mh[256], ml[256], mlen[256], mc[256], md[256], mo[256];
int me[256], mg[256], cb[256], db[256], cache[255];
int nfiles,nmods,nglobal,rootmod,cmdmode,pageflag;
int textlen,datalen,changed,rtype,rvalue,roff,relcount;
char outfile[32], codefile[32], finaltmp[32], inputnm[32];
char namebuf[16], member[8], header[32];
char *prog;
#include TOOLIO.C

main(argc,argv)
int argc,*argv[];
{
 int i,f,out,code,n,entry;
 char *p;
 prog="UniLD"; rootmod=-1;
 i=1;
 while (i<argc) {
  p=argv[i++];
  if (strcmp(p,"-o")==0) {
   if (i==argc) die("Missing output path");
   pathcpy(outfile,argv[i++]);
  } else if (*p=='/' | *p=='-') {
   p++;
   if (*p==0) die("Empty option");
   while (*p) {
    iopos[0]=tolower(*p++);
    if (iopos[0]=='c') cmdmode=1;
    else if (iopos[0]=='p') pageflag=1;
    else die("Unknown linker option");
   }
  } else if (*p=='@') argsfile(p+1);
  else addinput(p);
 }
 if (nmods==0) die("unild [/c] [/p] [-o output] objects/libs");
 if (rootmod<0) die("No entry point");
 selected[rootmod]=1;
 changed=1;
 while (changed) {
  changed=0; i=0;
  while (i<nmods) {
   if ((selected[i]&1)!=0 & walked[i]==0) walkmod(i);
   i++;
  }
 }
 duplicates(); layout();
 if (cmdmode & add16(cb[rootmod],me[rootmod])!=0)
  die("CMD entry point must be at its beginning");
 if (*outfile==0) {
  p=members+rootmod*8; i=0;
  while (i<8 & p[i]!=' ') { outfile[i]=p[i]; i++; }
  outfile[i]=0;
  if (cmdmode) strcat(outfile,".CMD");
  else strcat(outfile,".PGM");
 }
 tmpname(codefile,outfile,'C'); cleanup(codefile);
 code=ioopen(codefile,"w+");
 n=0; i=0;
 while (i<nmods) {
  if (selected[i]&1) {
   zeroblk(code,cb[i]-n);
   f=ioopen(paths+mf[i]*32,"r"); seekmod(f,i,mo[i]);
   copyblk(f,code,mc[i]); ioclose(f); n=cb[i]+mc[i];
  }
  i++;
 }
 zeroblk(code,textlen-n);
 tmpname(finaltmp,outfile,'L'); cleanup(finaltmp);
 out=ioopen(finaltmp,"w+");
 if (cmdmode==0) zeroblk(out,16);
 i=0;
 while (i<nmods) {
  if (selected[i]&1) patchmod(i,code,out);
  i++;
 }
 seekabs(code,0,0); copyblk(code,out,textlen);
 ioclose(code); unlink(codefile);
 if (cmdmode==0) {
  seekabs(out,0,0); ww(out,42330); ww(out,relcount);
  ww(out,add16(16,relcount*2)); ww(out,textlen);
  entry=add16(cb[rootmod],me[rootmod]);
  ww(out,entry); ww(out,datalen); ww(out,0); ww(out,0);
 }
 ioclose(out); publish(finaltmp,outfile);
 puts(outfile); numout(textlen); puts(" bytes code");
 numout(datalen); puts(" bytes BSS");
 exit(0);
}

gs(i)
int i;
{
 return globals+i*21;
}

readname(fd,p)
int fd;
char *p;
{
 int i,c,ended;
 ended=0; i=0;
 while (i<16) {
  c=rb(fd);
  if (c==0) ended=1;
  if (ended) p[i]=0;
  else p[i]=toupper(c);
  i++;
 }
 if (ended==0 | *p==0) die("Invalid object symbol name");
}

seekmod(fd,m,offset)
int fd,m,offset;
{
 posadd(mh[m],ml[m],offset); seekabs(fd,poshi,poslo);
}

addinput(path)
char *path;
{
 int f,i,hi,lo,bytes,nh,nl,count;
 defaultx(inputnm,path,".OBJ");
 if (exists(inputnm)==0) extension(inputnm,path,".LIB");
 i=0;
 while (i<nfiles) {
  if (strcmp(paths+i*32,inputnm)==0) return;
  i++;
 }
 if (nfiles==32) die("More than 32 input files");
 pathcpy(paths+nfiles*32,inputnm);
 f=ioopen(inputnm,"r"); getsize(f); nh=poshi; nl=poslo;
 rdblk(f,header,32);
 if (be16(header)==23205) {
  if (nh) die("OBJ exceeds 65535 bytes");
  membernm(inputnm,member);
  addmod(f,nfiles,0,0,nl,member,0);
 } else if (be16(header)==2992) {
  count=be16(header+4);
  if (count<0 | count>256) die("Invalid library count");
  hi=0; lo=32; i=0;
  while (i<count) {
   seekabs(f,hi,lo); rdblk(f,member,8);
   if (rw(f)!=0) die("Library OBJ exceeds 65535 bytes");
   bytes=rw(f); getpos(f); hi=poshi; lo=poslo;
   posadd(hi,lo,bytes);
   if (ult(nh,poshi) | (nh==poshi & ult(nl,poslo)))
    die("Truncated library member");
   addmod(f,nfiles,hi,lo,bytes,member,1);
   posadd(hi,lo,bytes); hi=poshi; lo=poslo; i++;
  }
  if (hi!=nh | lo!=nl) die("Trailing library bytes");
 } else die("Invalid OBJ or LIB magic");
 ioclose(f); nfiles++;
}

addmod(fd,file,hi,lo,bytes,name,islib)
int fd,file,hi,lo,bytes;
char *name;
int islib;
{
 int m,i,np,ne,ver,off,val,flags;
 char *s;
 if (nmods==256) die("More than 256 object modules");
 m=nmods++; mh[m]=hi; ml[m]=lo; mf[m]=file; mlen[m]=bytes;
 i=0; while (i<8) { members[m*8+i]=name[i]; i++; }
 seekabs(fd,hi,lo); rdblk(fd,header,32);
 if (be16(header)!=23205) die("Invalid object magic");
 ver=be16(header+2);
 if (ver!=0 & ver!=1) die("Unsupported object version");
 np=be16(header+4); ne=be16(header+6);
 if (np<0 | np>255 | ne<0 | ne>255) die("Invalid symbol count");
 mc[m]=be16(header+8); off=be16(header+12); mo[m]=off;
 if (be16(header+10)) die("Object code offset too large");
 if (ult(off,32+(np+ne)*20)) die("Overlapping object tables");
 if (ult(bytes,add16(add16(off,mc[m]),3)))
  die("Truncated object code");
 md[m]=0; if (ver) md[m]=be16(header+16);
 me[m]=be16(header+14); mpub[m]=np; mext[m]=ne; mg[m]=nglobal;
 if (ver==0 & islib) me[m]=-1;
 if (ver==0 & m!=0) me[m]=-1;
 if (me[m]!=-1) {
  if (ult(me[m],mc[m])==0) die("Entry point outside code");
  if (rootmod>=0) die("Multiple entry points");
  rootmod=m;
 }
 i=0;
 while (i<np) {
  if (nglobal==512) die("More than 512 public symbols");
  s=gs(nglobal++); readname(fd,s);
  val=rw(fd); flags=rw(fd);
  set16(s+16,val); set16(s+18,m); s[20]=0;
  if (flags&32768) s[20]=1;
  else if (md[m]!=0 & ult(val,mc[m])==0) s[20]=2;
  i++;
 }
 /* Validate every relocation stream, even in unselected archive members. */
 seekmod(fd,m,add16(off,mc[m])); np=rw(fd);
 if (np<0 | np>13100) die("Invalid relocation count");
 ne=add16(add16(add16(off,mc[m]),3),np*5);
 if (ne!=bytes) die("Invalid object length");
 off=0; i=0;
 while (i<np) {
  readrel(fd,m);
  if (i!=0 & ult(roff,off)) die("Unsorted object relocations");
  off=roff; i++;
 }
 if (rb(fd)) die("Invalid object end marker");
}

readrel(fd,m)
int fd,m;
{
 int width;
 rtype=rb(fd); rvalue=rw(fd); roff=rw(fd);
 width=rtype&3;
 if (width==0 | (rtype&124)!=0) die("Invalid relocation type");
 if (ult(roff,mc[m])==0) die("Relocation outside code");
 if (width==3 & ult(roff,mc[m]-1)==0)
  die("Word relocation crosses code boundary");
 if ((rtype&128)!=0 & ult(rvalue,mext[m]&255)==0)
  die("External index outside table");
}

findsym(name)
char *name;
{
 int i,first,m;
 char *s;
 first=-1; i=0;
 while (i<nglobal) {
  s=gs(i);
  if (strcmp(name,s)==0) {
   m=be16(s+18);
   if (selected[m]) return i;
   if (first<0) first=i;
  }
  i++;
 }
 return first;
}

extmap(fd,m)
int fd,m;
{
 int i;
 seekmod(fd,m,32+(mpub[m]&255)*20);
 i=0;
 while (i<(mext[m]&255)) {
  readname(fd,namebuf); cache[i++]=findsym(namebuf);
  rw(fd); rw(fd);
 }
}

need(m,kind)
int m,kind;
{
 if ((selected[m]&kind)==0) {
  selected[m]=selected[m]|kind; changed=1;
 }
}

target(m)
int m;
{
 int index;
 char *s;
 if (rtype&128) {
  index=cache[rvalue];
  if (index<0) {
   puts(paths+mf[m]*32); die("Unresolved external symbol");
  }
  s=gs(index); m=be16(s+18); rvalue=be16(s+16);
  if (s[20]==1) { need(m,4); return -1; }
 }
 if (md[m]!=0 & ult(rvalue,mc[m])==0) need(m,2);
 else need(m,1);
 return m;
}

walkmod(m)
int m;
{
 int f,i,n;
 walked[m]=1; f=ioopen(paths+mf[m]*32,"r"); extmap(f,m);
 seekmod(f,m,add16(mo[m],mc[m])); n=rw(f); i=0;
 while (i<n) { readrel(f,m); target(m); i++; }
 ioclose(f);
}

duplicates()
{
 int i,j,m;
 char *s,*t;
 i=0;
 while (i<nglobal) {
  s=gs(i); m=be16(s+18);
  if (selected[m]) {
   j=i+1;
   while (j<nglobal) {
    t=gs(j); m=be16(t+18);
    if (selected[m]!=0 & strcmp(s,t)==0) {
     puts(s); die("Duplicate public symbol");
    }
    j++;
   }
  }
  i++;
 }
}

aligned(v)
int v;
{
 if (pageflag & (v&255)!=0) v=add16(v,256-(v&255));
 return v;
}

layout()
{
 int i;
 i=0;
 while (i<nmods) {
  if (selected[i]&1) {
   textlen=aligned(textlen); cb[i]=textlen;
   textlen=aligned(add16(textlen,mc[i]));
  }
  if (selected[i]&2) {
   datalen=aligned(datalen); db[i]=datalen;
   datalen=aligned(add16(datalen,md[i]));
  }
  i++;
 }
 add16(textlen,datalen);
}

patchmod(m,code,out)
int m,code,out;
{
 int f,i,n,t,v,off,width,relative;
 f=ioopen(paths+mf[m]*32,"r"); extmap(f,m);
 seekmod(f,m,add16(mo[m],mc[m])); n=rw(f); i=0;
 while (i<n) {
  readrel(f,m); off=add16(cb[m],roff); width=rtype&3;
  t=target(m); relative=t>=0; v=rvalue;
  if (relative) {
   if (md[t]!=0 & ult(v,mc[t])==0)
    v=add16(add16(textlen,db[t]),v-mc[t]);
   else v=add16(cb[t],v);
   if (cmdmode) v=add16(256,v);
  }
  seekabs(code,0,off);
  if (width==1) wb(code,v);
  else {
   wb(code,(v>>8)&255);
   if (width==3) wb(code,v);
  }
  if (cmdmode==0 & relative!=0 & width!=1) {
   if (ult(off,textlen-1)==0)
    die("PGM high-byte relocation needs a following byte");
   if (relcount==32759) die("PGM relocation table too large");
   ww(out,off); relcount++;
  }
  i++;
 }
 ioclose(f);
}
