extern int inline2();
#asm
inline2 db 86
private_assembly_label_longer_than_c_names
    tsx
    ldaa 2,x
    ldab 3,x
    jmp RTSC
#endasm
int inlinen(n)
int n;
{
 return inline2(n);
}
