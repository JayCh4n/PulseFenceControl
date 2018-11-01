#include "uart.h"

extern UART_HandleTypeDef huart1;

uint8_t uart1_tx_data[50];
uint8_t uart1_rx_data[50];
uint8_t uart1_rx_buff;

void uart1_deal(uint8_t *data_package)
{
	uint8_t command, zone_num, data;
	command = data_package[3];
	zone_num = data_package[4];
	data = data_package[5];
	
	switch(command)
	{
		case AMING_DISARM: 
			arming_disarm(zone_num, data);
			return_set_msg(AMING_DISARM, zone_num, data);
			break;
		case ZONE_TYPE: zone_type = data;
			return_set_msg(ZONE_TYPE, zone_num, data);
			break;
		case HIGH_LOW_VOLTAGE:
			set_protection_level(zone_num, data);
			return_set_msg(HIGH_LOW_VOLTAGE, zone_num, data);
			break;
		case SENSITIVITY: set_sensitivity(zone_num, data); return_set_msg(SENSITIVITY, zone_num, data);
			break;
		case ZONE_MODE:
			set_zone_mode(zone_num, data);
			return_set_msg(ZONE_MODE, zone_num, data);
			break;
		case AUTO_DETECT: auto_detect_sta = 1; return_set_msg(AUTO_DETECT, zone_num, data);
			break;
		case TARGE_DELAY:
			break;
		case 0x20:
			get_init_value(data_package);
		break;
		case 0x30:return_sta_msg(); break;
		default: break;
	}
}

void return_set_msg(uint8_t cmd, uint8_t zone_num, uint8_t sta)
{
	uart1_tx_data[0] = 0xA5;
	uart1_tx_data[1] = 0x5A;
	uart1_tx_data[2] = 0x03;
	uart1_tx_data[3] = cmd;
	uart1_tx_data[4] = zone_num;
	uart1_tx_data[5] = sta;
	uart1_tx_data[6] = 0x00;
	uart1_tx_data[7] = 0x00;
	HAL_UART_Transmit(&huart1, uart1_tx_data, 8, 1000);
}


void get_init_value(uint8_t *data_package)
{
	zone_type = data_package[4];
	zone1_protection_level = data_package[5];
	zone2_protection_level = data_package[6];
	zone1_sensitivity = data_package[7];
	zone2_sensitivity = data_package[8];
	zone1_mode = data_package[9];
	zone2_mode = data_package[10];
}

void return_sta_msg(void)
{
	uart1_tx_data[0] = 0xA5;
	uart1_tx_data[1] = 0x5A;
	uart1_tx_data[2] = 0x03;
	uart1_tx_data[3] = 0x30;
	uart1_tx_data[4] = (uint8_t)zone1_alarm_sta;
	uart1_tx_data[5] = (uint8_t)zone2_alarm_sta;
	uart1_tx_data[6] = 0x00;
	uart1_tx_data[7] = 0x00;	
	
	HAL_UART_Transmit(&huart1, uart1_tx_data, 8, 1000);
}







