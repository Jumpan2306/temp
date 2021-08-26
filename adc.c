#include "adc.h"


#define VREF 3.3
#define FULL_SCALE 4096

#define BMC_12V (VREF / FULL_SCALE * 136 / 36) //12V��ѹת����ֵ
#define BMC_5V 	  (VREF / FULL_SCALE )	//5V��ѹת����ֵ

#define ADDR 	 ((VREF / FULL_SCALE))	//������ѹת����ֵ
float adc_value[3];
uint16_t initial_value[3];
float conversion_value[3] = {BMC_12V , BMC_5V , ADDR};


void dma_adc_init(void)
{
	dma_parameter_struct dma_adc;											//DMA�ṹ��
	
	dma_deinit(DMA_CH0);													//��λDMAͨ��x�����мĴ���
	dma_struct_para_init(&dma_adc);											//��DMA�ṹ�������в�����ʼ��ΪĬ��ֵ
	dma_adc.memory_addr = (uint32_t)(initial_value);						//�洢������ַ
	dma_adc.memory_inc = 	DMA_MEMORY_INCREASE_ENABLE;						//�洢����ַ�����㷨ģʽ
	dma_adc.memory_width = DMA_MEMORY_WIDTH_16BIT;							//�洢�����ݴ����� //�������ݵ��ڴ��СΪ16λ
	dma_adc.periph_addr  = (uint32_t)(&ADC_RDATA);							//�������ַ         //�������ݼĴ���
	dma_adc.periph_inc = DMA_PERIPH_INCREASE_DISABLE;						//�����ַ�����㷨ģʽ
	dma_adc.periph_width = DMA_PERIPHERAL_WIDTH_16BIT;						//�������ݴ����� //�������ݵ��ڴ��СΪ16λ
	dma_adc.direction = DMA_PERIPHERAL_TO_MEMORY;							//DMAͨ�����ݴ��䷽��// ����Χ�豸��ȡ��д���ڴ�
	dma_adc.number = 3U;													//DMAͨ�����ݴ�������
	dma_adc.priority = DMA_PRIORITY_HIGH;									//DMAͨ������������ȼ�
	dma_circulation_enable(DMA_CH0);										//DMAѭ��ģʽʹ�� 			DMA_CH0 DMAͨ��
	dma_init(DMA_CH0, &dma_adc);											//��ʼ��DMAͨ��x 			DMA_CH0 DMAͨ��   dma_adc  dma_parameter_struct�ṹ��
	
	//nvic_irq_enable(DMA_Channel0_IRQn, 2);
	//dma_interrupt_enable(DMA_CH0, DMA_INT_FTF);								//����DMA��ȡ�������ж�    DMA_CH0 DMA ͨ��
																            //							DMA_INT_FTF	DMAͨ����������ж�
	dma_channel_enable(DMA_CH0);											//DMAͨ��x����ʹ��
}
	
void adc_init(void)
{
	rcu_periph_clock_enable(RCU_ADC);																		//ADCʱ��
	rcu_periph_clock_enable(RCU_DMA);																		//DMAʱ��
	rcu_adc_clock_config(RCU_ADCCK_APB2_DIV6);//����adcʱ��Ԥ��Ƶѡ��	
	rcu_periph_clock_enable(RCU_GPIOA);		//RCU_GPIOA				GPIOxʱ��(x=A,B,C,F)
	
	//����ADC&DMA����
    //gpio_mode_set(GPIOA,GPIO_MODE_ANALOG,GPIO_PUPD_NONE,GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_8);
	gpio_mode_set(GPIOA,GPIO_MODE_ANALOG,GPIO_PUPD_NONE,GPIO_PIN_1);
	
	dma_adc_init();
	adc_special_function_config(ADC_CONTINUOUS_MODE,ENABLE);		//ʹ�ܻ����ADC���⹦��  ADC_CONTINUOUS_MODE ����ģʽѡ��
	adc_special_function_config(ADC_SCAN_MODE,ENABLE);
	
	adc_external_trigger_source_config(ADC_REGULAR_CHANNEL,ADC_EXTTRIG_REGULAR_NONE);		//����ADC�ⲿ����Դ ADC_REGULAR_CHANNEL			 ����ͨ����
																				            //			       ADC_EXTTRIG_REGULAR_NONE     ��������������飩
	adc_data_alignment_config(ADC_DATAALIGN_RIGHT);							//����ADC���ݶ��뷽ʽ ADC_DATAALIGN_RIGHT LSB ����
	adc_channel_length_config(ADC_REGULAR_CHANNEL, 3U);						//���ù���ͨ�����ע��ͨ����ĳ��� ADC_REGULAR_CHANNEL ����ͨ����

	adc_regular_channel_config(0,ADC_CHANNEL_1,ADC_SAMPLETIME_55POINT5);						//����ADC����ͨ����		0	������ͨ�����У�ȡֵ��ΧΪ0~15																																									//										ADC_CHANNEL_1 					ADCͨ��																																									//										ADC_SAMPLETIME_55POINT5 ����ʱ��55.5����
	adc_regular_channel_config(1,ADC_CHANNEL_2,ADC_SAMPLETIME_55POINT5);						//��·ADC�ɼ�
	adc_regular_channel_config(3,ADC_CHANNEL_8,ADC_SAMPLETIME_55POINT5);						//��·ADC�ɼ�
	
	adc_external_trigger_config(ADC_REGULAR_CHANNEL,ENABLE);												//����ADC�ⲿ���� 		ADC_REGULAR_CHANNEL	����ͨ����

	adc_enable();																						//ʹ��ADC����
	adc_calibration_enable();																//����ADCУ׼
	adc_dma_mode_enable();                                                //ʹ��ADC_DMA
	adc_software_trigger_enable(ADC_REGULAR_CHANNEL);			
}



float Get_12V(void)
{
	return adc_value[0] = initial_value[0] * conversion_value[0];
}

float Get_5V(void)
{
	return adc_value[1] = initial_value[1] * conversion_value[1];
}


uint8_t WaitFor12V_InValid(void)
{
	float vol12v = 0.0;
	while(1)
	{
		vol12v = Get_12V();
		if(vol12v < 1) //����ȴ�12VС��1V������Ӱ�쿪��ģʽ�жϣ�����ģʽ�жϾ���0.1V��
		{
				return 1;
		}
	}
}
