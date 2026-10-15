/* UniCROSS v1 object output; code and relocation streams stay on disk. */
labelkind()
{
 if (bintype == BIN_CMD) return 0;
 if (nsect) {
  if (sectkind == 0) return 0;
  return SYMBLBL | SYMBBSS;
 }
 return SYMBLBL;
}
typeflag()
{
 if (flgrel == 3) { error("External EQU expression"); return 0; }
 if (flgrel == 2) return SYMBLBL | SYMBBSS;
 if (flgrel) return SYMBLBL;
 return 0;
}
exprbin(a,t,e,b,op)
int a,t,e,b,op;
{
 int u;
 u=flgrel;
 if (t==3 | u==3) error("External expression not allowed");
 if (op=='+') {
  if (t!=0 & u!=0) error("Illegal relative addition");
  if (t) { flgrel=t; flgext=e; }
  return a+b;
 }
 if (op=='-') {
  if (u) {
   if (t != u) error("Different expression sections");
   flgrel=0;
  } else { flgrel=t; flgext=e; }
  return a-b;
 }
 if (t | u) error("Expected absolute expression");
 flgrel=0;
 if (op=='*') return a*b;
 if (op=='/') return udiv16(a,b,0);
 if (op=='%') return udiv16(a,b,1);
 if (op=='|') return a|b;
 if (op=='^') return a^b;
 return a&b;
}
rawfind(name)
char *name;
{
 char *p;
 p=hashtab[hash(name)];
 while (p) {
  if (tagcmp(p,name)==0) return p;
  p=mgetw(p+NEXTPTR);
 }
 return 0;
}
public_()
{
 char *p;
 if (bintype==BIN_CMD) error("PUBLIC in absolute program");
 makeobj=1;
 while (sym(sbuf)) {
  if (strlen(sbuf)>15) error("Object name exceeds 15 characters");
  if (pass==1) {
   p=rawfind(sbuf);
   if (p==0) { p=install(sbuf,0); p[NAME]=SYMBFWD; }
   if (p[NAME]&SYMBEXT) error("PUBLIC is EXTERN");
   if ((p[NAME]&SYMBPUB)==0) {
    if (npub==255) error("Too many PUBLIC symbols");
    npub++; p[NAME]=p[NAME]|SYMBPUB;
   }
  }
  if (match(',')==0) break;
 }
}
extern_()
{
 char *p;
 if (bintype==BIN_CMD) error("EXTERN in absolute program");
 makeobj=1;
 while (sym(sbuf)) {
  if (strlen(sbuf)>15) error("Object name exceeds 15 characters");
  if (pass==1) {
   p=rawfind(sbuf);
   if (p) error("EXTERN redefined");
   else {
    if (nextidx==255) error("Too many EXTERN symbols");
    p=install(sbuf,nextidx++); p[NAME]=SYMBEXT;
   }
  }
  if (match(',')==0) break;
 }
}
main_()
{
 if (entryset) error("Too many entry points");
 if (nsect) error("MAIN in SECTION");
 entry=loc; entryset=1;
}
radix_()
{
 int v;
 v=exp_();
 if (flgrel | v<2 | v>16) error("Invalid RADIX");
 else radix=v;
}
rebasebss()
{
 char *p;
 p=start;
 while (p<freeptr) {
  if ((p[NAME]&SYMBLNK)==0 & (p[NAME]&SYMBBSS))
   mputw(p+VALUE,mgetw(p+VALUE)+codesize);
  p=p+strlen(p+NAME+1)+6;
 }
}
beginout(src)
char *src;
{
 int i;
 if (bintype==BIN_CMD) {
  if (ofile) strcpy(bfile,ofile);
  else addext(bfile,src,".CMD");
 } else {
  if (ofile) strcpy(pfile,ofile);
  else {
   if (makeobj) addext(pfile,src,".OBJ");
   else addext(pfile,src,".PGM");
  }
  if (makeobj) {
   addext(rfile,pfile,".REL");
   outpgm=fopen(rfile,"w+");
  } else {
   outpgm=fopen(pfile,"w+");
   i=0; while (i++<8) outword(0);
  }
  if (outpgm==0) { error("Cannot create output"); fatal(-1); }
  addext(bfile,pfile,".TMP"); relsize=0;
 }
 out=fopen(bfile,"w+");
 if (out==0) { error("Cannot create temporary code"); fatal(-1); }
}
emitref(offset,width)
int offset,width;
{
 if (nsect) return;
 if (makeobj) {
  if (flgrel==3) { width=width|128; refvalue=flgext; }
  if (putc(width,outpgm)==-1) fatal(-1);
  outword(refvalue); outword(offset); relsize++;
 } else {
  if (flgrel==3) error("Unresolved external in PGM");
  if (width!=1) outrel(offset);
 }
}
copycode(name)
char *name;
{
 int count;
 out=fopen(name,"r");
 if (out==0) { error("Cannot read temporary file"); fatal(-1); }
 while ((count=fread(obj,1,128,out))>0) {
  if (fwrite(obj,1,count,outpgm)!=count) {
   error("Write error"); fatal(-1);
  }
 }
 if (feof(out)==0) { error("Read error"); fatal(-1); }
 fclose(out); out=0;
}
objname(name)
char *name;
{
 int i,c;
 i=0;
 while (i<16) {
  c=0;
  if (*name) c=*name++;
  putc(c,outpgm); i++;
 }
}
objsymbols(kind)
int kind;
{
 char *p,*q;
 int val,flags;
 p=start;
 while (p<freeptr) {
  if (p[NAME]&kind) {
   q=p;
   if (p[NAME]&SYMBLNK) q=mgetw(p+VALUE);
   if (q[NAME]&SYMBFWD) error("Undefined PUBLIC symbol");
   tagname(p); objname(sbuf);
   val=0; flags=0;
   if (kind==SYMBPUB) {
    val=mgetw(q+VALUE);
    if ((q[NAME]&SYMBLBL)==0) flags=32768;
   }
   outword(val); outword(flags);
  }
  p=p+strlen(p+NAME+1)+6;
 }
}
finishout()
{
 int pos[2],i;
 fclose(out); out=0;
 if (makeobj) {
  fclose(outpgm); outpgm=fopen(pfile,"w+");
  if (outpgm==0) fatal(-1);
  outword(23205); outword(1); outword(npub); outword(nextidx);
  outword(filepos); outword(0); outword(32+(npub+nextidx)*20);
  outword(entry); outword(bssused);
  i=0; while (i++<7) outword(0);
  objsymbols(SYMBPUB); objsymbols(SYMBEXT);
  copycode(bfile); outword(relsize); copycode(rfile);
  putc(0,outpgm); unlink(rfile);
  ftell(outpgm,pos);
  if (pos[0]) error("OBJ exceeds 65535 bytes");
 } else {
  copycode(bfile);
  pos[0]=0; pos[1]=0;
  if (fseek(outpgm,pos,0)) fatal(-1);
  outword(42330); outword(relsize); outword(16+relsize*2);
  outword(filepos);
  if (entry==-1) entry=0;
  outword(entry); outword(bssused); outword(0); outword(0);
 }
 if (fclose(outpgm)) { error("Output close error"); fatal(-1); }
 outpgm=0; unlink(bfile);
 if (errcnt) { unlink(pfile); fatal(-1); }
}
