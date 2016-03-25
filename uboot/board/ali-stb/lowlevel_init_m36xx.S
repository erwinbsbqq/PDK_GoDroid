/* Memory sub-system initialization code */

#include <config.h>
#include <asm/regdef.h>
#include <asm/m39xx.h>
#include <asm/mipsregs.h>

#define MMFC0(rt,rd,sel)	.word	0x40000000|(rt<<16)|(rd<<11)|sel
#define MMTC0(rt,rd,sel)	.word	0x40800000|(rt<<16)|(rd<<11)|sel

	.text
	.set noreorder
	.set mips32
	
	.globl	lowlevel_init
lowlevel_init:
	move    v0,ra  
	nop
	nop
	
# 1. Read CPU Num: Main CPU or SEE CPU (jump to 0xb8000280)    
	MMFC0(8,15,1)					# read CPUnum(Ebase register[9:0] : 0=> MAIN, 1=> SEE)
	andi    t0, 0x3ff
	beqz    t0, main_cpu
	nop
see_cpu:	
	li      t0, 0xb8000280
	jr      t0
	nop
main_cpu:

# 2. Disable EROM
	lui     t0, 0xb800
	lw      t1, 0x74(t0)
	or      t1, 0x40000000
	and     t1, 0xfffbffff
	sw      t1, 0x74(t0)
	
# 3. Init CPU for boot up.
	mfc0    t0, CP0_CONFIG
	li      t1, ~0x7
	and     t0, t1
	ori     t0, t0, 3
	mtc0    t0, CP0_CONFIG			# cacheable, write-back
	MMFC0(8,16,7)      
	ori     t0, 0x30
	MMTC0(8,16,7)					# set ULB to fix m36 NB bug(flash/erom deadlock), set NBLSU to fix mips24ke 2.1.0 Errata E54	
	
# 4. Check Chip Bonding (Chip ID and Version Register 0xB8000000)
#ifdef CONFIG_RAMBOOT
	j		init_end
#endif	
	lui  	t0, 0xb800   
	lw   	t1, 0(t0)
	and  	t1, 0xFFFF0CFF					# [31:16] 16'h3901/16'h3603, [11:00] Package Option
	beq  	t1, 0x36030C01, ddr_m3603_init	# 2'b11 M3603
	nop	
	beq  	t1, 0x36030801, ddr_m3606_init	# 2'b10 M3606
	nop	
	beq  	t1, 0x36020082, ddr_m3602_init	# M3602B
	nop		
	nop	
	.word	0x7000003f						# SDBBP opcode	( Not 3603/3606 Chip )
	nop			

# 5. Init Memory	
ddr_m3602_init:								# Not Implement Yet
	
	nop
	//.word	0x7000003f						# SDBBP opcode
	nop	
	j		init_end	
	
ddr_m3603_init:								# Not Implement Yet
	# (1) DDR driving/mclk setting
	li		t0, 0xb8001200	
	li		t1, 0x00008010
	sw		t1, 0x74(t0)	
	lhu		t2, 0x72(t0)		      
	li		t1, 0x04048020
	sw		t1, 0x74(t0)	       
	addiu 	t2,0x2c2c
	sh		t2, 0x72(t0)	
	li		t1, 0x4444
	sh		t1, 0x70(t0)
##CL = 6 
	li		t1, 0x4310  
	sh  	t1, 0x78(t0)   
	li  	t1, 0x01
	sb		t1, 0x7a(t0)
	
	# (2) Turn on ODT   
	li		t1, 0x0FF33000
	sw		t1, 0x7c(t0)
	li		t1, 0x0FF33000
	sw		t1, 0x80(t0)
	li		t1, 0x02244000
	sw		t1, 0x84(t0)
	li		t1, 0x02244000
	sw		t1, 0x88(t0)			
	li		t1, 0x02244000
	sw		t1, 0x8c(t0)
	li		t1, 0x00000000
	sw		t1, 0x90(t0)
	li		t1, 0x02244000
	sw		t1, 0x94(t0)
	li		t1, 0x00000000
	sw		t1, 0x98(t0)		
	li		t1, 0x00066000
	sw		t1, 0x9c(t0) 
        	          
	# (3) refresh disabled	
	li		t0, 0xb8001000
	li      t1, 0xb861       
	sh      t1, 0x30(t0) 
        	
	# (4) set parameters  
	#128MB,CL=5,CS=1
	li	    t1, 0x9c00c383
	sw      t1, 0x0(t0) 
        	                   
	# (5) Issue nop    
	li      t1, 0x0C   
	sb      t1, 0x04(t0)     
	li      t2, 0xa000aa60   
	lw      t1, 0x0(t2)      
                                                                      	                              	
	# (6) Issue Precharge       
	li      t1, 0x14   
	sb      t1, 0x04(t0)     
	li      t2, 0xa00026a0   
	lw      t1, 0x0(t2)   
        	     
	# (7) Issue MRS
	li      t1, 0x1C   
	sb      t1, 0x04(t0)     
	li      t2, 0xa0080000   
	lw      t1, 0x0(t2)   
	li      t2, 0xa00C0000   
	lw      t1, 0x0(t2)   
	li      t2, 0xa0040400   
	lw      t1, 0x0(t2)  
#	li      t2, 0xa000b5b0         #CL=5
	li      t2, 0xa000b6b0         #CL=6  
	lw      t1, 0x0(t2) 
        	
	# (8) Issue Precharge All
	li      t1, 0x14   
	sb      t1, 0x04(t0)     
	li      t2, 0xa00026a0   
	lw      t1, 0x0(t2) 
#	li      t2, 0xa80026a0   # just one CS
#	lw      t1, 0x0(t2) 
	
	# (9) Issue 2 Refresh
	li      t1, 0x24   
	sb      t1, 0x04(t0)     	                  
	li      t2, 0xa00026a0   
	lw      t1, 0x0(t2)      	                  
	lw      t1, 0x0(t2)        
#	li      t2, 0xa80026a0
#	lw      t1, 0x0(t2)
#	lw      t1, 0x0(t2) 
	
	# (10) Issue Final MRS
	li      t1, 0x1C   
	sb      t1, 0x04(t0)     
	                  
#	li      t2, 0xa000a5b0   #CL=5
	li      t2, 0xa000a6b0   #CL=6    
	lw      t1, 0x0(t2) 
				
	li      t2, 0xa0043c00  
	lw      t1, 0x0(t2)      
## $display("\nOCD setting out:\t %t\n", $realtime); and ODT value of DRAM 150 Ohm	
	li      t2, 0xa0040400  
	lw      t1, 0x0(t2)  

##	li      t2, 0xa0040400  
##	lw      t1, 0x0(t2)  
   
	# (11) Register final value: default is 32bit CL=5 128M
#	li      t1, 0x569a4083    # 16bit CL=5 128M
#	li      t1, 0x569a4082    # 16bit CL=5 64M
#	li      t1, 0x569a4081    # 16bit CL=5 32M	
#	li      t1, 0xd69a4084    # 32bit CL=5 256M
#	li      t1, 0xd69a4083    # 32bit CL=5 128M
#	li      t1, 0xd69a4082    # 32bit CL=5 64M		
#	li      t1, 0x569a40c3    # 16bit CL=6 128M
#	li      t1, 0x569a40c2    # 16bit CL=6 64M
#	li      t1, 0x569a40c1    # 16bit CL=6 32M	
#	li      t1, 0xd69a40c4    # 32bit CL=6 256M
#	li      t1, 0xd69a40c3    # 32bit CL=6 128M
#	li      t1, 0xd69a40c2    # 32bit CL=6 64M		
	li      t1, 0xd69a40c3    # 32bit CL=6 128M
	sw      t1, 0(t0)
	
	# (12) end initialization
	li		t1, 0x0011AF40    	#no remap
#	li		t1, 0x0211AF40    	#remap				
	sw		t1, 0x04(t0)	
	li		t1, 0x50          #Norman mode CL=5
#	li  	t1, 0x10          #bypass mode CL=5				
	sb		t1, 0x31(t0)	
	
	# (13) Turn on DRAM ODT    		
	li		t1, 0x80      				
	sb		t1, 0x33(t0)
			  						
	# (14) performance setting
	li      t1, 0x4000     #Enable innerloop clock gated and mem sequencer clock gated
	sh      t1, 0x0C(t0)
	li      t1, 0xFF000000 #Set all latency source to master all high priority and QL enable 
	sw      t1, 0x10(t0)
	li      t1, 0x33333333 #Set command slice counter 
	sw      t1, 0x18(t0)
	li      t1, 0x0F0F0F0F #Set latency counter of MST3-0
	sw      t1, 0x20(t0)
	li      t1, 0x0F0F0F0F #Set latency counter of MST7-4
	sw      t1, 0x24(t0)
	li      t1, 0x00008000 #Disable mem sub-sequencer clock gated
	sw      t1, 0x08(t0)
	li      t1, 0x33330F00 #Set all latency source to master all high priority enable command slice counter
	sw      t1, 0x14(t0)
	li      t1, 0xFFFFFFFF #Set latency counter of MST3-0
	sw      t1, 0x1c(t0)
	li      t1, 0x00008000 #Disable mem sub-sequencer clock gated
	sw      t1, 0x34(t0)
	li      t1, 0x33330F00 #Set all latency source to master all high priority enable command slice counter
	sw      t1, 0x38(t0)  
	li      t1, 0xFFFFFFFF  #Set latency counter of MST3-0
	sw      t1, 0x3c(t0)
	li      t1, 0x00008000  #Disable mem sub-sequencer clock gated
	sw      t1, 0x28(t0)
	li      t1, 0x33330F00  #Set all latency source to master all high priority enable command slice counter
	sw      t1, 0x2c(t0)
	li      t1, 0xFFFFFFFF  #Set latency counter of MST3-0
	sw      t1, 0x68(t0)        	
	j		init_end
	nop
			 
ddr_m3606_init:	
	# (1) DDR driving/mclk setting
	li		t0, 0xb8001200        
	li		t1, 0x00008010
	sw		t1, 0x74(t0)        
	lhu		t2, 0x72(t0)	              
	li		t1, 0x00008020
	sw		t1, 0x74(t0)               
	addiu 	t2,0x2C2C
	sh		t2, 0x72(t0)
	li		t1, 0x4545
	sh		t1, 0x70(t0)                
##CL = 6 
	li  	t1, 0x4510  
	sh  	t1, 0x78(t0)   
	li  	t1, 0x01
	sb  	t1, 0x7a(t0)
      	  	
	# (2) Turn on ODT  
#	li		t1, 0x09933000
	li		t1, 0x0FF33000
	sw		t1, 0x7c(t0)
#	li		t1, 0x09933000
	li		t1, 0x0FF33000
	sw		t1, 0x80(t0)
#	li		t1, 0x08866000
	li		t1, 0x07766000
	sw		t1, 0x84(t0)
	li		t1, 0x00000000
	sw		t1, 0x88(t0)				
	li		t1, 0x02266000
	sw		t1, 0x8c(t0)
	li		t1, 0x00000000
	sw		t1, 0x90(t0)
	li		t1, 0x00000000
	sw		t1, 0x94(t0)
	li		t1, 0x00000000
	sw		t1, 0x98(t0)		        
	li		t1, 0x00055000
	sw		t1, 0x9c(t0) 
        	           
	# (3) refresh disabled	
	li		t0, 0xb8001000
	li      t1, 0xb861       
	sh      t1, 0x30(t0)  
        	
	# (4) set parameters 
	#128MB,CL=5,CS=1
	li	    t1, 0x9c00c383
	sw      t1, 0x0(t0)    
        	                    
	# (5) Issue nop 
	li      t1, 0x0C   
	sb      t1, 0x04(t0)     
	li      t2, 0xa000aa60   
	lw      t1, 0x0(t2)
        	                                 	
	# (6) Issue Precharge  
	li      t1, 0x14   
	sb      t1, 0x04(t0)     
	li      t2, 0xa00026a0   
	lw      t1, 0x0(t2)
        	          
	# (7) Issue MRS
	li      t1, 0x1C   
	sb      t1, 0x04(t0)     
	li      t2, 0xa0080000   
	lw      t1, 0x0(t2)   
	li      t2, 0xa00C0000   
	lw      t1, 0x0(t2)   
	li      t2, 0xa0040400   
	lw      t1, 0x0(t2)  
#	li      t2, 0xa000b5b0         #CL=5
	li      t2, 0xa000b6b0         #CL=6  
	lw      t1, 0x0(t2)      
        	
	# (8) Issue Precharge All
	li      t1, 0x14   
	sb      t1, 0x04(t0)     
	li      t2, 0xa00026a0   
	lw      t1, 0x0(t2) 
#	li      t2, 0xa80026a0   # just one CS
#	lw      t1, 0x0(t2)    
	
	# (9) Issue 2 Refresh
	li      t1, 0x24   
	sb      t1, 0x04(t0)     		          
	li      t2, 0xa00026a0   
	lw      t1, 0x0(t2)      		          
	lw      t1, 0x0(t2)	
#	li      t2, 0xa80026a0
#	lw      t1, 0x0(t2)
#	lw      t1, 0x0(t2) 
	
	# (10) Issue Final MRS
	li      t1, 0x1C   
	sb      t1, 0x04(t0)     		          
#	li      t2, 0xa000a5b0   #CL=5
	li      t2, 0xa000a6b0   #CL=6    
	lw      t1, 0x0(t2) 				
	li      t2, 0xa0043c00  
	lw      t1, 0x0(t2)      
## $display("\nOCD setting out:\t %t\n", $realtime); and ODT value of DRAM 75 ohm	
	li      t2, 0xa0040040  
	lw      t1, 0x0(t2)  

##	li      t2, 0xa0040400  
##	lw      t1, 0x0(t2) 

	# (11) Register final value: default is 32bit CL=5 128M
#	li      t1, 0x569a4083    # 16bit CL=5 128M
#	li      t1, 0x569a4082    # 16bit CL=5 64M
#	li      t1, 0x569a4081    # 16bit CL=5 32M	
#	li      t1, 0xd69a4084    # 32bit CL=5 256M
#	li      t1, 0xd69a4083    # 32bit CL=5 128M
#	li      t1, 0xd69a4082    # 32bit CL=5 64M		
	li      t1, 0x569a40c3    # 16bit CL=6 128M
#	li      t1, 0x569a40c2    # 16bit CL=6 64M
#	li      t1, 0x569a40c1    # 16bit CL=6 32M	
#	li      t1, 0xd69a40c4    # 32bit CL=6 256M
#	li      t1, 0xd69a40c3    # 32bit CL=6 128M
#	li      t1, 0xd69a40c2    # 32bit CL=6 64M		

#	li      t1, 0x569a40c2    # 16bit CL=6 64M
	sw      t1, 0(t0)

	# (12) end initialization
	li		t1, 0x0011AF40    	#no remap
#	li		t1, 0x0211AF40    	#remap	
	sw		t1, 0x04(t0)	
	li		t1, 0x50          #Norman mode CL=5
#	li  	t1, 0x10          #bypass mode CL=5	
	sb		t1, 0x31(t0)				
					
	# (13) Turn on DRAM ODT    							
	li		t1, 0x80      					
	sb		t1, 0x33(t0)	
			  	
	# (14) performance setting
	li      t1, 0x4000     #Enable innerloop clock gated and mem sequencer clock gated
	sh      t1, 0x0C(t0)
	li      t1, 0xFF000000 #Set all latency source to master all high priority and QL enable 
	sw      t1, 0x10(t0)
	li      t1, 0x33333333 #Set command slice counter 
	sw      t1, 0x18(t0)
	li      t1, 0x0F0F0F0F #Set latency counter of MST3-0
	sw      t1, 0x20(t0)
	li      t1, 0x0F0F0F0F #Set latency counter of MST7-4
	sw      t1, 0x24(t0)
	li      t1, 0x00008000 #Disable mem sub-sequencer clock gated
	sw      t1, 0x08(t0)
	li      t1, 0x33330F00 #Set all latency source to master all high priority enable command slice counter
	sw      t1, 0x14(t0)
	li      t1, 0xFFFFFFFF #Set latency counter of MST3-0
	sw      t1, 0x1c(t0)
	li      t1, 0x00008000 #Disable mem sub-sequencer clock gated
	sw      t1, 0x34(t0)
	li      t1, 0x33330F00 #Set all latency source to master all high priority enable command slice counter
	sw      t1, 0x38(t0)  
	li      t1, 0xFFFFFFFF  #Set latency counter of MST3-0
	sw      t1, 0x3c(t0)
	li      t1, 0x00008000  #Disable mem sub-sequencer clock gated
	sw      t1, 0x28(t0)
	li      t1, 0x33330F00  #Set all latency source to master all high priority enable command slice counter
	sw      t1, 0x2c(t0)
	li      t1, 0xFFFFFFFF  #Set latency counter of MST3-0
	sw      t1, 0x68(t0) 
	j		init_end
	nop	
	
init_end:
	nop	
	move    ra, v0
	jr      ra
	nop
	.end    lowlevel_init
