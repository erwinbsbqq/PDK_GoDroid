#ifndef _LINUX_ALI_OVG_REG_H
#define _LINUX_ALI_OVG_REG_H

/*Define index of HW register*/
//#define ALI_OPEN_VG_BASE_ADDR 0xA1000000
#define ALI_OPEN_VG_BASE_ADDR 0xB800A000
#define ALI_SOC_BASE_ADDR 		0xB8000000
 
#define	ALI_OPENVG_GET_UINT32(i)		(*(volatile UINT32 *)(ALI_OPEN_VG_BASE_ADDR+i))
#define ALI_OPENVG_SET_UINT32(i,d)	((*((volatile UINT32*)(ALI_OPEN_VG_BASE_ADDR+i)))=d)
#define ALI_OPENVG_GET_FLOAT(i)			(*((volatile float*)(ALI_OPEN_VG_BASE_ADDR+i)))
#define ALI_OPENVG_SET_FLOAT(i,d)   ((*((volatile float*)(ALI_OPEN_VG_BASE_ADDR+i)))=d)

//index of register
#define VG_HWRESET					       (*(volatile UINT32 *)(ALI_SOC_BASE_ADDR+0x80))
#define VG_HWCLOCK					       (*(volatile UINT32 *)(ALI_SOC_BASE_ADDR+0x64))
#define VG_CMD_Q_CTRL_INDX					0x00
#define VG_LOW_PRI_CMD_Q_BA_INDX		0x04
#define VG_LOW_PRI_CMD_NUMBER_INDX	0x08
#define VG_MED_PRI_CMD_Q_BA_INDX		0x0C
#define VG_MED_PRI_CMD_NUMBER_INDX	0x10
#define VG_HIGH_PRI_CMD_Q_BA_INDX		0x14
#define VG_HIGH_PRI_CMD_NUMBER_INDX	0x18
#define VG_CTRL_INDX								0x20 //This register is for internal IP test
/*
#define VG_GRANT_COMMAND_INDX                      0x1C // New added 20100526

#define VG_CPU_SRAM_D_INDX			0x24 //This register is for internal IP test
#define VG_CPU_SRAM_CTRL_INDX			0x28 //This register is for internal IP test
#define VG_CPU_SRAM_Q_INDX			0x2C
#define VG_TESS_PIN_REGISTER_INDX                  0x30 // New added 20100526
#define VG_TESS_OUT_REGISTER_INDX                  0x34 // New added 20100526

#define VG_DRAM_CNT_INDX                           0x3C // New added 20100526
*/
#define VG_BUSY_CNT_INDX                           0x38 // New added 20100526
#define VG_TESS_CTRL_INDX			0x40
/*
#define VG_TESS_DATA_BA_INDX			0x44 //coordinate of path segment
#define VG_STROKE_LINE_WIDTH_DIV2_INDX		0x48
#define VG_STROKE_MITER_LIMIT_INDX		0x4C
#define VG_STROKE_DASH_PHASE_INDX		0x50
#define VG_STROKE_DASH_PTN_BA_INDX		0x54
#define VG_TESS_SEGMENT_BA_INDX			0x58 //the content is path segment
#define VG_TESS_AF_M_A_INDX			0x5C //The tessellation affine transform matrix[0][0]
#define VG_TESS_AF_M_B_INDX			0x60 //The tessellation affine transform matrix[0][1]
#define VG_TESS_AF_M_C_INDX			0x64 //The tessellation affine transform matrix[0][2]
#define VG_TESS_AF_M_D_INDX			0x68 //The tessellation affine transform matrix[1][0]
#define VG_TESS_AF_M_E_INDX			0x6C //The tessellation affine transform matrix[1][1]
#define VG_TESS_AF_M_F_INDX			0x70 //The tessellation affine transform matrix[1][2]
#define VG_TESS_VP_X_WIDTH_INDX			0x74
#define VG_TESS_VP_Y_HIGHT_INDX			0x78
#define VG_TESS_VERTEX_OUTPUT_BA_INDX		0x7C
#define VG_TESS_VERTEX_SWAP_BA_INDX		0x80
#define VG_TESS_HISTOGRM_OUTPUT_BA_INDX	        0x84
*/
#define VG_TESS_ADAPTIVE_CUBIC_THR_INDX		0x88
#define VG_TESS_MODE_RADIUS_INDX          0x8C // New added 20100526
/*
#define VG_TESS_READ_EGDE_SIZE_INDX                0x90 // New added 20100526
#define VG_TESS_POINT_SEGMENT_SIZE_HIGH_BITS_INDX  0x94 // New added 20100526
#define VG_TESS_READ_BBMINX_INDX                   0x98 // New added 20100526
#define VG_TESS_READ_BBMINY_INDX                   0x9C // New added 20100526
#define VG_TESS_READ_BBMAXX_INDX                   0xA0 // New added 20100526
#define VG_TESS_READ_BBMAXY_INDX                   0xA4 // New added 20100526
#define VG_FIXED_DRAM_LAT_CTR_INDX                 0xB0 // New added 20100526
*/
#define VG_RAST_CTRL_INDX			0x100
/*
#define VG_RAST_BOUND_BOX_SX_EX_INDX		0x104
#define VG_RAST_BOUND_BOX_SY_EY_INDX		0x108
#define VG_RAST_VERTEX_BA_INDX			0x10C
#define VG_RAST_HISTOGRM_BA_INDX		0x110
#define VG_RAST_SWAP1_BA_INDX			0x114
#define VG_RAST_SWAP2_BA_INDX			0x118
#define VG_RAST_ASE_BA_INDX			0x11C
#define VG_RAST_SCISSOR_EX_EY_INDX		0x120
#define VG_RAST_VPX_WIDTH_INDX			0x124
#define VG_RAST_VERTEX_SIZE_INDX		0x128
#define VG_RAST_SCISSOR_SX_SY_INDX		0x12C
#define VG_RAST_SIMPLE_RADIUS_INDX                 0x130 // New added 20100526
*/
#define VG_IMAGE_CTR_INDX			0x140

#define VG_MMU_ENABLE					0x270 // New added 20100526
#define VG_MMU_STATUS_ASE				0x274
#define VG_MMU_STATUS_CMDQ				0x278
#define VG_MMU_STATUS_IMWR			0x27C
#define VG_ID							0x280
#define VG_MMU_STATUS_IMRD			0x284
#define VG_MMU_STATUS_TERD			0x288
#define VG_MMU_INTERRUPT_STATUS		0x28C 
#define VG_MMU_STATUS_TEWR			0x290
#define VG_MMU_STATUS_RASTRD			0x294
#define VG_MMU_STATUS_RASTWR		0x298

/*
#define VG_IMAGE_S2P_M00_INDX			0x144 //surface to paint matrix[0][0], fix-point s15.16
#define VG_IMAGE_S2P_M01_INDX			0x148 //surface to paint matrix[0][1], fix-point s15.16
#define VG_IMAGE_S2P_M02_INDX			0x14C //surface to paint matrix[0][2], fix-point s15.16
#define VG_IMAGE_S2P_M10_INDX			0x150 //surface to paint matrix[1][0], fix-point s15.16
#define VG_IMAGE_S2P_M11_INDX			0x154 //surface to paint matrix[1][1], fix-point s15.16
#define VG_IMAGE_S2P_M12_INDX			0x158 //surface to paint matrix[1][2], fix-point s15.16
#define VG_IMAGE_S2I_M00_INDX			0x160 //Surface to Image Matrix [0][0],  fix-point format, s15.16
#define VG_IMAGE_S2I_M01_INDX			0x164 //Surface to Image Matrix [0][1],  fix-point format, s15.16
#define VG_IMAGE_S2I_M02_INDX			0x168 //Surface to Image Matrix [0][2],  fix-point format, s15.16
#define VG_IMAGE_S2I_M10_INDX			0x16C //Surface to Image Matrix [1][0],  fix-point format, s15.16
#define VG_IMAGE_S2I_M11_INDX			0x170 //Surface to Image Matrix [1][1],  fix-point format, s15.16
#define VG_IMAGE_S2I_M12_INDX			0x174 //Surface to Image Matrix [1][2],  fix-point format, s15.16
#define VG_IMAGE_COLOR_BA_INDX			0x17C //The destination color buffer base address
#define VG_IMAGE_MASK_BA_INDX			0x180 //The destination mask buffer base address
#define VG_IMAGE_RADIAL_A_INDX			0x184
#define VG_IMAGE_RADIAL_B_Y_INDX		0x188
#define VG_IMAGE_RADIAL_B_C_INDX		0x18C
#define VG_IMAGE_RADIAL_C_INDX			0x190
#define VG_IMAGE_RADIAL_D_Y_INDX		0x194
#define VG_IMAGE_RADIAL_D_C_INDX		0x198
#define VG_IMAGE_RADIAL_E_YY_INDX		0x19C
#define VG_IMAGE_RADIAL_E_Y_INDX		0x1A0
#define VG_IMAGE_RADIAL_E_C_INDX		0x1A4
#define VG_IMAGE_RADIAL_DEN_INDX		0x1A8
#define VG_IMAGE_RADIAL_DEN2_INDX		0x1AC
#define VG_IMAGE_LINEAR_A_X_INDX		0x1B0
#define VG_IMAGE_LINEAR_B_Y_INDX		0x1B4
#define VG_IMAGE_LINEAR_C_INDX			0x1B8
#define VG_IMAGE_GLOBAL_ALPHA_INDX              0x1BC // Doesn't exist in datasheet 
#define VG_IMAGE_TIL_FILL_COLOR_GB_INDX		0x1C0
#define VG_IMAGE_TIL_FILL_COLOR_AR_INDX		0x1C4
#define VG_IMAGE_PAINT_COLOR_GB_INDX		0x1C8
#define VG_IMAGE_PAINT_COLOR_AR_INDX		0x1CC
#define VG_IMAGE_PAT_PARA_INDX			0x1D0
#define VG_IMAGE_PAT_OFFSET_XY_INDX		0x1D4
#define VG_IMAGE_IM_PARA_INDX			0x1D8
#define VG_IMAGE_IM_OFFSET_XY_INDX		0x1DC
#define VG_IMAGE_COLOR_DEST_PARA_INDX		0x1E0
#define VG_IMAGE_COLOR_DEST_OFFSET_XY_INDX	0x1E4
#define VG_IMAGE_MASK_DEST_PARA_INDX		0x1E8
#define VG_IMAGE_MASK_DEST_OFFSET_XY_INDX	0x1EC
#define VG_IMAGE_IMAGE_BA_INDX			0x1F0
#define VG_IMAGE_PATTERN_BA_INDX		0x1F4
#define VG_IMAGE_COLORRAMP_SRAM_D_INDX		0x1F8
#define VG_IMAGE_COLORRAMP_SRAM_CTR_INDX	0x1FC
#define VG_IMAGE_COLOR_TRANSFORM_VALUE_01_INDX	0x200
#define VG_IMAGE_COLOR_TRANSFORM_VALUE_23_INDX	0x204
#define VG_IMAGE_COLOR_TRANSFORM_VALUE_45_INDX	0x208
#define VG_IMAGE_COLOR_TRANSFORM_VALUE_67_INDX	0x20C
#define VG_IMAGE_IM_WIDTH_HEIGHT_INDX              0x210 // Doesn't exist in datasheet 
#define VG_IMAGE_PAT_WIDTH_HEIGHT_INDX             0x214 // Doesn't exist in datasheet 
#define VG_IMAGE_COLOR_WIDTH_HEIGHT_INDX           0x218 // Doesn't exist in datasheet 
#define VG_IMAGE_MASK_WIDTH_HEIGHT_INDX            0x21C // Doesn't exist in datasheet 
#define VG_IMAGE_IM_WIDTH_HEIGHT_R_INDX            0x220 // Doesn't exist in datasheet 
#define VG_IMAGE_PAT_WIDTH_HEIGHT_R_INDX           0x224 // Doesn't exist in datasheet 
#define VG_IMAGE_UNKNOWN_INDX                      0x228 // New added 20100526
#define VG_IMAGE_MASK_WIDTH_HEIGHT_R_INDX          0x22C // New added 20100526
#define VG_IMAGE_CKC_AR_REGISTER_INDX              0x230 // New added 20100526
#define VG_IMAGE_CKC_GB_REGISTER_INDX              0x234 // New added 20100526
#define VG_IMAGE_CKC_CTR_REGISTER_INDX             0x238 // New added 20100526
#define VG_ID_INDX                                 0x280 // New added 20100526


//Declare function for R/W HW reg
//bool ali_ovg_check_cmdQ_busy() {}
*/
#endif
