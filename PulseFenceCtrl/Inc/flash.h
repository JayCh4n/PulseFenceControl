#ifndef __FLASH_H
#define __FLASH_H

#include "main.h"
#include "stm32f1xx_hal.h"
#include "boost.h"

typedef struct{
	uint8_t flash_first_start;
	float flash_zone1_high_max_normal_voltage;
	float flash_zone1_high_min_normal_voltage;
	float flash_zone1_low_max_normal_voltage;
	float flash_zone1_low_min_normal_voltage;
	float flash_zone2_high_max_normal_voltage;
	float flash_zone2_high_min_normal_voltage;
	float flash_zone2_low_max_normal_voltage;
	float flash_zone2_low_min_normal_voltage;
}flash_data_typedef;

#define TEXT_LENTH sizeof(flash_data_struct)	 		  		//数组长度
#define FLASH_DATA_SIZE TEXT_LENTH/2+((TEXT_LENTH%2)?1:0)

#define	PAGE_ADDR	((uint32_t)0x0801FC00)

//用户根据自己的需要设置
#define STM32_FLASH_SIZE 	128 	 		//所选STM32的FLASH容量大小(单位为K)
#define STM32_FLASH_WREN 	1              	//使能FLASH写入(0，不是能;1，使能)
//////////////////////////////////////////////////////////////////////////////////////////////////////

//FLASH起始地址
#define STM32_FLASH_BASE 0x08000000 		//STM32 FLASH的起始地址
//FLASH解锁键值

extern flash_data_typedef flash_data_struct;
extern uint8_t write_flash_flag;

void read_data_from_flash(void);

void STMFLASH_Unlock(void);					  //FLASH解锁
void STMFLASH_Lock(void);					  //FLASH上锁
uint8_t STMFLASH_GetStatus(void);				  //获得状态
uint8_t STMFLASH_WaitDone(uint16_t time);				  //等待操作结束
uint8_t STMFLASH_ErasePage(uint32_t paddr);			  //擦除页
uint8_t STMFLASH_WriteHalfWord(uint32_t faddr, uint16_t dat);//写入半字
uint16_t STMFLASH_ReadHalfWord(uint32_t faddr);		  //读出半字  
void STMFLASH_WriteLenByte(uint32_t WriteAddr,uint32_t DataToWrite,uint16_t Len);	//指定地址开始写入指定长度的数据
uint32_t STMFLASH_ReadLenByte(uint32_t ReadAddr,uint16_t Len);						//指定地址开始读取指定长度数据
void STMFLASH_Write(uint32_t WriteAddr,uint16_t *pBuffer,uint16_t NumToWrite);		//从指定地址开始写入指定长度的数据
void STMFLASH_Read(uint32_t ReadAddr,uint16_t *pBuffer,uint16_t NumToRead);   		//从指定地址开始读出指定长度的数据
void write_flash_process(void);










#endif /* __FLASH_H */









