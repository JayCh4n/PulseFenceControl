#include "flash.h"

flash_data_typedef flash_data_struct;
uint8_t write_flash_flag = 0;

void read_data_from_flash(void)
{
	if(*(__IO uint8_t*)(PAGE_ADDR) != 0x01)
	{
		flash_data_struct.flash_first_start = 0x01;

		HAL_Delay(1000);
		STMFLASH_Write(PAGE_ADDR, (uint16_t *)&flash_data_struct, FLASH_DATA_SIZE);
	}
	else
	{
		STMFLASH_Read(PAGE_ADDR, (uint16_t *)&flash_data_struct,FLASH_DATA_SIZE);

		zone1_high_max_normal_voltage = flash_data_struct.flash_zone1_high_max_normal_voltage;
		zone1_high_min_normal_voltage = flash_data_struct.flash_zone1_high_min_normal_voltage;
		zone1_low_max_normal_voltage = flash_data_struct.flash_zone1_low_max_normal_voltage;
		zone1_low_min_normal_voltage = flash_data_struct.flash_zone1_low_min_normal_voltage;
		zone2_high_max_normal_voltage = flash_data_struct.flash_zone2_high_max_normal_voltage;
		zone2_high_min_normal_voltage = flash_data_struct.flash_zone2_high_min_normal_voltage;
		zone2_low_max_normal_voltage = flash_data_struct.flash_zone2_low_max_normal_voltage;
		zone2_low_min_normal_voltage = flash_data_struct.flash_zone2_low_min_normal_voltage;
	}
}

//void flash_read(uint32_t page_addr,uint16_t *p_data, uint16_t data_lenth)
//{
//	uint16_t i = 0;

//	do
//	{
//		*(p_data+(i/2)) = *(__IO uint16_t*)(page_addr+i);
//	}while((i+=2) < data_lenth);
//}

//void flash_write(uint32_t page_addr,uint16_t *p_data, uint16_t data_lenth)
//{
//	uint16_t i = 0;
//	FLASH_EraseInitTypeDef EraseInit_Struct;
//	uint32_t page_error;
//	uint32_t addr;
//	uint16_t *data;
//	
//	if(HAL_FLASH_Unlock() != HAL_OK)
//	{
//		return;
//	}
//	
//	EraseInit_Struct.TypeErase = FLASH_TYPEERASE_PAGES;
//	EraseInit_Struct.PageAddress = page_addr;
//	EraseInit_Struct.NbPages = 1;
//	
//	HAL_FLASHEx_Erase(&EraseInit_Struct, &page_error);
//	
//	do
//	{
//		addr = page_addr + i;
//		data = p_data + (i/2);
//		HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, addr, *data);
//	}while((i+=2) < data_lenth);
//	
//	HAL_FLASH_Lock();
//}


//����STM32��FLASH
void STMFLASH_Unlock(void)
{
	FLASH->KEYR=FLASH_KEY1;//д���������.
	FLASH->KEYR=FLASH_KEY2;
}
//flash����
void STMFLASH_Lock(void)
{
	FLASH->CR|=1<<7;//����
}
//�õ�FLASH״̬
uint8_t STMFLASH_GetStatus(void)
{	
	uint32_t res;		
	res=FLASH->SR; 
	if(res&(1<<0))return 1;		    //æ
	else if(res&(1<<2))return 2;	//��̴���
	else if(res&(1<<4))return 3;	//д��������
	return 0;						//�������
}
//�ȴ��������
//time:Ҫ��ʱ�ĳ���
//����ֵ:״̬.
uint8_t STMFLASH_WaitDone(uint16_t time)
{
	uint8_t res;
	do
	{
		res=STMFLASH_GetStatus();
		if(res!=1)break;//��æ,����ȴ���,ֱ���˳�.
		HAL_Delay(1);
		time--;
	 }while(time);
	 if(time==0)res=0xff;//TIMEOUT
	 return res;
}
//����ҳ
//paddr:ҳ��ַ
//����ֵ:ִ�����
uint8_t STMFLASH_ErasePage(uint32_t paddr)
{
	uint8_t res=0;
	res=STMFLASH_WaitDone(20);//�ȴ��ϴβ�������,>20ms    
	if(res==0)
	{ 
		FLASH->CR|=1<<1;//ҳ����
		FLASH->AR=paddr;//����ҳ��ַ 
		FLASH->CR|=1<<6;//��ʼ����		  
		res=STMFLASH_WaitDone(20);//�ȴ���������,>20ms  
		if(res!=1)//��æ
		{
			FLASH->CR&=~(1<<1);//���ҳ������־.
		}
	}
	return res;
}
//��FLASHָ����ַд�����
//faddr:ָ����ַ(�˵�ַ����Ϊ2�ı���!!)
//dat:Ҫд�������
//����ֵ:д������
uint8_t STMFLASH_WriteHalfWord(uint32_t faddr, uint16_t dat)
{
	uint8_t res;	   	    
	res=STMFLASH_WaitDone(0XFF);	 
	if(res==0)//OK
	{
		FLASH->CR|=1<<0;		//���ʹ��
		*(__IO uint16_t*)faddr=dat;		//д������
		res=STMFLASH_WaitDone(0XFF);//�ȴ��������
		if(res!=1)//�����ɹ�
		{
			FLASH->CR&=~(1<<0);	//���PGλ.
		}
	} 
	return res;
} 
//��ȡָ����ַ�İ���(16λ����) 
//faddr:����ַ 
//����ֵ:��Ӧ����.
uint16_t STMFLASH_ReadHalfWord(uint32_t faddr)
{
	return *(__IO uint16_t*)faddr; 
}
#if STM32_FLASH_WREN	//���ʹ����д   
//������д��
//WriteAddr:��ʼ��ַ
//pBuffer:����ָ��
//NumToWrite:����(16λ)��   
void STMFLASH_Write_NoCheck(uint32_t WriteAddr,uint16_t *pBuffer,uint16_t NumToWrite)   
{ 			 		 
	uint16_t i;
	for(i=0;i<NumToWrite;i++)
	{
		STMFLASH_WriteHalfWord(WriteAddr,pBuffer[i]);
	    WriteAddr+=2;//��ַ����2.
	}  
} 
//��ָ����ַ��ʼд��ָ�����ȵ�����
//WriteAddr:��ʼ��ַ(�˵�ַ����Ϊ2�ı���!!)
//pBuffer:����ָ��
//NumToWrite:����(16λ)��(����Ҫд���16λ���ݵĸ���.)
#if STM32_FLASH_SIZE<256
#define STM_SECTOR_SIZE 1024 //�ֽ�
#else 
#define STM_SECTOR_SIZE	2048
#endif		 
uint16_t STMFLASH_BUF[STM_SECTOR_SIZE/2];//�����2K�ֽ�
void STMFLASH_Write(uint32_t WriteAddr,uint16_t *pBuffer,uint16_t NumToWrite)	
{
	uint32_t secpos;	   //������ַ
	uint16_t secoff;	   //������ƫ�Ƶ�ַ(16λ�ּ���)
	uint16_t secremain; //������ʣ���ַ(16λ�ּ���)	   
 	uint16_t i;    
	uint32_t offaddr;   //ȥ��0X08000000��ĵ�ַ
	if(WriteAddr<STM32_FLASH_BASE||(WriteAddr>=(STM32_FLASH_BASE+1024*STM32_FLASH_SIZE)))return;//�Ƿ���ַ
	STMFLASH_Unlock();						//����
	offaddr=WriteAddr-STM32_FLASH_BASE;		//ʵ��ƫ�Ƶ�ַ.
	secpos=offaddr/STM_SECTOR_SIZE;			//������ַ  0~127 for STM32F103RBT6
	secoff=(offaddr%STM_SECTOR_SIZE)/2;		//�������ڵ�ƫ��(2���ֽ�Ϊ������λ.)
	secremain=STM_SECTOR_SIZE/2-secoff;		//����ʣ��ռ��С   
	if(NumToWrite<=secremain)secremain=NumToWrite;//�����ڸ�������Χ
	while(1) 
	{	
		STMFLASH_Read(secpos*STM_SECTOR_SIZE+STM32_FLASH_BASE,STMFLASH_BUF,STM_SECTOR_SIZE/2);//������������������
		for(i=0;i<secremain;i++)	//У������
		{
			if(STMFLASH_BUF[secoff+i]!=0XFFFF)break;//��Ҫ����  	  
		}
		if(i<secremain)				//��Ҫ����
		{
			STMFLASH_ErasePage(secpos*STM_SECTOR_SIZE+STM32_FLASH_BASE);//�����������
			for(i=0;i<secremain;i++)//����
			{
				STMFLASH_BUF[i+secoff]=pBuffer[i];	  
			}
			STMFLASH_Write_NoCheck(secpos*STM_SECTOR_SIZE+STM32_FLASH_BASE,STMFLASH_BUF,STM_SECTOR_SIZE/2);//д����������  
		}else STMFLASH_Write_NoCheck(WriteAddr,pBuffer,secremain);//д�Ѿ������˵�,ֱ��д������ʣ������. 				   
		if(NumToWrite==secremain)break;//д�������
		else//д��δ����
		{
			secpos++;				//������ַ��1
			secoff=0;				//ƫ��λ��Ϊ0 	 
		   	pBuffer+=secremain;  	//ָ��ƫ��
			WriteAddr+=secremain*2;	//д��ַƫ��(16λ���ݵ�ַ,��Ҫ*2)	   
		   	NumToWrite-=secremain;	//�ֽ�(16λ)���ݼ�
			if(NumToWrite>(STM_SECTOR_SIZE/2))secremain=STM_SECTOR_SIZE/2;//��һ����������д����
			else secremain=NumToWrite;//��һ����������д����
		}	 
	};	
	STMFLASH_Lock();//����
}
#endif

//��ָ����ַ��ʼ����ָ�����ȵ�����
//ReadAddr:��ʼ��ַ
//pBuffer:����ָ��
//NumToWrite:����(16λ)��
void STMFLASH_Read(uint32_t ReadAddr,uint16_t *pBuffer,uint16_t NumToRead)   	
{
	uint16_t i;
	for(i=0;i<NumToRead;i++)
	{
		pBuffer[i]=STMFLASH_ReadHalfWord(ReadAddr);//��ȡ2���ֽ�.
		ReadAddr+=2;//ƫ��2���ֽ�.	
	}
}













