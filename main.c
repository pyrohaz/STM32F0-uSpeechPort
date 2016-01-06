#include "uspeech_stm.h"

/*
 * STM32F0 Discovery implementation of LEDTest.ino, originally written by Arjo
 * for his uSpeech library.
 *
 * Date: 6-1-16
 *
 * Information on my implementation of this library can be found at:
 * http://hsel.co.uk/2016/01/06/stm32f0-uspeech-port/
 */

signal voice;

int main(void)
{
	GPIO_InitTypeDef G;
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOC, ENABLE);

	//Onboard LED
	G.GPIO_Pin = GPIO_Pin_9;
	G.GPIO_Mode = GPIO_Mode_OUT;
	G.GPIO_OType = GPIO_OType_PP;
	G.GPIO_PuPd = GPIO_PuPd_NOPULL;
	G.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_Init(GPIOC, &G);

	uSpeech_hwInit();
	uSpeech_signalInit(&voice);

	voice.f_enabled = 1;
	voice.minVolume = 1500;
	voice.fconstant = 400;
	voice.econstant = 1;
	voice.aconstant = 2;
	voice.vconstant = 3;
	voice.shconstant = 4;

	uSpeech_calibrate(&voice);

	char p;
	uint8_t newline = 0;
	while(1)
	{
		uSpeech_sample(&voice);
		p = uSpeech_getPhoneme(&voice);

		if(p != ' '){
			if(p == 'f'){	//Turn LED on
				newline = 0;
			}
			else{
				newline = 1;
			}
		}
		else{
			if(newline){
				GPIO_ResetBits(GPIOC, GPIO_Pin_9);
			}
			else{
				GPIO_SetBits(GPIOC, GPIO_Pin_9);
			}
		}
	}
}
