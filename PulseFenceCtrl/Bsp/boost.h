#ifndef __BOOST_H
#define __BOOST_H

#include "main.h"
#include "stm32f1xx_hal.h"
#include "uart.h"

#define HIGH_VOLTAGE_MODE		1			//高压模式
#define LOW_VOLTAGE_MODE		0			//低压模式

#define SINGLE_ZONE					1
#define DOUBLE_ZONE 				2

#define BOOST_IN_START			1			//正在升压状态
#define BOOST_IN_STOP				0			//停止升压状态
#define ZONE1								1
#define ZONE2								2

#define MAX_FILTER_QUANTITY			8

typedef enum
{
	NORMAL_STA = 0,
	P_BROKEN_STA,
	N_BROKEN_STA,
	PN_BROKEN_STA,
	SHORT_STA,			
	TOUCH_NET_STA,
	BYPASS_STA
}alarm_sta_typedef;

extern uint8_t arming_sta;
extern uint8_t zone_mode;
extern uint8_t protection_level;
extern uint8_t touch_net_mode;

extern uint8_t boost_delay_cnt;
extern uint8_t release_pulse_time_cnt;
extern uint16_t zone1_alarm_delay_cnt;
extern uint16_t zone2_alarm_delay_cnt;
//extern uint8_t broken_short_detect_time_cnt;

extern uint8_t targe_delay_flag;
extern uint32_t targe_delay_time;	//触发延时
extern uint32_t targe_delay_time_cnt;

extern uint8_t boost_finish_flag;
extern uint8_t boost_delay_finish_mask;
extern uint8_t release_pulse_finish_mask;
extern uint8_t zone1_alarm_delay_finish_mask;
extern uint8_t zone2_alarm_delay_finish_mask;

extern uint16_t broken_short_a1_line_cnt;
extern uint16_t broken_short_a2_line_cnt;
extern uint16_t broken_short_a3_line_cnt;
extern uint16_t broken_short_a4_line_cnt;
extern uint16_t broken_short_b1_line_cnt;
extern uint16_t broken_short_b2_line_cnt;
extern uint16_t broken_short_b3_line_cnt;
extern uint16_t broken_short_b4_line_cnt;

extern uint8_t zone1_touch_net_cnt;	//触网计数
extern uint8_t zone2_touch_net_cnt;	//触网计数

extern uint16_t zone1_alarm_delay_time;
extern uint16_t zone2_alarm_delay_time;

extern float zone1_high_max_normal_voltage;
extern float zone1_high_min_normal_voltage;

extern float zone1_low_max_normal_voltage;
extern float zone1_low_min_normal_voltage;

extern float zone2_high_max_normal_voltage;
extern float zone2_high_min_normal_voltage;

extern float zone2_low_max_normal_voltage;
extern float zone2_low_min_normal_voltage;

extern uint8_t auto_detect_sta;

void start_primary_boost(void);
void stop_primary_boost(void);
void primary_boost(uint8_t level);
void ralease_pulse(uint8_t zone_num);
void boost_release(void);
void arming_disarm(uint8_t sta);
void set_sensitivity(uint8_t zone_num, uint8_t level);
void broken_detect(uint8_t zone_num);
void alarm_inquire(void);
void touch_net_dectec(uint8_t zone_num);
void auto_dectect(void);

#endif /* __BOOST_H */
