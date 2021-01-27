/*=================================================================================================
NOTE:
	This file contains the basic definitions for S3C2440+K9F2G08.  
	To use on other hardware platforms,please modify accordingly.	
==================================================================================================*/


#ifndef _FLASH_H_
#define _FLASH_H_

// #include "Types.h"

/*=================================================================================================
NOTE:
	PLEASE DO NOT make any change!
==================================================================================================*/


#ifndef _TYPES_H_


#define U8  unsigned char
#define U16 unsigned short
#define U32 unsigned int


#endif


////////////////////////////////////NAND FLASH INFORMATION//////////////////////////////////////
//SAMSUNG K9F2G08
#define NAND_PAGE_SIZE			2048			//PAGE SIZE  (BYTES)
#define NAND_BLOCK_SIZE			(128*1024)		//BLOCK SIZE (BYTES)
#define NAND_BLOCK_NUM			2048			//BLOCK NUMBER
#define NAND_BLOCK_PAGE_NUM		64				//BLOCK PAGE NUMBER


////////////////////////////////////////MCU REGISTERS///////////////////////////////////////////
//SAMSUNG S3C2440
#define NFCONF					0x4E000000
#define NFCTRL					0x4E000004
#define NFCMD					0x4E000008
#define NFADDR					0x4E00000C
#define NFDATA					0x4E000010
#define NFSTAT					0x4E000020


//////////////////////////////////////////DON'T CHANGE//////////////////////////////////////////

#define SysAddr8(addr)  ((volatile U8*)(addr))
#define SysAddr16(addr) ((volatile U16*)(addr))
#define SysAddr32(addr) ((volatile U32*)(addr))


extern void nand_init(void);
extern U32  nand_read_id(void);
extern void nand_read_page(U32 blockidx, U32 pageidx);
extern U32  nand_program_page(U32 blockidx, U32 pageidx);
extern U32  nand_erase_block(U32 blockidx);
extern U32  nand_check_blank(U32 blockidx);
extern U32  nand_mark_badblock(U32 blockidx);
extern U32  nand_is_badblock(U32 blockidx);
extern U32  nand_info_table(void);


#endif

