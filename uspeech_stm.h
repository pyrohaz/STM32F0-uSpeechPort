#ifndef USPEECH_STM_H
#define USPEECH_STM_H

#include <stm32f0xx_gpio.h>
#include <stm32f0xx_adc.h>
#include <stm32f0xx_rcc.h>
#include <stm32f0xx_tim.h>
#include <math.h>

/* A port of Arjo's uSpeech  library for the STM32F0 Discovery board
 * Date: 6-11-16
 *
 * Original source: https://github.com/arjo129/uSpeech
 *
 * All code concepts and ideas originally developed by Arjo and the license for
 * his sections of code can be found at: https://github.com/arjo129/uSpeech/blob/master/LICENSE.txt
 *
 * Information on my implementation of this library can be found at:
 * http://hsel.co.uk/2016/01/06/stm32f0-uspeech-port/
 *
 * I take no responsibility for the uses of this code as this is merely a port.
 */

//STM32 specifics
#define US_ADCGPIN		GPIO_Pin_0
#define US_ADCCHAN		ADC_Channel_0
#define US_ADCGPIO		GPIOA
#define US_ADC			ADC1
#define US_TIM			TIM15

//Arduino specific defines
#define sq(x) ((x)*(x))

#define SILENCE 2000
#define F_DETECTION 3
#define F_CONSTANT 350
#define MAX_PLOSIVETIME 1000
#define PROCESS_SKEWNESS_TIME 15

typedef struct{
	int32_t arr[32];  /*!< This is the audio buffer*/
	int32_t avgPower;
	int32_t testCoeff;
	int32_t minVolume;  /*!< This is the highest audio power that should be considered ready */
	int32_t fconstant;  /*!< This is the threshold for /f/, configure it yourself */
	int32_t econstant;  /*!< This is the threshold for /ee/, /i/, configure it yourself */
	int32_t aconstant;  /*!< This is the threshold for /a/ /o/ /r/ /l/, configure it yourself */
	int32_t vconstant;  /*!< This is the threshold for /z/ /v/ /w/, configure it yourself */
	int32_t shconstant; /*!< This is the threshold for /sh/ /ch/, above this everything else is regarded as /s/ */
	uint8_t f_enabled; /*!< Set this to false if you do not want to detect /f/s */
	int32_t amplificationFactor; /*!< Amplification factor: Adjust as you need*/
	int32_t micPowerThreshold; /*!< Ignore anything with micPower below this */
	int32_t scale;
	char phoneme;	/*!< The phoneme detected when f was returned */
	int32_t micPower;
	int32_t calib;

	int32_t mil;
	int32_t maxPos;
	uint8_t silence;
	uint32_t overview[7];
} signal;

void uSpeech_hwInit(void);
void uSpeech_signalInit(signal *s);
void uSpeech_calibrate(signal *s);
void uSpeech_sample(signal *s);
uint32_t uSpeech_power(signal *s);
uint32_t uSpeech_complexity(signal *s);
uint32_t uSpeech_maxPower(signal *s);
int32_t uSpeech_snr(signal *s, int32_t power);
char uSpeech_getPhoneme(signal *s);


#endif
