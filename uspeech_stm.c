#include "uspeech_stm.h"

static uint16_t US_RdSample(void){
	TIM_SetCounter(US_TIM, 0);
	ADC_StartOfConversion(US_ADC);
	while(!ADC_GetFlagStatus(US_ADC, ADC_FLAG_EOC));
	while(TIM_GetCounter(US_TIM)<100);	//Ensure ~100us has passed

	return ADC_GetConversionValue(US_ADC);
}

void uSpeech_hwInit(void){
	GPIO_InitTypeDef G;
	ADC_InitTypeDef A;
	TIM_TimeBaseInitTypeDef T;

	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM15, ENABLE);

	G.GPIO_Pin = US_ADCGPIN;
	G.GPIO_Mode = GPIO_Mode_AN;
	G.GPIO_OType = GPIO_OType_PP;
	G.GPIO_PuPd = GPIO_PuPd_NOPULL;
	G.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_Init(US_ADCGPIO, &G);

	ADC_StructInit(&A);
	A.ADC_ContinuousConvMode = DISABLE;
	A.ADC_DataAlign = ADC_DataAlign_Right;
	A.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_None;
	A.ADC_Resolution = ADC_Resolution_12b;
	ADC_Init(US_ADC, &A);
	ADC_Cmd(US_ADC, ENABLE);
	ADC_ChannelConfig(US_ADC, US_ADCCHAN, ADC_SampleTime_239_5Cycles);

	//1us timebase
	T.TIM_ClockDivision = TIM_CKD_DIV1;
	T.TIM_CounterMode = TIM_CounterMode_Up;
	T.TIM_Period = 0xFFFF;
	T.TIM_Prescaler = SystemCoreClock/1000000;
	T.TIM_RepetitionCounter = 0;
	TIM_TimeBaseInit(US_TIM, &T);
	TIM_Cmd(US_TIM, ENABLE);
}

void uSpeech_signalInit(signal *s){
	s->fconstant = F_CONSTANT;
	s->econstant = 2;
	s->aconstant = 4;
	s->vconstant = 6;
	s->shconstant = 10;
	s->amplificationFactor = 10;
	s->micPowerThreshold = 50;
	s->scale = 1;
}

void uSpeech_calibrate(signal *s){
	s->calib = 0;
	uint32_t samp = 0;
	uint16_t ind;

	for(ind = 0; ind<10000; ind++){
		samp += US_RdSample()*s->scale;
	}

	s->calib = samp/10000;
}

void uSpeech_sample(signal *s){
	uint8_t i = 0;

	while(i<32){
		s->arr[i] = (US_RdSample()*s->scale - s->calib);
		i++;
	}
}

uint32_t uSpeech_power(signal *s){
	uint32_t j = 0;
	uint8_t i = 0;

	while(i<32){
		j+=abs(s->arr[i]);
		i++;
	}

	return j;
}

uint32_t uSpeech_complexity(signal *s){
	uint32_t j = 0;
	uint8_t i = 1;

	while(i<32){
		j+=abs(s->arr[i] - s->arr[i-1]);
		i++;
	}

	return j;
}

uint32_t uSpeech_maxPower(signal *s){
	uint32_t max = 0;
	uint8_t i = 0;

	while(i<32){
		if(max<abs(s->arr[i])){
			max = abs(s->arr[i]);
			s->maxPos = i;
		}
		i++;
		s->avgPower += s->arr[i];
	}

	s->avgPower /= 32;
	return max;
}

int32_t uSpeech_snr(signal *s, int32_t power){
	uint8_t i = 0, j = 0;
	int mean = power/32;

	while(i<32){
		j+=sq(s->arr[i]-mean);
		i++;
	}

	return sqrtf(j/mean)/power;
}

char uSpeech_getPhoneme(signal *s){
	uSpeech_sample(s);

	uint32_t pp = uSpeech_power(s);

	if(pp>SILENCE){
		int32_t k = uSpeech_complexity(s);
		s->overview[6] = s->overview[5];
		s->overview[5] = s->overview[4];
		s->overview[4] = s->overview[3];
		s->overview[3] = s->overview[2];
		s->overview[2] = s->overview[1];
		s->overview[1] = s->overview[0];
		s->overview[0] = k;

		int32_t coeff = 0;
		uint8_t f;

		for(f = 0; f<6; f++){
			coeff += s->overview[f];
		}

		coeff /= 7;

		s->micPower = 0.05 * uSpeech_maxPower(s) + (1-0.05) * s->micPower;

		s->testCoeff = coeff;

		if(coeff<s->econstant){
			s->phoneme = 'e';
		}
		else if(coeff<s->aconstant){
			s->phoneme = 'o';
		}
		else if(coeff<s->vconstant){
			s->phoneme = 'v';
		}
		else if(coeff<s->shconstant){
			s->phoneme = 'h';
		}
		else{
			s->phoneme = 's';
		}

		if(s->f_enabled){
			if(s->micPower > s->fconstant){
				return 'f';
			}
		}
		return s->phoneme;
	}
	else{
		s->micPower = 0;
		s->testCoeff = 0;
		return ' ';
	}
}
