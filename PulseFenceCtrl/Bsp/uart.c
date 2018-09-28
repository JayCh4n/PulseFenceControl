#include "uart.h"

extern UART_HandleTypeDef huart1;

uint8_t uart1_tx_data[50];
uint8_t uart1_rx_data[50];
uint8_t uart1_rx_buff;

void uart1_deal(uint8_t *data_package)
{
//	uint8_t package_lenth;
	uint8_t command;
	
//	package_lenth = data_package[2] + 5;
	command = data_package[3];
	
	switch(command)
	{
		case AMING_DISARM: 
			arming_disarm(data_package[4]);
			zone1_touch_net_cnt = 0;
			zone2_touch_net_cnt = 0;
			send_sta_msg(AMING_DISARM, data_package[4]); 
			break;
		case SINGLE_DOUBLE_ZONE: zone_mode = data_package[4];	send_sta_msg(SINGLE_DOUBLE_ZONE, data_package[4]);
			break;
		case HIGH_LOW_VOLTAGE: protection_level = data_package[4]; send_sta_msg(HIGH_LOW_VOLTAGE, data_package[4]);
			break;
		case ZONE1_SENSITIVITY: set_sensitivity(ZONE1, data_package[4]); send_sta_msg(ZONE1_SENSITIVITY, data_package[4]); 
			break;
		case ZONE2_SENSITIVITY: set_sensitivity(ZONE2, data_package[4]); send_sta_msg(ZONE2_SENSITIVITY, data_package[4]);
			break;
		case TOUCH_NET:	touch_net_mode = data_package[4]; send_sta_msg(TOUCH_NET, data_package[4]);
			break;
		case AUTO_DETECT: auto_detect_sta = 1; send_sta_msg(AUTO_DETECT, data_package[4]); break;
		default: break;
	}
}

/*zone_sta: 0x11 防区1状态  0x12 防区2状态*/
void send_sta_msg(uint8_t sta_type, uint8_t sta)
{
	uart1_tx_data[0] = 0xA5;
	uart1_tx_data[1] = 0x5A;
	uart1_tx_data[2] = 0x02;
	uart1_tx_data[3] = sta_type;
	uart1_tx_data[4] = sta;
	uart1_tx_data[5] = 0x00;
	uart1_tx_data[6] = 0x00;		/*校验位暂时不加*/
	HAL_UART_Transmit_IT(&huart1, uart1_tx_data, 7);
}

