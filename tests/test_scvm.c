/* Build with the same interpreter, and compare against independent wider math. */
#define main scvm_program_main
#include "../scvm.c"
#undef main
#include <assert.h>
int main(void) {
 const word values[]={0,1,3,7,0x7fff,0x8000,0x8001,0xfff9,0xfffd,0xffff};
 unsigned count=0;
 for (unsigned a=0;a<10;a++) {
  for (unsigned n=0;n<64;n++) {
   uint32_t expected=values[a];
   for(unsigned k=0;k<(n&31);k++)expected=(expected>>1)|(expected&0x8000);
   assert(asr16(values[a],n)==expected);count++;
  }
  for(unsigned b=0;b<10;b++)if(values[b]) {
   int32_t x=values[a],y=values[b];
   if(x&0x8000)x-=65536;if(y&0x8000)y-=65536;
   assert(div16(values[a],values[b],0)==(word)(x/y));
   assert(div16(values[a],values[b],1)==(word)(x%y));count+=2;
  }
 }
 mem[65535]=0x12;mem[0]=0x34;assert(GETWORD(65535)==0x1234);
 printf("PASS host VM: %u signed division/remainder/shift cases and address wrap\n",count);
 return 0;
}
