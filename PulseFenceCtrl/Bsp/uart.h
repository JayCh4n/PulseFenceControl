#ifndef __UART_H
#define __UART_H

#include "main.h"
#include "stm32f1xx_hal.h"
#include "boost.h"
#include "flash.h"

#define AMING_DISARM				0x01			//布防/撤防命令   		数据：0x00 撤防   0x01布防
#define ZONE_TYPE	0x02								//单/双防区命令				数据：0x01 单			0x02双
#define HIGH_LOW_VOLTAGE		0x03			//高/低压模式命令			数据：0x00 低压   0x01高压
#define SENSITIVITY		0x04						//防区1灵敏度命令			数据：0x01 - 0x03	对应1——3等级灵敏度
//#define ZONE2_SENSITIVITY		0x05		//防区2灵敏度命令			数据：0x01 - 0x03	对应1——3等级灵敏度
#define ZONE_MODE						0x05			//防区模式						数据:	0x00 脉冲   0x01触网
#define AUTO_DETECT					0x06			//自动检测命令				数据: 0x01 ：开始自动检测（接收）	0x02:自动检测完成（发送）
#define TARGE_DELAY					0x07			//触发延时时间				数据:	时间    单位：秒

#define PULSE_MODE			0
#define TOUCH_NET_MODE 	1

#define ZONE1_STA		0x11	//防区1状态返回命令	 	数据: 0x00 正常 0x01 断线 0x02短路 0x03触网
#define ZONE2_STA 	0x12	//防区2状态返回命令		数据: 0x00 正常 0x01 断线 0x02短路 0x03触网


//#define BROKEN_STA			0x01		//断线状态
//#define SHORT_STA				0x02		//短路状态
//#define	TOUCH_NET_STA		0x03		//触网状态

//#define ZONE1_BROKEN_STA  			0x11			//防区1断线状态命令					数据：0x00 正常 0x01：断线
//#define ZONE1_SHORT_STA					0x12			//防区1短路状态命令					数据：0x00 正常 0x01：短路
//#define ZONE1_BYPASS_STA				0x13			//防区1旁路状态							数据：0x00 正常 0x01：旁路
//#define ZONE1_TOUCH_NET_STA			0x14			//防区1触网状态							数据：0x00 正常 0x01：触网

#define ZONE2_BROKEN_STA 			  0x21			//防区2断线状态命令					数据：0x00 正常 0x01: 断线
#define ZONE2_SHORT_STA					0x22			//防区2短路状态命令					数据：0x00 正常 0x01：短路
#define	ZONE2_BYPASS_STA				0x23			//防区2旁路状态							数据：0x00 正常 0x01：旁路
#define ZONE2_TOUCH_NET_STA			0x24			//防区2触网状态							数据：0x00 正常 0x01：触网

extern uint8_t uart1_rx_data[50];
extern uint8_t uart1_rx_buff;

void uart1_deal(uint8_t *data_package);
void return_set_msg(uint8_t cmd, uint8_t zone_num, uint8_t sta);
//void send_sta_msg(uint8_t sta_type, uint8_t sta);
void get_init_value(uint8_t *data_package);
void return_sta_msg(void);
#endif /* __UART_H */
