#include "boost.h"

extern TIM_HandleTypeDef htim2;
extern TIM_HandleTypeDef htim1;
extern ADC_HandleTypeDef hadc1;

uint8_t flag = 0;		//测试数据 可删除
/* 默认：撤防 双防区 高压模式 */
uint8_t zone1_arming_sta = 0;							//防区1布防状态 1：布防 0：撤防
uint8_t zone2_arming_sta = 0;							//防区2布防状态 1：布防 0：撤防
uint8_t zone_type = DOUBLE_ZONE;
uint8_t zone1_protection_level = HIGH_VOLTAGE_MODE;
uint8_t zone2_protection_level = HIGH_VOLTAGE_MODE;
uint8_t zone1_sensitivity = 3;
uint8_t zone2_sensitivity = 3;
uint8_t zone1_mode = 0;  				//0:脉冲模式(不检测触网)   1:触网模式
uint8_t zone2_mode = 0;

uint8_t boost_delay_cnt = 0;
uint8_t release_pulse_time_cnt = 0;

uint8_t boost_finish_flag = 0;
uint8_t boost_delay_finish_mask = 0;
uint8_t release_pulse_finish_mask = 0;

uint16_t broken_short_a1_line_cnt = 0;
uint16_t broken_short_a2_line_cnt = 0;
uint16_t broken_short_a3_line_cnt = 0;
uint16_t broken_short_a4_line_cnt = 0;
uint16_t broken_short_b1_line_cnt = 0;
uint16_t broken_short_b2_line_cnt = 0;
uint16_t broken_short_b3_line_cnt = 0;
uint16_t broken_short_b4_line_cnt = 0;

uint8_t zone1_touch_net_cnt = 0;	//触网计数
uint8_t zone2_touch_net_cnt = 0;	//触网计数

alarm_sta_typedef zone1_alarm_sta = NORMAL_STA;
alarm_sta_typedef zone2_alarm_sta = NORMAL_STA;

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

/*电压检测滤波缓存*/
float zone1_high_voltage_buff[MAX_FILTER_QUANTITY];
float zone1_low_voltage_buff[MAX_FILTER_QUANTITY];
float zone2_high_voltage_buff[MAX_FILTER_QUANTITY];
float zone2_low_voltage_buff[MAX_FILTER_QUANTITY];

uint8_t auto_detect_sta = 0;

uint8_t zone1_high_voltage_filter_finish = 0;	//防区1高压滤波完成
uint8_t zone1_low_voltage_filter_finish = 0;		//防区1低压滤波完成 
uint8_t zone2_high_voltage_filter_finish = 0;	//防区2高压滤波完成
uint8_t zone2_low_voltage_filter_finish = 0;		//防区2低压滤波完成

uint8_t zone1_fliter_finish = 0;
uint8_t zone2_fliter_finish = 0;

/*电压检测计数*/
uint8_t zone1_high_voltage_detect_cnt = 0;
uint8_t zone1_low_voltage_detect_cnt = 0;
uint8_t zone2_high_voltage_detect_cnt = 0;
uint8_t zone2_low_voltage_detect_cnt = 0;

uint8_t power_use_sta = USE_AC_POWER;
uint8_t pre_power_use_sta = USE_AC_POWER;

/*从小到大排序*/
void sort_float(float *data, uint16_t data_size)
{
	uint16_t i, j;
	float temp;
	
	for(i=0; i<data_size; i++)
	{
		for(j=i+1; j<data_size; j++)
		{
			if(*(data+i)>*(data+j))
			{
					temp = *(data+i);
					*(data+i) = *(data+j);
					*(data+j) = temp;
			}
		}
	}
}

/*获取做大值 最小值*/
void get_extreme_value(float *max_value, float *min_value, float *data, uint16_t data_size)
{
	sort_float(data, data_size);
	*max_value = *(data + (data_size-1));
	*min_value = *data;
}

/*
功能：	滤波： 去掉最大值  去掉最小值 求平均数
返回值：滤波值
*/
float filter(float *data, uint16_t data_size)
{
	uint16_t i;
	float sum = 0.0f;
	float filter_value = 0.0f;
	float data_temp[MAX_FILTER_QUANTITY];
	
	for(i=0; i<data_size; i++)
	{
		*(data_temp + i) = *(data + i);
	}
	
	sort_float(data_temp, data_size);
	for(i=1; i<(data_size-1); i++)
	{
		sum += *(data_temp+i);
	}
	
	filter_value = sum / (data_size-2);
	
	return filter_value;
}


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
	uint64_t i = 0;
	
	if(power_use_sta != pre_power_use_sta)
	{
		pre_power_use_sta = power_use_sta;
		if(power_use_sta == USE_AC_POWER)
		{
			configure_boost_pwm_duty_cycle(20);
		}
		else if(power_use_sta == USE_BATTERY_POWER)
		{
			configure_boost_pwm_duty_cycle(28);
		}
	}
	
	start_primary_boost();	/*开始前级升压 */
	
	/*根据升压等级 等待升到设定等级对应电压*/
	if(level == LOW_VOLTAGE_MODE)
	{
		while(!HAL_GPIO_ReadPin(DETECT_LEVEL_LOW_GPIO_Port, DETECT_LEVEL_LOW_Pin))
		{
			if(++i >= 450000)
			{
				flag = 1;
				break;
			}
		};
		i = 0;
	}
	else if(level == HIGH_VOLTAGE_MODE)
	{
		while(!HAL_GPIO_ReadPin(DETECT_LEVEL_HIGH_GPIO_Port, DETECT_LEVEL_HIGH_Pin))
		{
			if(++i >= 450000)
			{
				flag = 1;
				break;
			}
		};
		i = 0;
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
	
	while(!release_pulse_finish_mask)
	{
		if(!(zone1_arming_sta || zone1_arming_sta))
		{
			break;
		}
	}
	
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
		if(zone1_arming_sta)
		{
			primary_boost(zone1_protection_level);
			ralease_pulse(ZONE1);
			broken_detect(ZONE1);
		}

		if(zone2_arming_sta)
		{
			primary_boost(zone2_protection_level);
			ralease_pulse(ZONE2);
			broken_detect(ZONE2);
		}
	}
}

/*布防、撤防*/
void arming_disarm(uint8_t zone_num, uint8_t sta)
{
	if(zone_num == 0x01)
	{
		zone1_arming_sta = sta;
	}
	else if(zone_num == 0x02)
	{
		if(zone_type == DOUBLE_ZONE)
			zone2_arming_sta = sta;
		else 
			return;
	}
	else if(zone_num == 0xFF)
	{
		if(zone_type == DOUBLE_ZONE)
			zone1_arming_sta = zone2_arming_sta = sta;
		else
			zone1_arming_sta = sta;
	}
	
	/*如果撤防 清除计数值 和标志位*/
	
	if(!(zone1_arming_sta && zone2_arming_sta))
	{
		boost_delay_cnt = 0;
		release_pulse_time_cnt = 0;

		boost_finish_flag = 0;
		boost_delay_finish_mask = 0;
		release_pulse_finish_mask = 0;
		
		broken_short_a1_line_cnt = 0;
		broken_short_a2_line_cnt = 0;
		broken_short_a3_line_cnt = 0;
		broken_short_a4_line_cnt = 0;
		broken_short_b1_line_cnt = 0;
		broken_short_b2_line_cnt = 0;
		broken_short_b3_line_cnt = 0;
		broken_short_b4_line_cnt = 0;
		
		zone1_alarm_sta = NORMAL_STA;
		zone2_alarm_sta = NORMAL_STA;
		
		zone1_touch_net_cnt = 0;
		zone2_touch_net_cnt = 0;
	}
	else if((zone1_arming_sta == 0) && (zone2_arming_sta == 1))
	{
		broken_short_a1_line_cnt = 0;
		broken_short_a2_line_cnt = 0;
		broken_short_a3_line_cnt = 0;
		broken_short_a4_line_cnt = 0;
		
		zone1_alarm_sta = NORMAL_STA;
		zone1_touch_net_cnt = 0;
	}
	else if((zone1_arming_sta == 1) && (zone2_arming_sta == 0))
	{
		broken_short_b1_line_cnt = 0;
		broken_short_b2_line_cnt = 0;
		broken_short_b3_line_cnt = 0;
		broken_short_b4_line_cnt = 0;
		
		zone2_alarm_sta = NORMAL_STA;
		zone2_touch_net_cnt = 0;
	}
}

/*设置电压等级*/
/*....................
zone_num:防区号 0x01:防区1  0x02:防区2 0xFF：全防区
level:
...................*/
void set_protection_level(uint8_t zone_num, uint8_t level)
{
	if(zone_num == ZONE1)
	{
		zone1_protection_level = level;
	}
	else if(zone_num == ZONE2)
	{
		if(zone_type == DOUBLE_ZONE)
			zone2_protection_level = level;
		else
			return;
	}
	else if(zone_num == ZONE_ALL)
	{
		if(zone_type == DOUBLE_ZONE)
			zone1_protection_level = zone2_protection_level = level;
		else
			zone1_protection_level = level;
	}
}

/*设置灵敏度*/
void set_sensitivity(uint8_t zone_num, uint8_t level)
{
	if(zone_num == ZONE1)
	{
		zone1_sensitivity = level;
	}
	else if(zone_num == ZONE2)
	{
		if(zone_type == DOUBLE_ZONE)
			zone2_sensitivity = level;
	}
	else if(zone_num == ZONE_ALL)
	{
		if(zone_type == DOUBLE_ZONE)
			zone1_sensitivity = zone2_sensitivity = level;
		else
			zone1_sensitivity = level;
	}
}

void set_zone_mode(uint8_t zone_num, uint8_t mode)
{
	if(zone_num == ZONE1)
	{
		zone1_mode = mode;
	}
	else if(zone_num == ZONE2)
	{
		if(zone_type == DOUBLE_ZONE)
			zone2_mode = mode;
	}
	else if(zone_num == ZONE_ALL)
	{
		if(zone_type == DOUBLE_ZONE)
			zone1_mode = zone2_mode = mode;
		else
			zone1_mode = mode;
	}
}

void configure_boost_pwm_duty_cycle(uint8_t duty_cycle)
{
		TIM_OC_InitTypeDef sConfigOC;
	
		sConfigOC.OCMode = TIM_OCMODE_PWM1;
		sConfigOC.Pulse = duty_cycle;
		sConfigOC.OCPolarity = TIM_OCPOLARITY_LOW;
		sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
		if (HAL_TIM_PWM_ConfigChannel(&htim2, &sConfigOC, TIM_CHANNEL_2) != HAL_OK)
		{
			_Error_Handler(__FILE__, __LINE__);
		}
}

/*断线、短路检测*/
void broken_detect(uint8_t zone_num)
{

	if(zone_num == ZONE1)
	{
//			if(broken_short_a1_line_cnt <= 1)
//			{
//				broken_short_a1_line_cnt = 0;
//			}
//			if(broken_short_a2_line_cnt <= 1)
//			{
//				broken_short_a2_line_cnt = 0;
//			}
//			if(broken_short_a3_line_cnt <= 1)
//			{
//				broken_short_a3_line_cnt = 0;
//			}
//			if(broken_short_a4_line_cnt <= 1)
//			{
//				broken_short_a4_line_cnt = 0;
//			}
			
			/*如果有一条线或多条没有检测到 则计数值不会增加 所以相乘等于0  报警*/
			if(!(broken_short_a1_line_cnt * broken_short_a2_line_cnt * broken_short_a3_line_cnt * broken_short_a4_line_cnt))
			{
				/*如果所有线均没有返回脉冲 则所有线计数都为0 则计数为0 相加为0 短路报警 */
				if(!(broken_short_a1_line_cnt + broken_short_a2_line_cnt + broken_short_a3_line_cnt + broken_short_a4_line_cnt))
				{
					if(zone1_short_mask)
					{
						zone1_alarm_sta = PN_BROKEN_STA;			//正线负线全断线
					}
					else
					{
						zone1_alarm_sta = SHORT_STA;
					}
				}
				else if((!(broken_short_a1_line_cnt * broken_short_a2_line_cnt)) && (!(broken_short_a3_line_cnt * broken_short_a4_line_cnt)))
				{
					zone1_alarm_sta = PN_BROKEN_STA;			//正线负线全断线
				}
				else if(!(broken_short_a1_line_cnt * broken_short_a2_line_cnt))	//负线断线
				{
					zone1_alarm_sta = N_BROKEN_STA;
				}
				else if(!(broken_short_a3_line_cnt * broken_short_a4_line_cnt))	//正线断线
				{
					zone1_alarm_sta = P_BROKEN_STA;
				}
			}
			else if((zone1_alarm_sta != TOUCH_NET_STA) && (zone1_alarm_sta != BYPASS_STA))
			{
				zone1_alarm_sta = NORMAL_STA;
			}
			broken_short_a1_line_cnt = 0;
			broken_short_a2_line_cnt = 0;
			broken_short_a3_line_cnt = 0;
			broken_short_a4_line_cnt = 0;		//把防区2也清掉为了防止防区二释放脉冲时对防区2断线检测中断有干扰到
			
			broken_short_b1_line_cnt = 0;
			broken_short_b2_line_cnt = 0;
			broken_short_b3_line_cnt = 0;
			broken_short_b4_line_cnt = 0;
	}
	else if(zone_num == ZONE2)
	{
//			if(broken_short_b1_line_cnt <= 1)
//			{
//				broken_short_b1_line_cnt = 0;
//			}
//			if(broken_short_b2_line_cnt <= 1)
//			{
//				broken_short_b2_line_cnt = 0;
//			}
//			if(broken_short_b3_line_cnt <= 1)
//			{
//				broken_short_b3_line_cnt = 0;
//			}
//			if(broken_short_b4_line_cnt <= 1)
//			{
//				broken_short_b4_line_cnt = 0;
//			}
		
			if(!(broken_short_b1_line_cnt * broken_short_b2_line_cnt * broken_short_b3_line_cnt * broken_short_b4_line_cnt))
			{
				if(!(broken_short_b1_line_cnt + broken_short_b2_line_cnt + broken_short_b3_line_cnt + broken_short_b4_line_cnt))
				{
					if(zone2_short_mask)
					{
						zone2_alarm_sta = PN_BROKEN_STA;		//正线负线全断线
					}
					else
					{
						zone2_alarm_sta = SHORT_STA;
					}
				}
				else if((!(broken_short_b1_line_cnt * broken_short_b2_line_cnt)) && (!(broken_short_b3_line_cnt * broken_short_b4_line_cnt)))
				{
					zone2_alarm_sta = PN_BROKEN_STA;		//正线负线全断线
				}
				else if(!(broken_short_b1_line_cnt * broken_short_b2_line_cnt))	//正线断线
				{
					zone2_alarm_sta = N_BROKEN_STA;
				}
				else if(!(broken_short_b3_line_cnt * broken_short_b4_line_cnt))	//负线断线
				{
					zone2_alarm_sta = P_BROKEN_STA;
				}
			}
			else if(zone2_alarm_sta != TOUCH_NET_STA)
			{
				zone2_alarm_sta = NORMAL_STA;
			}

			broken_short_a1_line_cnt = 0;
			broken_short_a2_line_cnt = 0;
			broken_short_a3_line_cnt = 0;
			broken_short_a4_line_cnt = 0;	//把防区1也清掉为了防止防区二释放脉冲时对防区1断线检测中断有干扰到
			broken_short_b1_line_cnt = 0;
			broken_short_b2_line_cnt = 0;
			broken_short_b3_line_cnt = 0;
			broken_short_b4_line_cnt = 0;
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

void insert(float *array, float data, uint16_t data_size)
{
	uint16_t i;
	
	for(i=(data_size-1); i>=1; i--)
	{
		array[i] = array[i-1];
	}
	array[0] = data;
}

float zone1_voltage = 0.0f;
float zone2_voltage = 0.0f;

void touch_net_dectec(uint8_t zone_num)
{
	float voltage;
	
	float zone1_max_normal_voltage;						//最大正常电压				短路<正线触网<正常<旁路<负线触网
	float zone1_min_normal_voltage;						//最小正常电压
	float zone2_max_normal_voltage;
	float zone2_min_normal_voltage;
	float zone1_short_voltage;
	float zone2_short_voltage;
	
	uint8_t zone1_filter_quantity;
	uint8_t zone2_filter_quantity;
	uint8_t zone1_trigger_quantity;
	uint8_t zone2_trigger_quantity;	
	
	switch(zone1_sensitivity)
	{
		case 3:	zone1_filter_quantity = 3; zone1_trigger_quantity = 1; break;
		case 2:	zone1_filter_quantity = 5; zone1_trigger_quantity = 2; break;
		case 1:	zone1_filter_quantity = 8; zone1_trigger_quantity = 2; break;
		default: break;
	}
	
	switch(zone2_sensitivity)
	{
		case 3:	zone2_filter_quantity = 3; zone2_trigger_quantity = 1; break;
		case 2:	zone2_filter_quantity = 5; zone2_trigger_quantity = 2; break;
		case 1:	zone2_filter_quantity = 8; zone2_trigger_quantity = 2; break;
		default: break;
	}
	
	if(zone1_protection_level == HIGH_VOLTAGE_MODE)
	{
		zone1_max_normal_voltage = zone1_high_max_normal_voltage + 0.04;
		zone1_min_normal_voltage = zone1_high_min_normal_voltage - 0.04;
		
		zone1_short_voltage = 0.02;
	}
	else if(zone1_protection_level == LOW_VOLTAGE_MODE)
	{
		zone1_max_normal_voltage = zone1_low_max_normal_voltage + 0.02;
		zone1_min_normal_voltage = zone1_low_min_normal_voltage - 0.02;

		zone1_short_voltage = 0.002;
	}
	
	if(zone2_protection_level == HIGH_VOLTAGE_MODE)
	{
		zone2_max_normal_voltage = zone2_high_max_normal_voltage + 0.04;
		zone2_min_normal_voltage = zone2_high_min_normal_voltage - 0.04;
		
		zone2_short_voltage = 0.02;
	}
	else if(zone2_protection_level == LOW_VOLTAGE_MODE)
	{
		zone2_max_normal_voltage = zone2_low_max_normal_voltage + 0.02;
		zone2_min_normal_voltage = zone2_low_min_normal_voltage - 0.02;
		
		zone2_short_voltage = 0.002;
	}
	
	voltage = get_max_voltage(zone_num);
	
	if(zone_num == ZONE1)
	{
		zone1_voltage = voltage;
		if(zone1_protection_level == HIGH_VOLTAGE_MODE)
		{
			insert(zone1_high_voltage_buff, voltage, zone1_filter_quantity);
			
			if(!zone1_high_voltage_filter_finish)												//如果没有完成滤波
			{
				zone1_fliter_finish = 0;
				if(++zone1_high_voltage_detect_cnt >= zone1_filter_quantity)
				{
					zone1_high_voltage_filter_finish = 1;
					zone1_fliter_finish = 1;
				}
			}
			
			if(zone1_high_voltage_filter_finish)
			{
				voltage = filter(zone1_high_voltage_buff, zone1_filter_quantity);
			}
		}
		else if(zone1_protection_level == LOW_VOLTAGE_MODE)
		{
			insert(zone1_low_voltage_buff, voltage, zone1_filter_quantity);
			
			if(!zone1_low_voltage_filter_finish)	//如果没有完成滤波（检测十次）
			{
				zone1_fliter_finish = 0;
				if(++zone1_low_voltage_detect_cnt >= zone1_filter_quantity)
				{
					zone1_low_voltage_filter_finish = 1;
					zone1_fliter_finish = 1;
				}
			}
			
			if(zone1_low_voltage_filter_finish)
			{
				voltage = filter(zone1_low_voltage_buff, zone1_filter_quantity);
			}			
		}
		
		if((((voltage > zone1_max_normal_voltage) || (voltage < zone1_min_normal_voltage))&&(voltage > zone1_short_voltage)) && zone1_mode && zone1_fliter_finish)
		{
			if(++zone1_touch_net_cnt >= zone1_trigger_quantity)
			{	
				zone1_touch_net_cnt = 0;
				zone1_alarm_sta = TOUCH_NET_STA;
				zone1_short_mask = 1;
			}
		}
//		else if((voltage > zone1_min_normal_voltage) && (voltage < zone1_max_normal_voltage))
//		{
//			zone1_short_mask = 1;
//			zone1_alarm_sta = NORMAL_STA;
//			zone1_touch_net_cnt = 0;
//		}
		else if(voltage < zone1_short_voltage)
		{
			zone1_short_mask = 0;
			zone1_alarm_sta = NORMAL_STA;
			zone1_touch_net_cnt = 0;
		}
		else
		{
			zone1_short_mask = 1;
			zone1_alarm_sta = NORMAL_STA;
			zone1_touch_net_cnt = 0;		
		}
	}
	else if(zone_num == ZONE2)
	{
		zone2_voltage = voltage;
		if(zone2_protection_level == HIGH_VOLTAGE_MODE)
		{
			insert(zone2_high_voltage_buff, voltage, zone2_filter_quantity);
			
			if(!zone2_high_voltage_filter_finish)	//如果没有完成滤波（检测十次）
			{
				zone2_fliter_finish = 0;
				if(++zone2_high_voltage_detect_cnt >= zone2_filter_quantity)
				{
					zone2_high_voltage_filter_finish = 1;
					zone2_fliter_finish = 1;
				}
			}
			
			if(zone2_high_voltage_filter_finish)
			{
				voltage = filter(zone2_high_voltage_buff, zone2_filter_quantity);
			}
		}
		else if(zone2_protection_level == LOW_VOLTAGE_MODE)
		{
			insert(zone2_low_voltage_buff, voltage, zone2_filter_quantity);
			
			if(!zone2_low_voltage_filter_finish)	//如果没有完成滤波（检测十次）
			{
				zone2_fliter_finish = 0;
				if(++zone2_low_voltage_detect_cnt >= zone2_filter_quantity)
				{
					zone2_low_voltage_filter_finish = 1;
					zone2_fliter_finish = 1;
				}
			}
			
			if(zone2_low_voltage_filter_finish)
			{
				voltage = filter(zone2_low_voltage_buff, zone2_filter_quantity);
			}			
		}
		
		if((((voltage > zone2_max_normal_voltage) || (voltage < zone2_min_normal_voltage))&&(voltage > zone2_short_voltage)) && zone2_mode && zone2_fliter_finish)
		{
			if(++zone2_touch_net_cnt >= zone2_trigger_quantity)
			{	
				zone2_short_mask = 1;
				zone2_touch_net_cnt = 0;
				zone2_alarm_sta = TOUCH_NET_STA;
			}
		}
//		else if((voltage > zone2_min_normal_voltage) && (voltage < zone2_max_normal_voltage))
//		{
//			zone2_short_mask = 1;
//			zone2_alarm_sta = NORMAL_STA;	
//			zone2_touch_net_cnt = 0;
//		}
		else if(voltage < zone2_short_voltage)
		{
			zone2_short_mask = 0;
			zone2_alarm_sta = NORMAL_STA;
			zone2_touch_net_cnt = 0;
		}
		else
		{
			zone2_short_mask = 1;
			zone2_alarm_sta = NORMAL_STA;	
			zone2_touch_net_cnt = 0;
		}
	}
}

void auto_dectect(void)
{
	uint8_t i;
	
	float zone1_high_normal_voltage[50];
	float zone1_low_normal_voltage[50];
	float zone2_high_normal_voltage[50];
	float zone2_low_normal_voltage[50];	
	
	if(!auto_detect_sta)
	{
		return;
	}
	
	for(i=0; i<50; i++)
	{
		primary_boost(HIGH_VOLTAGE_MODE);
		HAL_GPIO_WritePin(ZONE1_PULSE_RELEASE_PIN_GPIO_Port, ZONE1_PULSE_RELEASE_PIN_Pin, GPIO_PIN_RESET);
		HAL_Delay(1);
		zone1_high_normal_voltage[i] = get_max_voltage(ZONE1);
		HAL_Delay(20);
		HAL_GPIO_WritePin(ZONE1_PULSE_RELEASE_PIN_GPIO_Port, ZONE1_PULSE_RELEASE_PIN_Pin, GPIO_PIN_SET);
//		HAL_Delay(1200);
		
		primary_boost(HIGH_VOLTAGE_MODE);
		HAL_GPIO_WritePin(ZONE2_PULSE_RELEASE_PIN_GPIO_Port, ZONE2_PULSE_RELEASE_PIN_Pin, GPIO_PIN_RESET);
		HAL_Delay(1);
		zone2_high_normal_voltage[i] = get_max_voltage(ZONE2);
		HAL_Delay(20);
		HAL_GPIO_WritePin(ZONE2_PULSE_RELEASE_PIN_GPIO_Port, ZONE2_PULSE_RELEASE_PIN_Pin, GPIO_PIN_SET);
		HAL_Delay(1200);
	}
	
	for(i=0; i<50; i++)
	{
		primary_boost(LOW_VOLTAGE_MODE);
		HAL_GPIO_WritePin(ZONE1_PULSE_RELEASE_PIN_GPIO_Port, ZONE1_PULSE_RELEASE_PIN_Pin, GPIO_PIN_RESET);
		HAL_Delay(1);
		zone1_low_normal_voltage[i] = get_max_voltage(ZONE1);
		HAL_Delay(20);
		HAL_GPIO_WritePin(ZONE1_PULSE_RELEASE_PIN_GPIO_Port, ZONE1_PULSE_RELEASE_PIN_Pin, GPIO_PIN_SET);
//		HAL_Delay(300);
		
		primary_boost(LOW_VOLTAGE_MODE);
		HAL_GPIO_WritePin(ZONE2_PULSE_RELEASE_PIN_GPIO_Port, ZONE2_PULSE_RELEASE_PIN_Pin, GPIO_PIN_RESET);
		HAL_Delay(1);
		zone2_low_normal_voltage[i] = get_max_voltage(ZONE2);
		HAL_Delay(20);
		HAL_GPIO_WritePin(ZONE2_PULSE_RELEASE_PIN_GPIO_Port, ZONE2_PULSE_RELEASE_PIN_Pin, GPIO_PIN_SET);
		HAL_Delay(1200);
	}
	
	for(i=0; i<50; i++)
	{

	}
	
	for(i=0; i<50; i++)
	{

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
	return_set_msg(AUTO_DETECT, 0xFF, 0x02);
}



































