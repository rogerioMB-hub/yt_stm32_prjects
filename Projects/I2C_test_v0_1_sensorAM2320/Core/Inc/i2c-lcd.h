#include "stm32f1xx_hal.h"

void lcd_init (void);   				// inicializa o LCD

void lcd_send_cmd (char cmd);  			// envia instrução para o LCD

void lcd_send_data (char data); 		// envia dado para o LCD

void lcd_send_string (char *str);   	// envia string para o LCD

void lcd_posiciona(int row, int col);  	// posiciona cursor no display (linha <0 a 3>; coluna <0 a 19>);

void lcd_clear (void);					// limpa display LCD
