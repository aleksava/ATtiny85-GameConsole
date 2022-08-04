/*
 * OLED.h
 *
 * Created: 04.08.2022 22.16.10
 *  Author: aleks
 */ 


#ifndef OLED_H_
#define OLED_H_


void OLED_init(void);

void OLED_send_byte(uint8_t byte);

void OLED_send_command(uint8_t command);

void OLED_send_data_start(void);

void OLED_send_data_stop(void);

void OLED_setpos(uint8_t x, uint8_t y);
void OLED_fillscreen(uint8_t fill_Data);


#endif /* OLED_H_ */