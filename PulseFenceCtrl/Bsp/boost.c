#include "boost.h"

extern TIM_HandleTypeDef htim2;
extern TIM_HandleTypeDef htim1;
extern ADC_HandleTypeDef hadc1;

/* 默认：撤防 双防区 高压模式 */
uint8_t arming_sta = 0;							//布防状态 1：布防 0：撤防
uint8_t zone_mode = DOUBLE_ZONE;
uint8_t protection_level = HIGH_VOLTAGE_MODE;
uint8_t touch_net_mode = 0;  				//0:脉冲模式(不检测触网)   1:触网模式

uint8_t boost_delay_cnt = 0;
uint8_t release_pulse_time_cnt = 0;
uint16_t zone1_alarm_delay_cnt = 0;
uint16_t zone2_alarm_delay_cnt = 0;
//uint8_t broken_short_detect_time_cnt = 0;

uint8_t boost_finish_flag = 0;
uint8_t boost_delay_finish_mask = 0;
uint8_t release_pulse_finish_mask = 0;
uint8_t zone1_alarm_delay_finish_mask = 0;
uint8_t zone2_alarm_delay_finish_mask = 0;

uint16_t broken_short_a1_line_cnt = 0;
uint16_t broken_short_a2_line_cnt = 0;
uint16_t broken_short_a3_line_cnt = 0;
uint16_t broken_short_a4_line_cnt = 0;
uint16_t broken_short_b1_line_cnt = 0;
uint16_t broken_short_b2_line_cnt = 0;
uint16_t broken_short_b3_line_cnt = 0;
uint16_t broken_short_b4_line_cnt = 0;

uint16_t zone1_alarm_delay_time = 90;		// 根据灵敏度计算得出延时报警值  * 10ms  默认：灵敏度为3  3:900ms   2:2.5s 1:5s
uint16_t zone2_alarm_delay_time = 90;		

alarm_sta_typedef zone1_alarm_sta = NORMAL_STA;
alarm_sta_typedef zone2_alarm_sta = NORMAL_STA;

alarm_sta_typedef pre_zone1_alarm_sta = NORMAL_STA;
alarm_sta_typedef pre_zone2_alarm_sta = NORMAL_STA;

uint8_t zone1_short_mask = 0;	//短路屏蔽标志 区分两根主线被剪和短路两种情况（触网检测那里检测到电压 说明没有短路） 0：不屏蔽 1：屏蔽
uint8_t zone2_short_mask = 0;

float zone1_high_max_normal_voltage;
float zone1_high_min_normal_voltage;

float zone1_low_max_normal_voltage;
float zone1_low_min_normal_voltage;

float zone2_high_max_normal_voltage;
float zone2_high_min_normal_voltage;

float zone2_low_max_normal_voltage;
float zone2_low_min_normal_voltage;

uint8_t auto_detect_sta = 0;
/*开始前级升压*/
void start_primary_boost(void)
{
	HAL_GPIO_WritePin(PRIMARY_BOOST_ENABLE_PIN_GPIO_Port, PRIMARY_BOOST_ENABLE_PIN_Pin, GPIO_PIN_SET);	/*使能升压控制脚*/
	HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_2);	/*开启升压PWM*/
}

/*停止前级升压*/
void stop_primary_boost(void)
{
	HAL_GPIO_WritePin(PRIMARY_BOOST_ENABLE_PIN_GPIO_Port, PRIMARY_BOOST_ENABLE_PIN_Pin, GPIO_PIN_RESET); /*失能升压控制脚*/
	HAL_TIM_PWM_Stop(&htim2, TIM_CHANNEL_2);	/*停止升压PWM*/
}

/*升到设定电压 并停止*/
void primary_boost(uint8_t level)
{
//	uint32_t i = 0;
	start_primary_boost();	/*开始前级升压 */
	
	/*根据升压等级 等待升到设定等级对应电压*/
	if(level == LOW_VOLTAGE_MODE)
	{
		while(!HAL_GPIO_ReadPin(DETECT_LEVEL_LOW_GPIO_Port, DETECT_LEVEL_LOW_Pin));	
	}
	else if(level == HIGH_VOLTAGE_MODE)
	{
		while(!HAL_GPIO_ReadPin(DETECT_LEVEL_HIGH_GPIO_Port, DETECT_LEVEL_HIGH_Pin));	
	}
	//应加超时判断  防止烧坏电路
	
	stop_primary_boost(); /*停止前级升压*/
	
	//如果是自检模式 则为布防模式
	if(!auto_detect_sta)
	{
		boost_finish_flag = 1; /*升压完成标志置1*/
	}
}

/*释放脉冲*/
void ralease_pulse(uint8_t zone_num)
{
	if(zone_num == 1)
	{
		HAL_GPIO_WritePin(ZONE1_PULSE_RELEASE_PIN_GPIO_Port, ZONE1_PULSE_RELEASE_PIN_Pin, GPIO_PIN_RESET);
	}
	else if(zone_num == 2)
	{
		HAL_GPIO_WritePin(ZONE2_PULSE_RELEASE_PIN_GPIO_Port, ZONE2_PULSE_RELEASE_PIN_Pin, GPIO_PIN_RESET);	
	}
	
	HAL_Delay(1);
	touch_net_dectec(zone_num);
	
	while(!release_pulse_finish_mask);
	
	release_pulse_finish_mask = 0;
	boost_finish_flag = 0;
	
	if(zone_num == 1)
	{
		HAL_GPIO_WritePin(ZONE1_PULSE_RELEASE_PIN_GPIO_Port, ZONE1_PULSE_RELEASE_PIN_Pin, GPIO_PIN_SET);
	}
	else if(zone_num == 2)
	{
		HAL_GPIO_WritePin(ZONE2_PULSE_RELEASE_PIN_GPIO_Port, ZONE2_PULSE_RELEASE_PIN_Pin, GPIO_PIN_SET);	
	}
	
}

/*升压并且释放脉冲*/
void boost_release(void)
{
	if(boost_delay_finish_mask)
	{
		boost_delay_finish_mask = 0;
		primary_boost(protection_level);
		ralease_pulse(ZONE1);
		broken_detect(ZONE1);
		if(zone_mode == DOUBLE_ZONE)
		{
			primary_boost(protection_level);
			ralease_pulse(ZONE2);
			broken_detect(ZONE2);
		}
	}
}

/*布防、撤防*/
void arming_disarm(uint8_t sta)
{
	arming_sta = sta;
	
	/*如果撤防 清除计数值 和标志位*/
	if(!arming_sta)
	{
		boost_delay_cnt = 0;
		release_pulse_time_cnt = 0;
		zone1_alarm_delay_cnt = 0;
		zone2_alarm_delay_cnt = 0;
//		broken_short_detect_time_cnt = 0;

		boost_finish_flag = 0;
		boost_delay_finish_mask = 0;
		release_pulse_finish_mask = 0;
		zone1_alarm_delay_finish_mask = 0;
		zone2_alarm_delay_finish_mask = 0;
		
		broken_short_a1_line_cnt = 0;
		broken_short_a2_line_cnt = 0;
		broken_short_a3_line_cnt = 0;
		broken_short_a4_line_cnt = 0;
		broken_short_b1_line_cnt = 0;
		broken_short_b2_line_cnt = 0;
		broken_short_b3_line_cnt = 0;
		broken_short_b4_line_cnt = 0;
		
		zone1_alarm_sta = NORMAL_STA;
		pre_zone1_alarm_sta = NORMAL_STA;
		
		zone2_alarm_sta = NORMAL_STA;
		pre_zone2_alarm_sta = NORMAL_STA;
	}
}

/*设置灵敏度*/
void set_sensitivity(uint8_t zone_num, uint8_t level)
{
	if(zone_num == ZONE1)
	{
		switch(level)
		{
			case 1: zone1_alarm_delay_time = 500;	break;
			case 2: zone1_alarm_delay_time = 250; break;
			case 3: zone1_alarm_delay_time = 90; break;
			default: break;
		}
	}
	
	if(zone_num == ZONE2)
	{
		switch(level)
		{
			case 1: zone2_alarm_delay_time = 500; 	break;
			case 2: zone2_alarm_delay_time = 250; break;
			case 3: zone2_alarm_delay_time = 90; break;
			default: break;
		}		
	}
}

/*断线、短路检测*/
void broken_detect(uint8_t zone_num)
{

	if(zone_num == ZONE1)
	{
			if(broken_short_a1_line_cnt <= 1)
			{
				broken_short_a1_line_cnt = 0;
			}
			if(broken_short_a2_line_cnt <= 1)
			{
				broken_short_a2_line_cnt = 0;
			}
			if(broken_short_a3_line_cnt <= 1)
			{
				broken_short_a3_line_cnt = 0;
			}
			if(broken_short_a4_line_cnt <= 1)
			{
				broken_short_a4_line_cnt = 0;
			}
			
			/*如果有一条线或多条没有检测到 则计数值不会增加 所以相乘等于0  报警*/
			if(!(broken_short_a1_line_cnt * broken_short_a2_line_cnt * broken_short_a3_line_cnt * broken_short_a4_line_cnt))
			{
				/*如果所有线均没有返回脉冲 则所有线计数都为0 则计数为0 相加为0 短路报警 */
				if(!(broken_short_a1_line_cnt + broken_short_a2_line_cnt + broken_short_a3_line_cnt + broken_short_a4_line_cnt))
				{
					if(zone1_short_mask)
					{
						zone1_alarm_sta = BROKEN_STA;
					}
					else
					{
						zone1_alarm_sta = SHORT_STA;
					}
				}
				else
				{
					zone1_alarm_sta = BROKEN_STA;
				}
			}
			else if((zone1_alarm_sta != TOUCH_NET_STA) && (zone1_alarm_sta != BYPASS_STA))
			{
				zone1_alarm_sta = NORMAL_STA;
			}
			broken_short_a1_line_cnt = 0;
			broken_short_a2_line_cnt = 0;
			broken_short_a3_line_cnt = 0;
			broken_short_a4_line_cnt = 0;
	}
	else if(zone_num == ZONE2)
	{
			if(broken_short_b1_line_cnt <= 1)
			{
				broken_short_b1_line_cnt = 0;
			}
			if(broken_short_b2_line_cnt <= 1)
			{
				broken_short_b2_line_cnt = 0;
			}
			if(broken_short_b3_line_cnt <= 1)
			{
				broken_short_b3_line_cnt = 0;
			}
			if(broken_short_b4_line_cnt <= 1)
			{
				broken_short_b4_line_cnt = 0;
			}
		
			if(!(broken_short_b1_line_cnt * broken_short_b2_line_cnt * broken_short_b3_line_cnt * broken_short_b4_line_cnt))
			{
				if(!(broken_short_b1_line_cnt + broken_short_b2_line_cnt + broken_short_b3_line_cnt + broken_short_b4_line_cnt))
				{
					if(zone2_short_mask)
					{
						zone2_alarm_sta = BROKEN_STA;
					}
					else
					{
						zone2_alarm_sta = SHORT_STA;
					}
				}
				else
				{
					zone2_alarm_sta = BROKEN_STA;
				}
			}
			else if(zone2_alarm_sta != TOUCH_NET_STA)
			{
				zone2_alarm_sta = NORMAL_STA;
			}
			
			broken_short_b1_line_cnt = 0;
			broken_short_b2_line_cnt = 0;
			broken_short_b3_line_cnt = 0;
			broken_short_b4_line_cnt = 0;
	}
}

/*防区状态查询 */
void alarm_inquire(void)
{
	if(zone1_alarm_delay_finish_mask)
	{
		zone1_alarm_delay_finish_mask = 0;
		
		if(zone1_alarm_sta != pre_zone1_alarm_sta)
		{
			HAL_Delay(5);
			pre_zone1_alarm_sta = zone1_alarm_sta;
			send_sta_msg(ZONE1_STA, (uint8_t)zone1_alarm_sta);
		}
	}
	
	if(zone2_alarm_delay_finish_mask)
	{
		zone2_alarm_delay_finish_mask = 0;
		
		if(zone2_alarm_sta != pre_zone2_alarm_sta)
		{
			HAL_Delay(5);
			pre_zone2_alarm_sta = zone2_alarm_sta;
			send_sta_msg(ZONE2_STA, (uint8_t)zone2_alarm_sta);
		}
	}
}

float get_return_voltage(uint8_t zone_num)
{
	ADC_ChannelConfTypeDef sConfig;
	float reference_voltage = 1.24f;		//参考电压
	uint16_t vref_value;
	uint16_t get_value;
	float get_voltage;
	
	sConfig.Channel = ADC_CHANNEL_4;
  sConfig.Rank = 1;
  sConfig.SamplingTime = ADC_SAMPLETIME_239CYCLES_5;
	
  HAL_ADC_ConfigChannel(&hadc1, &sConfig);
	HAL_ADC_Start(&hadc1);
	HAL_ADC_PollForConversion(&hadc1, 10);
	vref_value = (uint16_t)HAL_ADC_GetValue(&hadc1);
	
	if(zone_num == ZONE1)
	{
		sConfig.Channel = ADC_CHANNEL_5;
	}
	else
	{
		sConfig.Channel = ADC_CHANNEL_6;
	}
	
	HAL_ADC_ConfigChannel(&hadc1, &sConfig);
	HAL_ADC_Start(&hadc1);
	HAL_ADC_PollForConversion(&hadc1, 10);
	get_value = (uint16_t)HAL_ADC_GetValue(&hadc1);
	
	get_voltage = (float)(reference_voltage*get_value/vref_value);
	
	return get_voltage;
}

float get_max_voltage(uint8_t zone_num)
{
	float voltage = 0.0f;
	float pre_voltage = 0.0f;
	
	while(voltage >= pre_voltage)
	{
		pre_voltage = voltage;
		voltage = get_return_voltage(zone_num);
		
		if(voltage == 0.0f)
		{
			break;
		}
	}
	voltage = pre_voltage;
	
	return voltage;
}

void touch_net_dectec(uint8_t zone_num)
{
	float voltage;
	
	float zone1_max_normal_voltage;						//最大正常电压				短路<正线触网<正常<旁路<负线触网
	float zone1_min_normal_voltage;						//最小正常电压
	float zone2_max_normal_voltage;
	float zone2_min_normal_voltage;
	float zone1_short_voltage;
	float zone2_short_voltage;
	
//	float p_touch_net_voltage;				//正线触网电压
//	float n_touch_net_voltage;					//负线触网电压
	
	if(protection_level == HIGH_VOLTAGE_MODE)
	{
		zone1_max_normal_voltage = zone1_high_max_normal_voltage + 0.1f;
		zone1_min_normal_voltage = zone1_high_min_normal_voltage - 0.1f;
	
		zone2_max_normal_voltage = zone2_high_max_normal_voltage + 0.1f;
		zone2_min_normal_voltage = zone2_high_min_normal_voltage - 0.1f;
		
		zone1_short_voltage = 0.04;
		zone2_short_voltage = 0.005;
//		n_touch_net_voltage = 0.91f;
	}
	else if(protection_level == LOW_VOLTAGE_MODE)
	{
		zone1_max_normal_voltage = zone1_low_max_normal_voltage + 0.03;
		zone1_min_normal_voltage = zone1_low_min_normal_voltage - 0.03;
		
		zone2_max_normal_voltage = zone2_low_max_normal_voltage + 0.03;
		zone2_min_normal_voltage = zone2_low_min_normal_voltage - 0.03;		
		
		zone1_short_voltage = 0.04;
		zone2_short_voltage = 0.002;
		
//		n_touch_net_voltage = 0.2f;
	}
	
	voltage = get_max_voltage(zone_num);
//	if(zone_num == ZONE1)
//	{
//	/*	 ((		      负线触网					)    || (                       正线触网                            )) && 触网模式		*/
//		if(((voltage >= n_touch_net_voltage) || ((voltage > short_voltage) && (voltage < zone1_min_normal_voltage))) && touch_net_mode)
//		{
//			zone1_alarm_sta = TOUCH_NET_STA;
//		}
//		else if((voltage > zone1_max_normal_voltage) && (voltage < n_touch_net_voltage) && touch_net_mode)
//		{
//			zone1_alarm_sta = BYPASS_STA;
//		}
//		else if((voltage >= zone1_min_normal_voltage) && (voltage <= zone1_max_normal_voltage))
//		{
//			zone1_short_mask = 1;
//			zone1_alarm_sta = NORMAL_STA;		
//		}
//		else
//		{
//			zone1_short_mask = 0;
//			zone1_alarm_sta = NORMAL_STA;			
//		}
//	}
//	else if(zone_num == ZONE2)
//	{
//	/*	 ((		      负线触网					)    || (                       正线触网                            )) && 触网模式		*/
//		if(((voltage >= n_touch_net_voltage) || ((voltage > short_voltage) && (voltage < zone2_min_normal_voltage))) && touch_net_mode)
//		{
//			zone2_alarm_sta = TOUCH_NET_STA;
//		}
//		else if((voltage > zone2_max_normal_voltage) && (voltage < n_touch_net_voltage) && touch_net_mode)
//		{
//			zone2_alarm_sta = BYPASS_STA;
//		}
//		else if((voltage >= zone2_min_normal_voltage) && (voltage <= zone2_max_normal_voltage))
//		{
//			zone2_short_mask = 1;
//			zone2_alarm_sta = NORMAL_STA;		
//		}
//		else
//		{
//			zone2_short_mask = 0;
//			zone2_alarm_sta = NORMAL_STA;			
//		}	
//	}

	if(zone_num == ZONE1)
	{
		if((((voltage > zone1_max_normal_voltage) || (voltage < zone1_min_normal_voltage))&&(voltage > zone1_short_voltage)) && touch_net_mode)
		{
			zone1_alarm_sta = TOUCH_NET_STA;
		}
		else if((voltage > zone1_min_normal_voltage) && (voltage < zone1_max_normal_voltage))
		{
			zone1_short_mask = 1;
			zone1_alarm_sta = NORMAL_STA;	
		}
		else
		{
			zone1_short_mask = 0;
			zone1_alarm_sta = NORMAL_STA;
		}
	}
	else if(zone_num == ZONE2)
	{
		if((((voltage > zone2_max_normal_voltage) || (voltage < zone2_min_normal_voltage))&&(voltage > zone2_short_voltage)) && touch_net_mode)
		{
			zone2_alarm_sta = TOUCH_NET_STA;
		}
		else if((voltage > zone2_min_normal_voltage) && (voltage < zone2_max_normal_voltage))
		{
			zone2_short_mask = 1;
			zone2_alarm_sta = NORMAL_STA;	
		}
		else
		{
			zone2_short_mask = 0;
			zone2_alarm_sta = NORMAL_STA;
		}		
	}
}

/*从小到大排序*/
void sort_float(float *data, uint16_t data_size)
{
	uint16_t i, j;
	float temp;
	
	for(i=0; i<data_size; i++)
	{
		for(j=i+1; j<data_size; j++)
		{
			if(data[i]>data[j])
			{
					temp = data[i];
					data[i] = data[j];
					data[j] = temp;
			}
		}
	}
}

/*获取做大值 最小值*/
void get_extreme_value(float *max_value, float *min_value, float *data, uint16_t data_size)
{
	sort_float(data, data_size);
	*max_value = data[data_size - 1];
	*min_value = data[0];
}

void auto_dectect(void)
{
	uint8_t i;
	
	float zone1_high_normal_voltage[50];
	float zone1_low_normal_voltage[50];
	float zone2_high_normal_voltage[50];
	float zone2_low_normal_voltage[50];	
	
	for(i=0; i<50; i++)
	{
		primary_boost(HIGH_VOLTAGE_MODE);
		HAL_GPIO_WritePin(ZONE1_PULSE_RELEASE_PIN_GPIO_Port, ZONE1_PULSE_RELEASE_PIN_Pin, GPIO_PIN_RESET);
		HAL_Delay(1);
		zone1_high_normal_voltage[i] = get_max_voltage(ZONE1);
		HAL_Delay(20);
		HAL_GPIO_WritePin(ZONE1_PULSE_RELEASE_PIN_GPIO_Port, ZONE1_PULSE_RELEASE_PIN_Pin, GPIO_PIN_SET);
		HAL_Delay(100);
	}

	for(i=0; i<50; i++)
	{
		primary_boost(LOW_VOLTAGE_MODE);
		HAL_GPIO_WritePin(ZONE1_PULSE_RELEASE_PIN_GPIO_Port, ZONE1_PULSE_RELEASE_PIN_Pin, GPIO_PIN_RESET);
		HAL_Delay(1);
		zone1_low_normal_voltage[i] = get_max_voltage(ZONE1);
		HAL_Delay(20);
		HAL_GPIO_WritePin(ZONE1_PULSE_RELEASE_PIN_GPIO_Port, ZONE1_PULSE_RELEASE_PIN_Pin, GPIO_PIN_SET);
		HAL_Delay(100);
	}
	
	for(i=0; i<50; i++)
	{
		primary_boost(HIGH_VOLTAGE_MODE);
		HAL_GPIO_WritePin(ZONE2_PULSE_RELEASE_PIN_GPIO_Port, ZONE2_PULSE_RELEASE_PIN_Pin, GPIO_PIN_RESET);
		HAL_Delay(1);
		zone2_high_normal_voltage[i] = get_max_voltage(ZONE2);
		HAL_Delay(20);
		HAL_GPIO_WritePin(ZONE2_PULSE_RELEASE_PIN_GPIO_Port, ZONE2_PULSE_RELEASE_PIN_Pin, GPIO_PIN_SET);
		HAL_Delay(100);
	}
	
	for(i=0; i<50; i++)
	{
		primary_boost(LOW_VOLTAGE_MODE);
		HAL_GPIO_WritePin(ZONE2_PULSE_RELEASE_PIN_GPIO_Port, ZONE2_PULSE_RELEASE_PIN_Pin, GPIO_PIN_RESET);
		HAL_Delay(1);
		zone2_low_normal_voltage[i] = get_max_voltage(ZONE2);
		HAL_Delay(20);
		HAL_GPIO_WritePin(ZONE2_PULSE_RELEASE_PIN_GPIO_Port, ZONE2_PULSE_RELEASE_PIN_Pin, GPIO_PIN_SET);
		HAL_Delay(100);
	}
	
	get_extreme_value(&zone1_high_max_normal_voltage,&zone1_high_min_normal_voltage, zone1_high_normal_voltage, 50);
	get_extreme_value(&zone1_low_max_normal_voltage,&zone1_low_min_normal_voltage, zone1_low_normal_voltage, 50);
	get_extreme_value(&zone2_high_max_normal_voltage,&zone2_high_min_normal_voltage, zone2_high_normal_voltage, 50);
	get_extreme_value(&zone2_low_max_normal_voltage,&zone2_low_min_normal_voltage, zone2_low_normal_voltage, 50);
	
	flash_data_struct.flash_zone1_high_max_normal_voltage = zone1_high_max_normal_voltage;
	flash_data_struct.flash_zone1_high_min_normal_voltage = zone1_high_min_normal_voltage;
	flash_data_struct.flash_zone1_low_max_normal_voltage = zone1_low_max_normal_voltage;
	flash_data_struct.flash_zone1_low_min_normal_voltage = zone1_low_min_normal_voltage;
	flash_data_struct.flash_zone2_high_max_normal_voltage = zone2_high_max_normal_voltage;
	flash_data_struct.flash_zone2_high_min_normal_voltage = zone2_high_min_normal_voltage;
	flash_data_struct.flash_zone2_low_max_normal_voltage = zone2_low_max_normal_voltage;
	flash_data_struct.flash_zone2_low_min_normal_voltage = zone2_low_min_normal_voltage;
	write_flash_flag = 1;
	
	auto_detect_sta = 0;
	send_sta_msg(AUTO_DETECT, 0x02);
}




































