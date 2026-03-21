#include "drv_flash.h"
#include "sys_app.h"
#include "app_common.h"

u64 STMFLASH_ReadDoubleWord(u32 faddr)
{
	return *(u64*)faddr;
}

void STMFLASH_Read(u32 ReadAddr,u64 *pBuffer,u16 Num64bitToRead)
{
	u16 i;
	//APP_PRINTF("STMFLASH_Read add=%x,len =%d \r\n",ReadAddr,Num64bitToRead);
	for(i=0;i<Num64bitToRead;i++)
	{
		pBuffer[i]=STMFLASH_ReadDoubleWord(ReadAddr);
		ReadAddr+=8;
	}
}

void STMFLASH_Write_NoCheck(u32 WriteAddr,u64 *pBuffer,u16 Num64bitToWrite)
{
	u16 i;
	uint64_t val;
	HAL_StatusTypeDef status;
	uint32_t err = 0;
	APP_PRINTF("STMFLASH_Write_NoCheck Addr=%x,dblwordlen=%d\n",WriteAddr,Num64bitToWrite);

	for(i=0;i<Num64bitToWrite;i++)
	{
		val = *pBuffer++;
		HAL_FLASH_Unlock();
		status = HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD,WriteAddr,val);
		HAL_FLASH_Lock();
	    if(status != HAL_OK)
	    {
	    	err = HAL_FLASH_GetError();  //HAL_FLASH_ERROR_PROG
	    	HAL_FLASH_Unlock();
	    	FLASH_WaitForLastOperation(1000);
	    	status = HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, WriteAddr, val);
	    	HAL_FLASH_Lock();
	    	APP_PRINTF("prog at addr:0x%x status=%d\r\n err=0x%x\r\n",WriteAddr,status,err);
	    }
	    WriteAddr+=8;
	}
}


void STMFLASH_Write(u32 WriteAddr, u64 *pBuffer, u16 Num64bitToWrite)
{
	u32 secpos;
	u32 offaddr;

	if (WriteAddr < STM32_FLASH_BASE || (WriteAddr >= (STM32_FLASH_BASE+1024*STM32_FLASH_SIZE)))
		return;

	if (Num64bitToWrite > 32) // Limit this to a single 2K sector
		return;

	offaddr = WriteAddr - STM32_FLASH_BASE;
	secpos = offaddr / STM_SECTOR_SIZE;

	APP_PRINTF("STMFLASH_Write ADDR=0x%x secpos %d \r\n", WriteAddr, secpos);

	HAL_FLASH_Unlock();
	__HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_SR_ERRORS);
	FLASH_PageErase(secpos);//erase entire page
	OS_DELAY_MS(20);
	CLEAR_BIT(FLASH->CR, (FLASH_CR_PER | FLASH_CR_PNB));
	HAL_FLASH_Lock();

	STMFLASH_Write_NoCheck(WriteAddr, pBuffer, Num64bitToWrite);
}

// u64 STMFLASH_BUF[STM_SECTOR_SIZE/8];//最多是2K字节

// void STMFLASH_Write(u32 WriteAddr,u64 *pBuffer,u16 Num64bitToWrite)
// {
// 	u32 secpos;	   //扇区地址
// 	u16 secoff;	   //扇区内偏移地址(16位字计算)
// 	u16 secremain; //扇区内剩余地址(16位字计算)
//  	u16 i;
// 	u32 offaddr;   //去掉0X08000000后的地址
// 	if(WriteAddr<STM32_FLASH_BASE||(WriteAddr>=(STM32_FLASH_BASE+1024*STM32_FLASH_SIZE)))return;//非法地址
// 						//解锁
// 	offaddr=WriteAddr-STM32_FLASH_BASE;		//实际偏移地址.
// 	secpos=offaddr/STM_SECTOR_SIZE;			//扇区地址
// 	secoff=(offaddr%STM_SECTOR_SIZE)/8;		//在扇区内的偏移(8个字节为基本单位.)
// 	secremain=STM_SECTOR_SIZE/8-secoff;		//扇区剩余空间大小
// 	if(Num64bitToWrite<=secremain)secremain=Num64bitToWrite;//不大于该扇区范围
// 	//APP_PRINTF("STMFLASH_Write ADDR=0x%x secpos %d secoff %d secremain %d \r\n",WriteAddr,secpos,secoff,secremain);
// 	while(1)
// 	{
// // 		STMFLASH_Read(secpos*STM_SECTOR_SIZE+STM32_FLASH_BASE,STMFLASH_BUF,STM_SECTOR_SIZE/8);//读出整个扇区的内容
// //		for(i=0;i<secremain;i++)//校验数据
// //		{
// //			if(STMFLASH_BUF[secoff+i]!=0xFFFFFFFFFFFFFFFF)break;//需要擦除
// //		}
// 		i = 0; //erase each time
// 		if(i<secremain)//需要擦除
// 		{
// 			HAL_FLASH_Unlock();
// 			__HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_SR_ERRORS);
// 			FLASH_PageErase(secpos);//erase page
// 			OS_DELAY_MS(20);
// 			CLEAR_BIT(FLASH->CR, (FLASH_CR_PER | FLASH_CR_PNB));
// 			HAL_FLASH_Lock();
// 			for(i=0;i<secremain;i++)//复制
// 			{
// 				STMFLASH_BUF[i+secoff]=pBuffer[i];
// //				uint32_t *pData = (uint32_t*)&pBuffer[i];
// //				APP_LOG(TS_ON,VLEVEL_M,"idx %d 0x%x-0x%x\r\n",i,*pData,*(pData+1));
// //				if(i%5==0)
// //				{
// //					OS_DELAY_MS(10);
// //				}
// 			}
// 			STMFLASH_Write_NoCheck(WriteAddr,pBuffer,secremain);//写已经擦除了的,直接写入扇区剩余区间.
// 			//STMFLASH_Write_NoCheck(secpos*STM_SECTOR_SIZE+STM32_FLASH_BASE,STMFLASH_BUF,STM_SECTOR_SIZE/8);//写入整个扇区
// 		}else
// 			STMFLASH_Write_NoCheck(WriteAddr,pBuffer,secremain);//写已经擦除了的,直接写入扇区剩余区间.

// 		if(Num64bitToWrite==secremain)break;//写入结束了
// 		else//写入未结束
// 		{
// 			secpos++;				//扇区地址增1
// 			secoff=0;				//偏移位置为0
// 		   	pBuffer+=secremain;  	//指针偏移
// 			WriteAddr+=secremain;	//写地址偏移
// 			Num64bitToWrite-=secremain;	//字节(16位)数递减
// 			if(Num64bitToWrite>(STM_SECTOR_SIZE/8))secremain=STM_SECTOR_SIZE/8;//下一个扇区还是写不完
// 			else secremain=Num64bitToWrite;//下一个扇区可以写完了
// 		}
// 	};
// 	//HAL_FLASH_Lock();//上锁
// }
