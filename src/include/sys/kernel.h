#ifndef _KERNEL_H_
#define _KERNEL_H_

void x86_cpuid(int id_eax, int id_ecx, int *eax, int *ebx, int *ecx, int *edx);
void init();


#endif

