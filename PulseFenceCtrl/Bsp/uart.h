#ifndef __UART_H
#define __UART_H

#include "main.h"
#include "stm32f1xx_hal.h"
#include "boost.h"
#include "flash.h"

#define AMING_DISARM				0x01			//����/��������   		���ݣ�0x00 ����   0x01����
#define ZONE_TYPE	0x02								//��/˫��������				���ݣ�0x01 ��			0x02˫
#define HIGH_LOW_VOLTAGE		0x03			//��/��ѹģʽ����			���ݣ�0x00 ��ѹ   0x01��ѹ
#define SENSITIVITY		0x04						//����1����������			���ݣ�0x01 - 0x03	��Ӧ1����3�ȼ�������
//#define ZONE2_SENSITIVITY		0x05		//����2����������			���ݣ�0x01 - 0x03	��Ӧ1����3�ȼ�������
#define ZONE_MODE						0x05			//����ģʽ						����:	0x00 ����   0x01����
#define AUTO_DETECT					0x06			//�Զ��������				����: 0x01 ����ʼ�Զ���⣨���գ�	0x02:�Զ������ɣ����ͣ�
#define TARGE_DELAY					0x07			//������ʱʱ��				����:	ʱ��    ��λ����

#define PULSE_MODE			0
#define TOUCH_NET_MODE 	1

#define ZONE1_STA		0x11	//����1״̬��������	 	����: 0x00 ���� 0x01 ���� 0x02��· 0x03����
#define ZONE2_STA 	0x12	//����2״̬��������		����: 0x00 ���� 0x01 ���� 0x02��· 0x03����


//#define BROKEN_STA			0x01		//����״̬
//#define SHORT_STA				0x02		//��·״̬
//#define	TOUCH_NET_STA		0x03		//����״̬

//#define ZONE1_BROKEN_STA  			0x11			//����1����״̬����					���ݣ�0x00 ���� 0x01������
//#define ZONE1_SHORT_STA					0x12			//����1��·״̬����					���ݣ�0x00 ���� 0x01����·
//#define ZONE1_BYPASS_STA				0x13			//����1��·״̬							���ݣ�0x00 ���� 0x01����·
//#define ZONE1_TOUCH_NET_STA			0x14			//����1����״̬							���ݣ�0x00 ���� 0x01������

#define ZONE2_BROKEN_STA 			  0x21			//����2����״̬����					���ݣ�0x00 ���� 0x01: ����
#define ZONE2_SHORT_STA					0x22			//����2��·״̬����					���ݣ�0x00 ���� 0x01����·
#define	ZONE2_BYPASS_STA				0x23			//����2��·״̬							���ݣ�0x00 ���� 0x01����·
#define ZONE2_TOUCH_NET_STA			0x24			//����2����״̬							���ݣ�0x00 ���� 0x01������

extern uint8_t uart1_rx_data[50];
extern uint8_t uart1_rx_buff;

void uart1_deal(uint8_t *data_package);
void return_set_msg(uint8_t cmd, uint8_t zone_num, uint8_t sta);
//void send_sta_msg(uint8_t sta_type, uint8_t sta);
void get_init_value(uint8_t *data_package);
void return_sta_msg(void);
#endif /* __UART_H */
