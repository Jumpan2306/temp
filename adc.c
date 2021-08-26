#include "adc.h"


#define VREF 3.3
#define FULL_SCALE 4096

#define BMC_12V (VREF / FULL_SCALE * 136 / 36) //12V分压转换的值
#define BMC_5V 	  (VREF / FULL_SCALE )	//5V分压转换的值

#define ADDR 	 ((VREF / FULL_SCALE))	//其他电压转换的值
float adc_value[3];
uint16_t initial_value[3];
float conversion_value[3] = {BMC_12V , BMC_5V , ADDR};


void dma_adc_init(void)
{
	dma_parameter_struct dma_adc;											//DMA结构体
	
	dma_deinit(DMA_CH0);													//复位DMA通道x的所有寄存器
	dma_struct_para_init(&dma_adc);											//将DMA结构体中所有参数初始化为默认值
	dma_adc.memory_addr = (uint32_t)(initial_value);						//存储器基地址
	dma_adc.memory_inc = 	DMA_MEMORY_INCREASE_ENABLE;						//存储器地址生成算法模式
	dma_adc.memory_width = DMA_MEMORY_WIDTH_16BIT;							//存储器数据传输宽度 //传输数据的内存大小为16位
	dma_adc.periph_addr  = (uint32_t)(&ADC_RDATA);							//外设基地址         //规则数据寄存器
	dma_adc.periph_inc = DMA_PERIPH_INCREASE_DISABLE;						//外设地址生成算法模式
	dma_adc.periph_width = DMA_PERIPHERAL_WIDTH_16BIT;						//外设数据传输宽度 //传输数据的内存大小为16位
	dma_adc.direction = DMA_PERIPHERAL_TO_MEMORY;							//DMA通道数据传输方向// 从外围设备读取和写入内存
	dma_adc.number = 3U;													//DMA通道数据传输数量
	dma_adc.priority = DMA_PRIORITY_HIGH;									//DMA通道传输软件优先级
	dma_circulation_enable(DMA_CH0);										//DMA循环模式使能 			DMA_CH0 DMA通道
	dma_init(DMA_CH0, &dma_adc);											//初始化DMA通道x 			DMA_CH0 DMA通道   dma_adc  dma_parameter_struct结构体
	
	//nvic_irq_enable(DMA_Channel0_IRQn, 2);
	//dma_interrupt_enable(DMA_CH0, DMA_INT_FTF);								//开启DMA读取完后产生中断    DMA_CH0 DMA 通道
																            //							DMA_INT_FTF	DMA通道传输完成中断
	dma_channel_enable(DMA_CH0);											//DMA通道x传输使能
}
	
void adc_init(void)
{
	rcu_periph_clock_enable(RCU_ADC);																		//ADC时钟
	rcu_periph_clock_enable(RCU_DMA);																		//DMA时钟
	rcu_adc_clock_config(RCU_ADCCK_APB2_DIV6);//配置adc时钟预分频选择	
	rcu_periph_clock_enable(RCU_GPIOA);		//RCU_GPIOA				GPIOx时钟(x=A,B,C,F)
	
	//配置ADC&DMA采样
    //gpio_mode_set(GPIOA,GPIO_MODE_ANALOG,GPIO_PUPD_NONE,GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_8);
	gpio_mode_set(GPIOA,GPIO_MODE_ANALOG,GPIO_PUPD_NONE,GPIO_PIN_1);
	
	dma_adc_init();
	adc_special_function_config(ADC_CONTINUOUS_MODE,ENABLE);		//使能或禁能ADC特殊功能  ADC_CONTINUOUS_MODE 连续模式选择
	adc_special_function_config(ADC_SCAN_MODE,ENABLE);
	
	adc_external_trigger_source_config(ADC_REGULAR_CHANNEL,ADC_EXTTRIG_REGULAR_NONE);		//配置ADC外部触发源 ADC_REGULAR_CHANNEL			 规则通道组
																				            //			       ADC_EXTTRIG_REGULAR_NONE     软件触发（规则组）
	adc_data_alignment_config(ADC_DATAALIGN_RIGHT);							//配置ADC数据对齐方式 ADC_DATAALIGN_RIGHT LSB 对齐
	adc_channel_length_config(ADC_REGULAR_CHANNEL, 3U);						//配置规则通道组或注入通道组的长度 ADC_REGULAR_CHANNEL 规则通道组

	adc_regular_channel_config(0,ADC_CHANNEL_1,ADC_SAMPLETIME_55POINT5);						//配置ADC规则通道组		0	规则组通道序列，取值范围为0~15																																									//										ADC_CHANNEL_1 					ADC通道																																									//										ADC_SAMPLETIME_55POINT5 采样时间55.5周期
	adc_regular_channel_config(1,ADC_CHANNEL_2,ADC_SAMPLETIME_55POINT5);						//多路ADC采集
	adc_regular_channel_config(3,ADC_CHANNEL_8,ADC_SAMPLETIME_55POINT5);						//多路ADC采集
	
	adc_external_trigger_config(ADC_REGULAR_CHANNEL,ENABLE);												//配置ADC外部触发 		ADC_REGULAR_CHANNEL	规则通道组

	adc_enable();																						//使能ADC外设
	adc_calibration_enable();																//开启ADC校准
	adc_dma_mode_enable();                                                //使能ADC_DMA
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
		if(vol12v < 1) //必须等待12V小于1V，否则影响开机模式判断（开机模式判断就是0.1V）
		{
				return 1;
		}
	}
}
