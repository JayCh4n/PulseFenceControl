#include "boost.h"

extern TIM_HandleTypeDef htim2;
extern TIM_HandleTypeDef htim1;
extern ADC_HandleTypeDef hadc1;

/* Ĭ�ϣ����� ˫���� ��ѹģʽ */
uint8_t arming_sta = 0;							//����״̬ 1������ 0������
uint8_t zone_mode = DOUBLE_ZONE;
uint8_t protection_level = HIGH_VOLTAGE_MODE;
uint8_t touch_net_mode = 0;  				//0:����ģʽ(����ⴥ��)   1:����ģʽ

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

uint8_t zone1_touch_net_cnt = 0;	//��������
uint8_t zone2_touch_net_cnt = 0;	//��������

uint16_t zone1_alarm_delay_time = 90;		// ���������ȼ���ó���ʱ����ֵ  * 10ms  Ĭ�ϣ�������Ϊ3  3:900ms   2:2.5s  1:5s
uint16_t zone2_alarm_delay_time = 90;

uint8_t targe_delay_flag = 0;
uint32_t targe_delay_time = 500;	//������ʱ
uint32_t targe_delay_time_cnt = 0;

alarm_sta_typedef zone1_alarm_sta = NORMAL_STA;
alarm_sta_typedef zone2_alarm_sta = NORMAL_STA;

alarm_sta_typedef pre_zone1_alarm_sta = NORMAL_STA;
alarm_sta_typedef pre_zone2_alarm_sta = NORMAL_STA;

uint8_t zone1_short_mask = 0;	//��·���α�־ �����������߱����Ͷ�·���������������������⵽��ѹ ˵��û�ж�·�� 0�������� 1������
uint8_t zone2_short_mask = 0;

float zone1_high_max_normal_voltage;
float zone1_high_min_normal_voltage;

float zone1_low_max_normal_voltage;
float zone1_low_min_normal_voltage;

float zone2_high_max_normal_voltage;
float zone2_high_min_normal_voltage;

float zone2_low_max_normal_voltage;
float zone2_low_min_normal_voltage;

/*��ѹ����˲�����*/
float zone1_high_voltage_buff[MAX_FILTER_QUANTITY];
float zone1_low_voltage_buff[MAX_FILTER_QUANTITY];
float zone2_high_voltage_buff[MAX_FILTER_QUANTITY];
float zone2_low_voltage_buff[MAX_FILTER_QUANTITY];

float view_voltage;

uint8_t auto_detect_sta = 0;

uint8_t zone1_high_voltage_filter_finish = 0;	//����1��ѹ�˲����
uint8_t zone1_low_voltage_filter_finish = 0;		//����1��ѹ�˲���� 
uint8_t zone2_high_voltage_filter_finish = 0;	//����2��ѹ�˲����
uint8_t zone2_low_voltage_filter_finish = 0;		//����2��ѹ�˲����

uint8_t zone1_fliter_finish = 0;
uint8_t zone2_fliter_finish = 0;

/*��ѹ������*/
uint8_t zone1_high_voltage_detect_cnt = 0;
uint8_t zone1_low_voltage_detect_cnt = 0;
uint8_t zone2_high_voltage_detect_cnt = 0;
uint8_t zone2_low_voltage_detect_cnt = 0;

/*��С��������*/
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

/*��ȡ����ֵ ��Сֵ*/
void get_extreme_value(float *max_value, float *min_value, float *data, uint16_t data_size)
{
	sort_float(data, data_size);
	*max_value = *(data + (data_size-1));
	*min_value = *data;
}

/*
���ܣ�	�˲��� ȥ�����ֵ  ȥ����Сֵ ��ƽ����
����ֵ���˲�ֵ
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


/*��ʼǰ����ѹ*/
void start_primary_boost(void)
{
	HAL_GPIO_WritePin(PRIMARY_BOOST_ENABLE_PIN_GPIO_Port, PRIMARY_BOOST_ENABLE_PIN_Pin, GPIO_PIN_SET);	/*ʹ����ѹ���ƽ�*/
	HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_2);	/*������ѹPWM*/
}

/*ֹͣǰ����ѹ*/
void stop_primary_boost(void)
{
	HAL_GPIO_WritePin(PRIMARY_BOOST_ENABLE_PIN_GPIO_Port, PRIMARY_BOOST_ENABLE_PIN_Pin, GPIO_PIN_RESET); /*ʧ����ѹ���ƽ�*/
	HAL_TIM_PWM_Stop(&htim2, TIM_CHANNEL_2);	/*ֹͣ��ѹPWM*/
}

/*�����趨��ѹ ��ֹͣ*/
void primary_boost(uint8_t level)
{
//	uint32_t i = 0;
	start_primary_boost();	/*��ʼǰ����ѹ */
	
	/*������ѹ�ȼ� �ȴ������趨�ȼ���Ӧ��ѹ*/
	if(level == LOW_VOLTAGE_MODE)
	{
		while(!HAL_GPIO_ReadPin(DETECT_LEVEL_LOW_GPIO_Port, DETECT_LEVEL_LOW_Pin));	
	}
	else if(level == HIGH_VOLTAGE_MODE)
	{
		while(!HAL_GPIO_ReadPin(DETECT_LEVEL_HIGH_GPIO_Port, DETECT_LEVEL_HIGH_Pin));	
	}
	//Ӧ�ӳ�ʱ�ж�  ��ֹ�ջ���·
	
	stop_primary_boost(); /*ֹͣǰ����ѹ*/
	
	//������Լ�ģʽ ��Ϊ����ģʽ
	if(!auto_detect_sta)
	{
		boost_finish_flag = 1; /*��ѹ��ɱ�־��1*/
	}
}

/*�ͷ�����*/
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

/*��ѹ�����ͷ�����*/
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

/*����������*/
void arming_disarm(uint8_t sta)
{
	arming_sta = sta;
	
	/*������� �������ֵ �ͱ�־λ*/
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

/*����������*/
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

/*���ߡ���·���*/
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
			
			/*�����һ���߻����û�м�⵽ �����ֵ�������� ������˵���0  ����*/
			if(!(broken_short_a1_line_cnt * broken_short_a2_line_cnt * broken_short_a3_line_cnt * broken_short_a4_line_cnt))
			{
				/*��������߾�û�з������� �������߼�����Ϊ0 �����Ϊ0 ���Ϊ0 ��·���� */
				if(!(broken_short_a1_line_cnt + broken_short_a2_line_cnt + broken_short_a3_line_cnt + broken_short_a4_line_cnt))
				{
					if(zone1_short_mask)
					{
						zone1_alarm_sta = PN_BROKEN_STA;			//���߸���ȫ����
					}
					else
					{
						zone1_alarm_sta = SHORT_STA;
					}
				}
				else if((!(broken_short_a1_line_cnt * broken_short_a2_line_cnt)) && (!(broken_short_a3_line_cnt * broken_short_a4_line_cnt)))
				{
					zone1_alarm_sta = PN_BROKEN_STA;			//���߸���ȫ����
				}
				else if(!(broken_short_a1_line_cnt * broken_short_a2_line_cnt))	//���߶���
				{
					zone1_alarm_sta = N_BROKEN_STA;
				}
				else if(!(broken_short_a3_line_cnt * broken_short_a4_line_cnt))	//���߶���
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
			broken_short_a4_line_cnt = 0;		//�ѷ���2Ҳ���Ϊ�˷�ֹ�������ͷ�����ʱ�Է���2���߼���ж��и��ŵ�
			
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
						zone2_alarm_sta = PN_BROKEN_STA;		//���߸���ȫ����
					}
					else
					{
						zone2_alarm_sta = SHORT_STA;
					}
				}
				else if((!(broken_short_b1_line_cnt * broken_short_b2_line_cnt)) && (!(broken_short_b3_line_cnt * broken_short_b4_line_cnt)))
				{
					zone2_alarm_sta = PN_BROKEN_STA;		//���߸���ȫ����
				}
				else if(!(broken_short_b1_line_cnt * broken_short_b2_line_cnt))	//���߶���
				{
					zone2_alarm_sta = N_BROKEN_STA;
				}
				else if(!(broken_short_b3_line_cnt * broken_short_b4_line_cnt))	//���߶���
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
			broken_short_a4_line_cnt = 0;	//�ѷ���1Ҳ���Ϊ�˷�ֹ�������ͷ�����ʱ�Է���1���߼���ж��и��ŵ�
			broken_short_b1_line_cnt = 0;
			broken_short_b2_line_cnt = 0;
			broken_short_b3_line_cnt = 0;
			broken_short_b4_line_cnt = 0;
	}
}

/*����״̬��ѯ */
void alarm_inquire(void)
{
	if(targe_delay_flag)
	{
		return;
	}
	
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
	float reference_voltage = 1.24f;		//�ο���ѹ
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

void touch_net_dectec(uint8_t zone_num)
{
	float voltage;
	
	float zone1_max_normal_voltage;						//���������ѹ				��·<���ߴ���<����<��·<���ߴ���
	float zone1_min_normal_voltage;						//��С������ѹ
	float zone2_max_normal_voltage;
	float zone2_min_normal_voltage;
	float zone1_short_voltage;
	float zone2_short_voltage;
	
	uint8_t zone1_filter_quantity;
	uint8_t zone2_filter_quantity;
	uint8_t zone1_trigger_quantity;
	uint8_t zone2_trigger_quantity;	
	
	switch(zone1_alarm_delay_time)
	{
		case 90:	zone1_filter_quantity = 3; zone1_trigger_quantity = 1; break;
		case 250:	zone1_filter_quantity = 5; zone1_trigger_quantity = 2; break;
		case 500:	zone1_filter_quantity = 8; zone1_trigger_quantity = 2; break;
		default: break;
	}
	
	switch(zone2_alarm_delay_time)
	{
		case 90:	zone2_filter_quantity = 3; zone2_trigger_quantity = 1; break;
		case 250:	zone2_filter_quantity = 5; zone2_trigger_quantity = 2; break;
		case 500:	zone2_filter_quantity = 8; zone2_trigger_quantity = 2; break;
		default: break;
	}
	
	if(protection_level == HIGH_VOLTAGE_MODE)
	{
		zone1_max_normal_voltage = zone1_high_max_normal_voltage + 0.02;
		zone1_min_normal_voltage = zone1_high_min_normal_voltage - 0.02;
	
		zone2_max_normal_voltage = zone2_high_max_normal_voltage + 0.02;
		zone2_min_normal_voltage = zone2_high_min_normal_voltage - 0.02;
		
		zone1_short_voltage = 0.05;
		zone2_short_voltage = 0.05;
	}
	else if(protection_level == LOW_VOLTAGE_MODE)
	{
		zone1_max_normal_voltage = zone1_low_max_normal_voltage;
		zone1_min_normal_voltage = zone1_low_min_normal_voltage;
		
		zone2_max_normal_voltage = zone2_low_max_normal_voltage;
		zone2_min_normal_voltage = zone2_low_min_normal_voltage;		
		
		zone1_short_voltage = 0.05;
		zone2_short_voltage = 0.05;
	}
	
	voltage = get_max_voltage(zone_num);
	
	if(zone_num == ZONE1)
	{
		if(protection_level == HIGH_VOLTAGE_MODE)
		{
			insert(zone1_high_voltage_buff, voltage, zone1_filter_quantity);
			
			if(!zone1_high_voltage_filter_finish)												//���û������˲������ʮ�Σ�
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
		else if(protection_level == LOW_VOLTAGE_MODE)
		{
			insert(zone1_low_voltage_buff, voltage, zone1_filter_quantity);
			
			if(!zone1_low_voltage_filter_finish)	//���û������˲������ʮ�Σ�
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
		
		if((((voltage > zone1_max_normal_voltage) || (voltage < zone1_min_normal_voltage))&&(voltage > zone1_short_voltage)) && touch_net_mode && zone1_fliter_finish)
		{
			if(++zone1_touch_net_cnt >= zone1_trigger_quantity)
			{	
				zone1_touch_net_cnt = 0;
				zone1_alarm_sta = TOUCH_NET_STA;
				zone1_short_mask = 1;
			}
		}
		else if((voltage > zone1_min_normal_voltage) && (voltage < zone1_max_normal_voltage))
		{
			zone1_short_mask = 1;
			zone1_alarm_sta = NORMAL_STA;	
			zone1_touch_net_cnt = 0;
		}
		else if(voltage < zone1_short_voltage)
		{
			zone1_short_mask = 0;
			zone1_alarm_sta = NORMAL_STA;
			zone1_touch_net_cnt = 0;
		}
	}
	else if(zone_num == ZONE2)
	{
		if(protection_level == HIGH_VOLTAGE_MODE)
		{
			insert(zone2_high_voltage_buff, voltage, zone2_filter_quantity);
			
			if(!zone2_high_voltage_filter_finish)	//���û������˲������ʮ�Σ�
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
		else if(protection_level == LOW_VOLTAGE_MODE)
		{
			insert(zone2_low_voltage_buff, voltage, zone2_filter_quantity);
			
			if(!zone2_low_voltage_filter_finish)	//���û������˲������ʮ�Σ�
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
		
		if((((voltage > zone2_max_normal_voltage) || (voltage < zone2_min_normal_voltage))&&(voltage > zone2_short_voltage)) && touch_net_mode && zone2_fliter_finish)
		{
			if(++zone2_touch_net_cnt >= zone2_trigger_quantity)
			{	
				zone2_short_mask = 1;
				zone2_touch_net_cnt = 0;
				zone2_alarm_sta = TOUCH_NET_STA;
			}
		}
		else if((voltage > zone2_min_normal_voltage) && (voltage < zone2_max_normal_voltage))
		{
			zone2_short_mask = 1;
			zone2_alarm_sta = NORMAL_STA;	
			zone2_touch_net_cnt = 0;
		}
		else if(voltage < zone2_short_voltage)
		{
			zone2_short_mask = 0;
			zone2_alarm_sta = NORMAL_STA;
			zone2_touch_net_cnt = 0;
		}		
	}
	view_voltage = voltage;
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
		HAL_Delay(300);
	}

	for(i=0; i<50; i++)
	{
		primary_boost(LOW_VOLTAGE_MODE);
		HAL_GPIO_WritePin(ZONE1_PULSE_RELEASE_PIN_GPIO_Port, ZONE1_PULSE_RELEASE_PIN_Pin, GPIO_PIN_RESET);
		HAL_Delay(1);
		zone1_low_normal_voltage[i] = get_max_voltage(ZONE1);
		HAL_Delay(20);
		HAL_GPIO_WritePin(ZONE1_PULSE_RELEASE_PIN_GPIO_Port, ZONE1_PULSE_RELEASE_PIN_Pin, GPIO_PIN_SET);
		HAL_Delay(300);
	}
	
	for(i=0; i<50; i++)
	{
		primary_boost(HIGH_VOLTAGE_MODE);
		HAL_GPIO_WritePin(ZONE2_PULSE_RELEASE_PIN_GPIO_Port, ZONE2_PULSE_RELEASE_PIN_Pin, GPIO_PIN_RESET);
		HAL_Delay(1);
		zone2_high_normal_voltage[i] = get_max_voltage(ZONE2);
		HAL_Delay(20);
		HAL_GPIO_WritePin(ZONE2_PULSE_RELEASE_PIN_GPIO_Port, ZONE2_PULSE_RELEASE_PIN_Pin, GPIO_PIN_SET);
		HAL_Delay(300);
	}
	
	for(i=0; i<50; i++)
	{
		primary_boost(LOW_VOLTAGE_MODE);
		HAL_GPIO_WritePin(ZONE2_PULSE_RELEASE_PIN_GPIO_Port, ZONE2_PULSE_RELEASE_PIN_Pin, GPIO_PIN_RESET);
		HAL_Delay(1);
		zone2_low_normal_voltage[i] = get_max_voltage(ZONE2);
		HAL_Delay(20);
		HAL_GPIO_WritePin(ZONE2_PULSE_RELEASE_PIN_GPIO_Port, ZONE2_PULSE_RELEASE_PIN_Pin, GPIO_PIN_SET);
		HAL_Delay(300);
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




































