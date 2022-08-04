/*
 * OLED.c
 *
 * Created: 04.08.2022 22.15.56
 *  Author: aleks
 */ 


#include "i2c.h"
#include "OLED.h"

#define OLED_ADDRESS 0x3C

void OLED_init(void) {
	OLED_send_command(0xAE); // display off
	OLED_send_command(0x00); // Set Memory Addressing Mode
	OLED_send_command(0x10); // 00,Horizontal Addressing Mode;01,Vertical Addressing Mode;10,Page Addressing Mode (RESET);11,Invalid
	OLED_send_command(0x40); // Set Page Start Address for Page Addressing Mode,0-7
	OLED_send_command(0x81); // Set COM Output Scan Direction
	OLED_send_command(0xCF); // ---set low rowumn address
	OLED_send_command(0xA1); // ---set high rowumn address
	OLED_send_command(0xC8); // --set start line address
	OLED_send_command(0xA6); // --set contrast control register
	OLED_send_command(0xA8);
	OLED_send_command(0x3F); // --set segment re-map 0 to 127
	OLED_send_command(0xD3); // --set normal display
	OLED_send_command(0x00); // --set multiplex ratio(1 to 64)
	OLED_send_command(0xD5); //
	OLED_send_command(0x80); // 0xa4,Output follows RAM content;0xa5,Output ignores RAM content
	OLED_send_command(0xD9); // -set display offset
	OLED_send_command(0xF1); // -not offset
	OLED_send_command(0xDA); // --set display clock divide ratio/oscillator frequency
	OLED_send_command(0x12); // --set divide ratio
	OLED_send_command(0xDB); // --set pre-charge period
	OLED_send_command(0x40); //
	OLED_send_command(0x20); // --set com pins hardware configuration
	OLED_send_command(0x02);
	OLED_send_command(0x8D); // --set vcomh
	OLED_send_command(0x14); // 0x20,0.77xVcc
	OLED_send_command(0xA4); // --set DC-DC enable
	OLED_send_command(0xA6); //
	OLED_send_command(0xAF); // --turn on oled panel
}


void OLED_send_byte(uint8_t byte) {
	i2c_write(byte);
}

void OLED_send_command(uint8_t command) {
	i2c_start(OLED_ADDRESS,0);			
	i2c_write(0x00);					// write command
	i2c_write(command);
	i2c_stop();
}

void OLED_send_data_start(void) {
	i2c_start(OLED_ADDRESS,0);
	i2c_write(0x40);					// write data
}

void OLED_send_data_stop(void) {
	i2c_stop();
}

void OLED_setpos(uint8_t x, uint8_t y)
{
	if (y > 7) return;
	i2c_start(OLED_ADDRESS, 0);
	OLED_send_byte(0x00);  //write command

	OLED_send_byte(0xb0 + y);
	OLED_send_byte(((x & 0xf0) >> 4) | 0x10); // |0x10
	OLED_send_byte((x & 0x0f) | 0x01); // |0x01

	OLED_send_data_stop();
}

void OLED_fillscreen(uint8_t fill_Data) {
	uint8_t m, n;
	for (m = 0; m < 8; m++)
	{
		OLED_send_command(0xb0 + m); //page0-page1
		OLED_send_command(0x00);   //low rowumn start address
		OLED_send_command(0x10);   //high rowumn start address
		OLED_send_data_start();
		for (n = 0; n < 128; n++)
		{
			OLED_send_byte(fill_Data);
		}
		OLED_send_data_stop();
	}
}

//void OLED_char_f6x8(uint8_t x, uint8_t y, const char ch[]) {
	//uint8_t c, i, j = 0;
	//while (ch[j] != '\0')
	//{
		//c = ch[j] - 32;
		//if (c > 0) c = c - 12;
		//if (c > 15) c = c - 6;
		//if (c > 40) c = c - 9;
		//if (x > 126)
		//{
			//x = 0;
			//y++;
		//}
		//OLED_setpos(x, y);
		//OLED_send_data_start();
		//for (i = 0; i < 6; i++)
		//{
			//OLED_send_byte(pgm_read_byte(&ssd1306xled_font6x8[c * 6 + i]));
		//}
		//OLED_send_data_stop();
		//x += 6;
		//j++;
	//}
//}