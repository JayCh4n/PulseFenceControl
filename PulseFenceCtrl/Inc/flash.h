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

#define TEXT_LENTH sizeof(flash_data_struct)	 		  		//���鳤��
#define FLASH_DATA_SIZE TEXT_LENTH/2+((TEXT_LENTH%2)?1:0)

#define	PAGE_ADDR	((uint32_t)0x0801FC00)

//�û������Լ�����Ҫ����
#define STM32_FLASH_SIZE 	128 	 		//��ѡSTM32��FLASH������С(��λΪK)
#define STM32_FLASH_WREN 	1              	//ʹ��FLASHд��(0��������;1��ʹ��)
//////////////////////////////////////////////////////////////////////////////////////////////////////

//FLASH��ʼ��ַ
#define STM32_FLASH_BASE 0x08000000 		//STM32 FLASH����ʼ��ַ
//FLASH������ֵ

extern flash_data_typedef flash_data_struct;
extern uint8_t write_flash_flag;

void read_data_from_flash(void);

void STMFLASH_Unlock(void);					  //FLASH����
void STMFLASH_Lock(void);					  //FLASH����
uint8_t STMFLASH_GetStatus(void);				  //���״̬
uint8_t STMFLASH_WaitDone(uint16_t time);				  //�ȴ���������
uint8_t STMFLASH_ErasePage(uint32_t paddr);			  //����ҳ
uint8_t STMFLASH_WriteHalfWord(uint32_t faddr, uint16_t dat);//д�����
uint16_t STMFLASH_ReadHalfWord(uint32_t faddr);		  //��������  
void STMFLASH_WriteLenByte(uint32_t WriteAddr,uint32_t DataToWrite,uint16_t Len);	//ָ����ַ��ʼд��ָ�����ȵ�����
uint32_t STMFLASH_ReadLenByte(uint32_t ReadAddr,uint16_t Len);						//ָ����ַ��ʼ��ȡָ����������
void STMFLASH_Write(uint32_t WriteAddr,uint16_t *pBuffer,uint16_t NumToWrite);		//��ָ����ַ��ʼд��ָ�����ȵ�����
void STMFLASH_Read(uint32_t ReadAddr,uint16_t *pBuffer,uint16_t NumToRead);   		//��ָ����ַ��ʼ����ָ�����ȵ�����
void write_flash_process(void);










#endif /* __FLASH_H */









