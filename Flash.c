/*=================================================================================================
NOTE:
	This file implements all the operations for S3C2440+K9F2G08.  
	To use on other hardware platforms, please modify accordingly.	
==================================================================================================*/


// #include "Types.h"
// #include "Comm.h"
#include "Flash.h"


int read_from_host();
void write_to_host(int word);

/*=================================================================================================
FUNCTION: nand_wait()          

DESCRIPTION: 
	Used to check if device is busy or ready for new command. Return only when the device is ready.
   
ARGUMENTS PASSED:
	None
			
RETURN VALUE:
	None
==================================================================================================*/
void nand_wait(void)
{
	while( !(*SysAddr16(NFSTAT) & 0x1) );
}



/*=================================================================================================
FUNCTION: nand_init()          

DESCRIPTION: 
	Used to do some initialization. User can put any initialization inside this function.
   
ARGUMENTS PASSED:
	None
			
RETURN VALUE:
	None
==================================================================================================*/
void nand_init(void)
{
	//Config
	*SysAddr16(NFCONF) = 0X377E;
	
	//Control
	*SysAddr16(NFCTRL) = 0x0061;
	   
	//Reset   
	nand_wait(); 
	
	*SysAddr16(NFCMD) = 0xFF;
	
	nand_wait();   
}



/*=================================================================================================
FUNCTION: nand_read_id()          

DESCRIPTION: 
	Used to read the flash ID.
   
ARGUMENTS PASSED:
	None
			
RETURN VALUE:
	U32		id 			- Read flash ID.
==================================================================================================*/
U32 nand_read_id(void)
{	
	U8 id1;
	U8 id2;
	   
	//Read ID  
	nand_wait();
	 
	*SysAddr16(NFCMD) = 0x90;   
	
	*SysAddr16(NFADDR) = 0x00;   
	
	nand_wait();   
	
	id1 = *SysAddr8(NFDATA);   
	id2 = *SysAddr8(NFDATA);

	return ((id2<<16) | id1);
}



/*=================================================================================================
FUNCTION: nand_read_page()          

DESCRIPTION: 
	Used to read one page.
   
ARGUMENTS PASSED:
	U32		blockidx		- [i] Block index/number for read operation.
	U32  		pageidx		- [i] Page  index/number for read operation.
			
RETURN VALUE:
	None
==================================================================================================*/
void nand_read_page(U32 blockidx, U32 pageidx)
{
	int i;
	U32 blkpgnum;
	U32 *ptr32;	
	U8  data[4];
	
	ptr32 = (U32*)data;
	blkpgnum = blockidx*NAND_BLOCK_PAGE_NUM + pageidx;
	
	//Read a page
	nand_wait();
	
	*SysAddr16(NFCMD)  = 0x00;
	
	*SysAddr16(NFADDR) = 0x00;
	*SysAddr16(NFADDR) = 0x00;
	*SysAddr16(NFADDR) = (blkpgnum>>0)  & 0xFF;
	*SysAddr16(NFADDR) = (blkpgnum>>8)  & 0xFF;
	*SysAddr16(NFADDR) = (blkpgnum>>16) & 0xFF;
	
	*SysAddr16(NFCMD)  = 0x30;
	
	nand_wait();
		
	for(i = 0; i < NAND_PAGE_SIZE; i += 4){
		data[0] = *SysAddr8(NFDATA);
		data[1] = *SysAddr8(NFDATA);
		data[2] = *SysAddr8(NFDATA);
		data[3] = *SysAddr8(NFDATA);
		write_to_host(*ptr32);	
	}
}



/*=================================================================================================
FUNCTION: nand_program_page()          

DESCRIPTION: 
	Used to program one page.
   
ARGUMENTS PASSED:
	U32		blockidx		- [i] Block index/number for program operation.
	U32 		pageidx		- [i] Page  index/number for program operation.
			
RETURN VALUE:
	U32		0			- Program the page successfully.
			1			- Program the page unsuccessfully.
==================================================================================================*/
U32 nand_program_page(U32 blockidx, U32 pageidx)
{
	int i;
	U32 blkpgnum;
	U32 *ptr32;	
	U8  data[4];

	ptr32 = (U32*)data;
	blkpgnum = blockidx*NAND_BLOCK_PAGE_NUM + pageidx;	
	
	//Program a page
	nand_wait();
	
	*SysAddr16(NFCMD) = 0x80;
	
	*SysAddr16(NFADDR) = 0x00;
	*SysAddr16(NFADDR) = 0x00;
	*SysAddr16(NFADDR) = (blkpgnum>>0)  & 0xFF;
	*SysAddr16(NFADDR) = (blkpgnum>>8)  & 0xFF;
	*SysAddr16(NFADDR) = (blkpgnum>>16) & 0xFF;

	for(i = 0; i < NAND_PAGE_SIZE; i += 4){
		*ptr32 = read_from_host();
		*SysAddr8(NFDATA) = data[0];
		*SysAddr8(NFDATA) = data[1];
		*SysAddr8(NFDATA) = data[2];
		*SysAddr8(NFDATA) = data[3];
	}

	*SysAddr16(NFCMD) = 0x10;
	
	nand_wait();
	
	//Read status
	*SysAddr16(NFCMD) = 0x70;
	
	nand_wait();
	
	if(*SysAddr8(NFDATA) & 0x1)
		return 1;	
	
	return 0;
}



/*=================================================================================================
FUNCTION: nand_erase_block()          

DESCRIPTION: 
	Used to erase one block. If can't erase, mark as bad/invalid block.
   
ARGUMENTS PASSED:
	U32		blockidx		- [i] Block index/number for erase operation.
			
RETURN VALUE:
	U32		0			- Erase the block successfully.
			1			- Erase the block unsuccessfully.
==================================================================================================*/
U32 nand_erase_block(U32 blockidx)
{
	U32 blkpgnum;
	
	blkpgnum = blockidx * NAND_BLOCK_PAGE_NUM;
	
	//Erase a block
	nand_wait();
	
	*SysAddr16(NFCMD) = 0x60;
	
	*SysAddr16(NFADDR) = (blkpgnum>>0)  & 0xFF;
	*SysAddr16(NFADDR) = (blkpgnum>>8)  & 0xFF;
	*SysAddr16(NFADDR) = (blkpgnum>>16) & 0xFF;
	
	*SysAddr16(NFCMD) = 0xD0;

	nand_wait();
	
	//Read status
	*SysAddr16(NFCMD) = 0x70;
	
	nand_wait();
	
	if(*SysAddr8(NFDATA) & 0x1){
		nand_mark_badblock(blockidx);
		return 1;
	}
	
	return 0;
}



/*=================================================================================================
FUNCTION: nand_check_blank()          

DESCRIPTION: 
	Used to check if a block is blank or not.
   
ARGUMENTS PASSED:
	U32		blockidx		- [i] Block index/number for blank check operation.
			
RETURN VALUE:
	U32		0			- The specified block is blank.
			1			- The specified block is NOT blank.
==================================================================================================*/
/*
U32 nand_check_blank(U32 blockidx)
{
	int i;
	U32 pageidx;
	U32 blkpgnum;
	U32 *ptr32;	
	U8  data[4];
	
	ptr32 = (U32*)data;
	blkpgnum = blockidx*NAND_BLOCK_PAGE_NUM;
	
	//Read a page
	for(pageidx = 0; pageidx < NAND_BLOCK_PAGE_NUM; pageidx += 1){

		nand_wait();
		
		*SysAddr16(NFCMD)  = 0x00;
		
		*SysAddr16(NFADDR) = 0x00;
		*SysAddr16(NFADDR) = 0x00;
		*SysAddr16(NFADDR) = (blkpgnum>>0)  & 0xFF;
		*SysAddr16(NFADDR) = (blkpgnum>>8)  & 0xFF;
		*SysAddr16(NFADDR) = (blkpgnum>>16) & 0xFF;
		
		*SysAddr16(NFCMD)  = 0x30;
		
		nand_wait();
			
		for(i = 0; i < NAND_PAGE_SIZE; i += 4){
			data[0] = *SysAddr8(NFDATA);
			data[1] = *SysAddr8(NFDATA);
			data[2] = *SysAddr8(NFDATA);
			data[3] = *SysAddr8(NFDATA);
			
			if(*ptr32 != 0xFFFFFFFF)
				return 1;
		}
		
		blkpgnum += 1;
	}
	
	return 0;
}
*/

/*=================================================================================================
FUNCTION: nand_mark_badblock()          

DESCRIPTION: 
	Used to mark a bad/invalid block.
   
ARGUMENTS PASSED:
	U32		blockidx		- [i] Block index/number to be marked.
			
RETURN VALUE:
	U32		0			-  Mark successfully.
			1			-  Mark unsuccessfully.
=================================================================================================*/

U32  nand_mark_badblock(U32 blockidx)
{
	U32 blkpgnum;

	blkpgnum = blockidx*NAND_BLOCK_PAGE_NUM;
	
	//Program the 1th byte of the sprae array in the first page to NON-0xFF
	nand_wait();
	
	*SysAddr16(NFCMD) = 0x80;
	
	*SysAddr16(NFADDR) = (2048>>0) & 0xFF;
	*SysAddr16(NFADDR) = (2048>>8) & 0xFF;
	*SysAddr16(NFADDR) = (blkpgnum>>0)   & 0xFF;
	*SysAddr16(NFADDR) = (blkpgnum>>8)   & 0xFF;
	*SysAddr16(NFADDR) = (blkpgnum>>16)  & 0xFF;
	
	*SysAddr8(NFDATA) = 0x00;	//MARKER NON-0xFF
	
	*SysAddr16(NFCMD) = 0x10;
	
	nand_wait();
	
	//Read status
	*SysAddr16(NFCMD) = 0x70;
	
	nand_wait();
	
	if(*SysAddr8(NFDATA) & 0x1)
		return 1;	
	
	return 0;
}



/*=================================================================================================
FUNCTION: nand_is_badblock()          

DESCRIPTION: 
	Used to check if a block is bad/invalid or not. 
   
ARGUMENTS PASSED:
	U32		blockidx		- [i] Block index/number to be checked.
			
RETURN VALUE:
	U32		0			- The specified block is good.
			1			- The specified block is bad/invalid
==================================================================================================*/
U32  nand_is_badblock(U32 blockidx)
{
	U8  data;
	U32 blkpgnum;

	blkpgnum = blockidx*NAND_BLOCK_PAGE_NUM;
	
	//Read  the 1th byte of the sprae array in the first page.
	nand_wait();
	
	*SysAddr16(NFCMD)  = 0x00;
	
	*SysAddr16(NFADDR) = (2048>>0) & 0xFF;
	*SysAddr16(NFADDR) = (2048>>8) & 0xFF;
	*SysAddr16(NFADDR) = (blkpgnum>>0)  & 0xFF;
	*SysAddr16(NFADDR) = (blkpgnum>>8)  & 0xFF;
	*SysAddr16(NFADDR) = (blkpgnum>>16) & 0xFF;
	
	*SysAddr16(NFCMD)  = 0x30;
	
	nand_wait();

	data = *SysAddr8(NFDATA);

	if(data != 0xFF)
		return 1;
	else
		return 0;
}
/*
*/


/*=================================================================================================
FUNCTION: nand_info_table()          

DESCRIPTION: 
	Used to receive the bad block table and relocation table. 
	User can modify this function as required.
   
ARGUMENTS PASSED:
	None
			
RETURN VALUE:
	U32		0			- Receive and process successfully.
			1			- Error
=================================================================================================*/
/*
U32 nand_info_table(void)
{
	U32 i;
	U32 num;
	U32 idx;
	U32 from;
	U32 to;
	
	//Bad Block Table
	num = read_from_host();	
	for(i = 0; i < num; i++)
		idx = read_from_host();	
				
	//Relocation Table	
	num = read_from_host();	
	for(i = 0; i < num; i++){
		from = read_from_host();		
		to   = read_from_host();	
	}

	return 0;
}
*/
