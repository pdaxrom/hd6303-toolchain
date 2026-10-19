/* MC68HC11 encoding with ordinary UniCROSS references. Prefixes precede
 * relocatable operands. Descriptor layout is defined in OPCODES.ASM.
 */
hc_mode(name)
char *name;
{
 if (strcmp(name,"hc11")==0 | strcmp(name,"68hc11")==0 |
     strcmp(name,"MC68HC11")==0) cpu_hc11=1;
 else if (strcmp(name,"hd6303")==0 | strcmp(name,"6303")==0 |
          strcmp(name,"HD6303")==0) cpu_hc11=0;
 else { printf("unias: unknown CPU %s\n",name); fatal(-1); }
}

hc_mnem(p)
char *p;
{
 int mode,val,reg;
 hc_index=0;
 if (p[OPFLAG]&128) {
  if ((p[OPFLAG]&96)!=0 & (p[DIRECT]==113 | p[DIRECT]==114)) {
   hc_bit(p,0); return;
  }
  error("HD6303-only instruction"); return;
 }
 if (p[OPFLAG]&32) { hc_bit(p,1); return; }
 if (p[INHERIT]) {
  if (p[INHERIT]==26) { error("SLP is HD6303-only"); return; }
  hc_emit(p,INHERIT,0); hc_end(); return;
 }
 if (p[REL]) { obj[0]=p[REL]; relative(); hc_end(); return; }
 if (match('#')) {
  val=exp_(); hc_emit(p,IMMED,val); hc_end(); return;
 }
 skip(); reg=hc_reg();
 if (reg) {
  val=0; if (match(',')) val=exp_(); mode=IND_X;
 } else {
  val=exp_();
  if (match(',')) {
   if (hc_reg()==0) error("X or Y expected");
   mode=IND_X;
  } else {
   mode=EXTEND;
   if (val>=0 & val<256 & p[DIRECT]!=0) {
    if (bintype==BIN_CMD | flgrel==0) mode=DIRECT;
    if ((p[DIRECT]&255)==157 & (val<40 | bintype!=BIN_CMD))
     mode=EXTEND;
   }
  }
 }
 hc_emit(p,mode,val); hc_end();
}

hc_reg()
{
 int r;
 skip(); r=*ip;
 if ((r=='X' | r=='Y') & isalnum(ip[1])==0 & ip[1]!='_') {
  ip++; hc_index=r=='Y'; return r;
 }
 return 0;
}

hc_prefix(p,mode)
char *p;
int mode;
{
 int op;
 op=p[IND_X]&255;
 if (p[OPFLAG]&16) {
  if (mode==IND_X & hc_index==0) return 26;
  return 24;
 }
 if (p[OPFLAG]&8) {
  if (mode==IND_X & hc_index) return 205;
  return 26;
 }
 if (mode==IND_X & hc_index) {
  if (op==172 | op==238 | op==239) return 205;
  return 24;
 }
 if (p[OPFLAG]&4) return 24;
 return 0;
}

hc_emit(p,mode,val)
char *p;
int mode,val;
{
 int pre,off,op;
 pre=hc_prefix(p,mode); off=0; op=p[mode]&255;
 if (op==0) { error("Illegal HC11 address mode"); nbytes=0; return; }
 if (mode==INHERIT & op==24) op=143; /* XGDX: HC11 8F, HD6303 18. */
 if (pre) obj[off++]=pre;
 obj[off++]=op;
 if (mode==INHERIT) { nbytes=off; return; }
 if (mode==IND_X) {
  if (flgrel | val<0 | val>255)
   error("HC11 indexed offset must be absolute 0..255");
 } else if (mode==IMMED & (p[OPFLAG]&1)==0) {
  if (val< -128 | val>255) error("HC11 byte operand out of range");
 }
 if (mode==EXTEND | (mode==IMMED & (p[OPFLAG]&1))) {
  if (pass==2 & bintype!=BIN_CMD & flgrel!=0) emitref(loc+off,3);
  obj[off++]=val>>8;
 } else if (mode!=IND_X) {
  if (pass==2 & bintype!=BIN_CMD & flgrel!=0)
   emitref(loc+off,refpart+1-(refpart!=0));
 }
 obj[off++]=val&255; nbytes=off;
}

/* BSET addr[,X|Y],#mask; BRSET addr[,X|Y],#mask,label.
 * '#' before the mask is optional. Bit branches have no PC relocations.
 */
hc_bit(p,branch)
char *p;
int branch;
{
 int addr,mask,mode,op,off,disp;
 addr=exp_(); mode=DIRECT;
 if (flgrel) error("HC11 bit address/offset must be absolute");
 if (addr<0 | addr>255) error("HC11 bit address/offset out of range");
 if (match(',')==0) { error("Comma before HC11 bit mask required"); return; }
 if (hc_reg()) {
  mode=IND_X;
  if (match(',')==0) { error("Comma before HC11 bit mask required"); return; }
 }
 match('#'); mask=exp_();
 if (flgrel | mask<0 | mask>255)
  error("HC11 bit mask must be absolute 0..255");
 if (branch) op=p[mode];
 else {
  op=20; if (p[DIRECT]==113) op=21;
  if (mode==IND_X) op=op+8;
 }
 off=0;
 if (hc_index) obj[off++]=24;
 obj[off++]=op; obj[off++]=addr; obj[off++]=mask;
 if (branch) {
  if (match(',')==0) { error("HC11 bit branch target required"); return; }
  disp=exp_()-loc-off-1;
  if (flgrel==3) error("External branch not allowed");
  if (pass==2 & (disp< -128 | disp>127)) error("branch out of range");
  obj[off++]=disp;
 }
 nbytes=off; hc_end();
}

hc_end()
{
 skip();
 if (*ip>32 & *ip!=';') error("Unexpected HC11 operand");
}
