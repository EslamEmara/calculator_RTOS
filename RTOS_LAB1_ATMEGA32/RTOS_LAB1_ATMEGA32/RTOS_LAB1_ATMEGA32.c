/*
 * RTOS_LAB1_ATMEGA32.c
 *
 * Created: 1/27/2014 12:22:11 AM
 *  Author: Islam
 */ 
#define F_CPU 8000000
#include <avr/io.h>
#include "FreeRTOS.h"
#include "task.h"
#include "FreeRTOSConfig.h"
#include "lcd_4bit.h"
#include "KeyPad_int.h"
#include "semphr.h"
#include "DIO_int.h"
#include <math.h>
#include <stdio.h>

void welcome (void *p);
void get_button (void *p);
void calc (void *p);
void led (void *p);
void calc_operation (void);
void led_operation (void);
void reset_button (void *p);

xSemaphoreHandle calc_start;
xSemaphoreHandle button_data;
xSemaphoreHandle led_start;

xTaskHandle welcome_h;

unsigned char start_button = 0;
unsigned char pressed_button = 255;

#define PUSH_BUTTON 29
int main ()
{
	xTaskHandle get_button_h;
	xTaskHandle calc_h;
	xTaskHandle led_h;
	xTaskHandle reset_h;

	xTaskCreate(welcome,NULL,150,NULL,4,&welcome_h);
	xTaskCreate(get_button,NULL,150,NULL,3,&get_button_h);
	xTaskCreate(calc,NULL,150,NULL,2,&calc_h);
	xTaskCreate(led,NULL,100,NULL,2,&led_h);
	xTaskCreate(reset_button,NULL,100,NULL,2,&reset_h);
	
	vSemaphoreCreateBinary(calc_start);
	xSemaphoreTake(calc_start,portMAX_DELAY);
	vSemaphoreCreateBinary(led_start);
	xSemaphoreTake(led_start,portMAX_DELAY);
	vSemaphoreCreateBinary(button_data);
	xSemaphoreTake(button_data,portMAX_DELAY);

	DDRD |= (1<<7);
	DDRD &= ~(1<<5);
	lcd_init();
	vTaskStartScheduler();

	return 0;
}
void welcome (void *p)
{
	int count = 0;
	for(int j=0;j<3;j++)
	{
		for (int i =0 ; i<8 ; i++)
		{
			if(i!=0)
			lcd_disp_string_xy(" " , 0 , i-1);
			lcd_disp_string_xy("Welcome" , 0 , i);
			vTaskDelay(500/13);
		}
		lcd_clrScreen();
		for (int i =9 ; i>0 ; i--)
		{
			lcd_disp_string_xy(" " , 0 , i+7);
			lcd_disp_string_xy("Welcome" , 0 , i);
			vTaskDelay(500/13);
		}
		lcd_clrScreen();
	}
	
	for(;;)
	{
		lcd_disp_string_xy("press any key to" , 0 , 0);
		lcd_disp_string_xy("continue" , 1 , 0);
		vTaskDelay(500);
		lcd_clrScreen();
		vTaskDelay(250);
			count++;
		
		if (count == 13)
		{
			xSemaphoreGive(led_start);
			vTaskDelete(NULL);
		}
	}
	vTaskDelete(NULL);
}
void get_button (void *p)
{
	unsigned char current_button = 255;
	portTickType wait = xTaskGetTickCount();
	vTaskDelay(3000);
	while (1)
	{
		vTaskDelayUntil (&wait , 10);
		pressed_button = KeyPad_u8GetPressedKey();
		if (pressed_button != 255)
		{
			if(current_button != pressed_button)
			{
				if(start_button == 0)
				{
					vTaskDelete(welcome_h);
					start_button = 1;
					xSemaphoreGive(calc_start);
				}
				else
				{
					xSemaphoreGive(button_data);
				}
				current_button = pressed_button;
			}
		}
		if (pressed_button == 255)
		current_button = 255;
	}
	vTaskDelete(NULL);
}

void calc (void *p)
{
	static unsigned char calc_started = 0;
	portTickType wait = xTaskGetTickCount();
	vTaskDelay(3000);

	for(;;)
	{
		vTaskDelayUntil (&wait , 100);
		if(calc_started == 0)
		{
			if (xSemaphoreTake(calc_start,portMAX_DELAY) == pdTRUE)
			{
				calc_started = 1;
				lcd_clrScreen();
				lcd_displayChar('0');
				continue;
			}
		}
		else
		{
			calc_operation();
		}

	}
	vTaskDelete(NULL);
}

void led (void *p)
{
	unsigned char led_started = 0;
	portTickType wait = xTaskGetTickCount();
	vTaskDelay(3000);
	while (1)
	{
		vTaskDelayUntil (&wait , 25);
		if (led_started == 0)
		{
			if (xSemaphoreTake(led_start,portMAX_DELAY) == pdTRUE)
			{	
				led_started = 1;
				lcd_clrScreen();
				lcd_disp_string_xy("Stand By",0,0);
				led_operation();
			}
		}
		else
		{
		
		}
	}
	vTaskDelete(NULL);
}
void calc_operation(void)
{	
	static unsigned char operator1[16]={0};
	static unsigned char operator1_count = 0;
	static unsigned char operator2[16]={0};
	static unsigned char operator2_count = 0;
	static unsigned char equation_op=1;
	static unsigned char operator1_complete = 0;
	static int result=0;
	static int operator1_int=0;
	static int operator2_int=0;
	static unsigned char current_position = 0;
	
		if (xSemaphoreTake(button_data , portMAX_DELAY) == pdTRUE)
		{	
			if (pressed_button <= '9' && pressed_button >= '0')
			{
				lcd_gotoxy(0,current_position);
				current_position++;
				lcd_displayChar(pressed_button);
				
				if (operator1_complete == 0)
				{
					operator1[operator1_count]=pressed_button;
					operator1_count++;
			
				}
				else 
				{
					operator2[operator2_count]=pressed_button;
					operator2_count++;
				}
			}
			else
			{
				if (pressed_button == '=')
				{	
					operator1_int = atoi(operator1);
					operator2_int = atoi(operator2);
					switch (equation_op)
					{
						case '+':
							result = operator1_int+operator2_int;
						break;
						case '-':
							result = operator1_int-operator2_int;
						break;
						case '*':
							result = operator1_int*operator2_int;
						break;													
						case '/':
							if(operator2_int == 0)
							{
								lcd_disp_string_xy("Math Error",0,0);
								vTaskDelay(500);	
								lcd_clrScreen();
								lcd_displayChar('0');
								current_position =0;
								operator1_complete = 0;
								operator1_count = 0;
								for (int i = 0 ; i< 16 ; i++)
								{
									operator1[i] = 0;
									operator2[i] = 0;
								}
								operator2_count =0;
								result = 0;
							}
							else
								result = operator1_int/operator2_int;
						break;						
					}
					
					lcd_clrScreen();
					dispaly_integer(result,operator1_count+operator2_count);

					sprintf(operator1, "%d", result);
					for (int i = 0 ; i< 16 ; i++)
					{
						operator2[i] = 0;
					}
					operator2_count =0;
				}
				else if (pressed_button == 13)
				{
					lcd_clrScreen();
					
					lcd_displayChar('0');
					current_position =0;
					operator1_complete = 0;
					operator1_count = 0;
					for (int i = 0 ; i< 16 ; i++)
					{
						operator1[i] = 0;
						operator2[i] = 0;
					}
					operator2_count =0;
					result = 0;
				}
				else 
				{
					lcd_clrScreen();
					current_position=0;
						equation_op = pressed_button;
						operator1_complete =1;
				}
			
			}
		}
}
void dispaly_integer (int n, int n_count)
{	
	unsigned char str[16];
	 sprintf(str, "%d", n);
	 for(int count = 0 ; count < n_count ; count++)
	 {
		 lcd_displayChar((unsigned char)str[count]);
	 }

}
void pwm_init()
{
	TCCR2 |= (1<<WGM00)|(1<<COM01)|(1<<WGM01)|(1<<CS00);
	
	DDRD |= (1<<7);
	OCR2 = 0;
}

void led_operation(void)
{
	
	pwm_init();

for ( int brightness = 0 ; brightness < 255 ; brightness += 10)
{
	OCR2 = brightness;
	vTaskDelay(10);
}
vTaskDelay(250);
for ( int brightness = 255 ; brightness > 0 ; brightness -= 10)
{
	OCR2 = brightness;
	vTaskDelay(10);
}
vTaskDelay(250);

TCCR2 = 0x00;
	lcd_clrScreen();
}
void reset_button (void *p)
{
	portTickType wait = xTaskGetTickCount();
	static unsigned char counter = 0; 
	vTaskDelay(3000);
	while (1)
	{
		vTaskDelayUntil(&wait , 100);
		if (DIO_u8GetPin(PUSH_BUTTON) == DIO_HIGH)
		{
			counter++;
		}
		if(counter == 30)
		{
			xSemaphoreGive(led_start);
			break;
		}
	}
	vTaskDelete(NULL);
}