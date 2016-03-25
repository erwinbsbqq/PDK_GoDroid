/*------------------------------------
 *     void _cpu_suspend()	
 *-----------------------------------*/

#if defined(CONFIG_ALI_CHIP_M3921)
    .globl _cpu_suspend 
_cpu_suspend:
1:  nop
    nop
    b   1b
#else
	.align  4
	.globl  _cpu_suspend
	.ent    _cpu_suspend

_cpu_suspend:
1:  nop
	.word	0x42000020
	b	1b
	nop
    .end    _cpu_suspend
    .set    at
    .set    reorder
#endif   
        
