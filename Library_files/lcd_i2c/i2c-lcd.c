
/** Put this in the src folder **/
#include "i2c-lcd.h"
extern I2C_HandleTypeDef hi2c1;  // change your handler here accordingly

#define SLAVE_ADDRESS_LCD 0x4E // change this according to ur setup

const uint8_t data_init_sequence[9]={0x30, 0x30, 0x30, 0x20, 0x28, 0x08, 0x01, 0x06, 0x0C};
const uint8_t time_init_sequence[9]={5, 1, 10, 10, 1, 1, 1, 1, 1};

void lcd_send_cmd (char cmd)
{
	// P7 P6 P5 P4 P3 P2 P1 P0
	// D7 D6 D5 D4 BL E  RW RS
	// obs1: quando é comando, instrução, RS=0;
	// obs2: coloca-se dado, Rw=0, RS=0; backlight=1; demais em 0; e dá-se pulso em E para escrita da instrução, logo...
	// Para escrever comando: coloca nibble alto combinando com OU os bits de função E, RW, RS e backlight, função, ou seja:
	//                        coloca E=1, rw=0; RS=0; backlight=BL=1; demais =0 --> 0x0C = 0000 1100
	//                        coloca E=0, rw=0; RS=0; backlight=BL=1; demais =0 --> 0x0C = 0000 1000
	char data_u, data_l;
	uint8_t data_t[4];

	data_u = (cmd&0xf0);		//comando AND com 0xF0 (1111 0000) apaga informações do nibble baixo
	data_l = ((cmd<<4)&0xf0);   //desloca 4 bits e faz o mesmo, limpando os 4 bits baixos

	data_t[0] = data_u|0x0C;  //en=1, rs=0 -- combina comandos de escrita do nibble baixo com o próprio comando (nibble alto)
	data_t[1] = data_u|0x08;  //en=0, rs=0 -- combina comando de escrita com E=0;
	data_t[2] = data_l|0x0C;  //en=1, rs=0 -- combina comandos de escrita do nibble baixo com o próprio comando (nibble baixo)
	data_t[3] = data_l|0x08;  //en=0, rs=0 -- combina comando de escrita com E=0;
	HAL_I2C_Master_Transmit (&hi2c1, SLAVE_ADDRESS_LCD,(uint8_t *) data_t, 4, 100);
}

void lcd_send_data (char data)
{
	// P7 P6 P5 P4 P3 P2 P1 P0
	// D7 D6 D5 D4 BL E  RW RS
	// obs1: quando é dado, RS=1;
	// obs2: coloca-se dado, Rw=0, RS=1; backlight=1; demais em 0; e dá-se pulso em E para escrita da instrução, logo...
	// Para escrever comando: coloca nibble alto combinando com OU os bits de função E, RW, RS e backlight, função, ou seja:
	//                        coloca E=1, rw=0; RS=1; backlight=BL=1; demais =0 --> 0x0D = 0000 1101
	//                        coloca E=0, rw=0; RS=1; backlight=BL=1; demais =0 --> 0x09 = 0000 1001
	char data_u, data_l;
	uint8_t data_t[4];

	data_u = (data&0xf0);		//comando AND com 0xF0 (1111 0000) apaga informações do nibble baixo
	data_l = ((data<<4)&0xf0);   //desloca 4 bits e faz o mesmo, limpando os 4 bits baixos

	data_t[0] = data_u|0x0D;  //en=1, rs=0 -- combina comandos de escrita do nibble baixo com o próprio comando (nibble alto)
	data_t[1] = data_u|0x09;  //en=0, rs=0 -- combina comando de escrita com E=0;
	data_t[2] = data_l|0x0D;  //en=1, rs=0 -- combina comandos de escrita do nibble baixo com o próprio comando (nibble baixo)
	data_t[3] = data_l|0x09;  //en=0, rs=0 -- combina comando de escrita com E=0;
	HAL_I2C_Master_Transmit (&hi2c1, SLAVE_ADDRESS_LCD,(uint8_t *) data_t, 4, 100);
}

void lcd_clear (void)
{
	lcd_send_cmd(0x01);
	HAL_Delay(1);
}

void lcd_posiciona(int row, int col)
{
	int ad_code=0;
    switch (row)
    {
        case 0:
        	ad_code=0x00;
            break;
        case 1:
        	ad_code=0x40;
            break;
        case 2:
            ad_code=0x14;
            break;
        case 3:
            ad_code=0x54;
            break;

    }
    col += ad_code;
    // é preciso ativar d7 para que o endereço de 7bits da ddram esteja completo
    // ou seja: rs rw ad7 ad6 ad5 ad4 ad3 ad2 ad1 ad0
    //          0  0  1   ADD ADD ADD ADD ADD ADD ADD
    // então, faz-se OU com 0x80 = 1000 0000, ligando o bit7 independente do que houver nos demais.
    lcd_send_cmd (col|0x80);
}


void lcd_init (void)
{
	/* Inicialização do display em 4bits, 5x8
		Conforme página 46, figura 24 do datasheet Hitachi HD44780U
		Espera >40ms
		(0x30); Envia 3 para bits altos, logo 0x30 (considerando palavras de 8bits
		Espera >4.1ms
		(0x30); Envia 3 para bits altos, logo 0x30 (considerando palavras de 8bits
		Espera >100us
		(0x30); Envia 3 para bits altos, logo 0x30 (considerando palavras de 8bits
		Espera >1ms
		(0x20); Envia 3 para bits altos, logo 0x30 (considerando palavras de 8bits
		Espera >1ms
		(0x28); Function set --> DL=0 (modo 4 bits), N = 1 (2 display 2 linhas) F = 0 (caracteres 5x8)
		Espera >1ms
		(0x08); Controle Display on/off  --> D=0,C=0, B=0  ---> display off
		Espera >1ms
		(0x01); limpa display
		Espera >1ms
		(0x06); Mode set --> I/D = 1 (incrementa cursor) & S = 0 (sem shift)
		Espera >1ms
		(0x0C); Controle Display on/off --> D = 1, C and B = 0. (Cursor and blink, últimos 2 bits)
		Espera >1ms
	*/

	HAL_Delay(50);  // wait for >40ms
	for (int x=0; x<sizeof(data_init_sequence); x++)
	{
		lcd_send_cmd(data_init_sequence[x]);
		HAL_Delay(time_init_sequence[x]);  // de acordo com datasheet hitachi
	}


}

void lcd_send_string (char *str)
{
	while (*str)
		lcd_send_data (*str++);
}
