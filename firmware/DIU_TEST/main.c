
#include "main.h"

static char repeat_text[] = "Repeat test [y/n]: ";
volatile unsigned char second;

ISR(PORTA_PORT_vect)
{
	second++;
	EXTERNAL_INTERRUPT_TRIGGER_PORT.INTFLAGS = EXTERNAL_INTERRUPT_TRIGGER_PIN;
}

ISR(TCA0_OVF_vect)
{
	TCA0.SINGLE.INTFLAGS = TCA_SINGLE_OVF_bm;
}

ISR(TCA0_CMP0_vect)
{
	TCA0.SINGLE.INTFLAGS = TCA_SINGLE_CMP0_bm;
}

ISR(TCB0_INT_vect)
{
	PORTB.OUTTGL = PIN4_bm;
	
	TCB0.INTFLAGS = TCB_CAPT_bm;
}

static unsigned char confirm(void)
{
	unsigned char repeat = '\0';
	
	do
	{
		printf("%s", repeat_text);
		scanf("%c", &repeat);
		printf("\n\r");
		
	} while (repeat != 'y' && repeat != 'n');
	
	printf("\n\r");
	
	if(repeat == 'n')
	{
		return 0;
	}
	return 1;
}

static void counter_init(void)
{
	TCB0.CCMP = 0x1387;
	TCB0.INTCTRL = TCB_CAPT_bm;
	TCB0.CTRLB = TCB_CNTMODE_INT_gc;
	TCB0.CTRLA = TCB_CLKSEL_CLKDIV2_gc | TCB_ENABLE_bm;
	sei();
}

static void counter_disable(void)
{
	TCA0.SINGLE.CTRLA &= ~(TCA_SINGLE_CLKSEL_DIV8_gc | TCA_SINGLE_ENABLE_bm);
}

int main(void)
{
	EXTERNAL_INTERRUPT_TRIGGER_PORT.DIRCLR = EXTERNAL_INTERRUPT_TRIGGER_PIN;
	EXTERNAL_INTERRUPT_TRIGGER_PORT.EXTERNAL_INTERRUPT_TRIGGER_PINCTRL = EXTERNAL_INTERRUPT_TRIGGER_PIN_SETUP;
	
	system_init();
	
	sei();
	
	input_init();
	uart_init();
	mcp7940_init(RTC_Enable);
	clock_init();
	clock_setdata(0UL, 0UL, CLOCK_LED_None);
	
	printf("\n\n\n\rStarting System Test:\n\n\r");
	
	do 
	{
		printf("1.) Button:\n\n\r");
		
		for (unsigned char i=0; i < 4; i++)
		{
			printf("    -> Press S%1u: ", (i + 1));
			while (input_status((1<<i)) == OFF);
			printf("Push detected!\n\r");
		}
		printf("\n\r");
		
	} while (confirm());
	
	do
	{
		printf("2.) Running LED Test:\n\n\r");
		
		for (unsigned char i=0; i < 24; i++)
		{
			clock_settime(i, 0, 0, CLOCK_LED_None);
			printf("    -> Hour: %2u\r", i);
			_delay_ms(250);
		}
		printf("\n\r");
		
		for (unsigned char i=0; i < 60; i++)
		{
			clock_settime(0, i, 0, CLOCK_LED_None);
			printf("    -> Minute: %2u\r", i);
			_delay_ms(250);
		}
		printf("\n\r");
		
		for (unsigned char i=0; i < 4; i++)
		{
			clock_settime(0, 0, 0, (1<<i));
			printf("    -> Clock Back-LEDS: %2u\r", (i + 1));
			_delay_ms(2000);
		}
		printf("\n\n\r");
		
	} while (confirm());
	
	do
	{
		printf("2.) Running Buzzer Test:\n\n\r");
		
		PORTB.DIRSET = PIN4_bm;
		PORTB.OUTCLR = PIN4_bm;
		counter_init();
		
		_delay_ms(5000);
		
		counter_disable();
		PORTB.DIRCLR = PIN4_bm;
		PORTB.OUTCLR = PIN4_bm;
		
		printf("    -> Done\r\n\n");
		
	} while (confirm());
	
	unsigned char fault = 0;
	
	do
	{
		printf("3.) Running RTC Test:\n\n\r");
		printf("    -> Resetting\n\r");
		second = 0;
		
		for (unsigned char i=0; i < 10; i++)
		{
			printf("    -> Wait for trigger: %2u\r", (10 - i));
			_delay_ms(1200);
		}
		
		if (second > 9)
		{
			printf("    -> Trigger confirmed (%u)\n\r", second);
			printf("    -> Real Time Clock running\n\r");
		}
		else
		{
			printf("    -> Trigger not found (%u)\n\r", second);
			printf("    -> Real Time Clock error\n\r");
			
			fault = 1;
		}
		printf("\n\r");
		
	} while (confirm());
	
	if(fault)
	{
		printf("\n\n\rSystem test failure, check system status!\n\r");
	}
	else
	{
		printf("\n\n\rSystem test successful, everything is running!\n\r");
	}
	
    while (1) 
    {
    }
}

