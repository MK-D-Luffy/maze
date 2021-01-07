#include <reg52.h>

sfr P4 = 0xe8;
sbit Beep = P3 ^ 7;
sbit A0 = P4 ^ 0;
sbit A1 = P2 ^ 0;
sbit A2 = P2 ^ 7;
sbit irR1_C = P2 ^ 1;
sbit irR2_LU = P2 ^ 2;
sbit irR3_L = P2 ^ 3;
sbit irR4_R = P2 ^ 4;
sbit irR5_RU = P2 ^ 5;
sbit tube1 = P4 ^ 3;
sbit tube2 = P4 ^ 2;

int irL = 0, irLU = 0, irC = 0, irRU = 0, irR = 0;
unsigned char code table[] = {0xc0, 0xf9, 0xa4, 0xb0, 0x99, 0x92, 0x82, 0xf8, 0x80, 0x90};

void delay_ms(unsigned int ms)
{
	unsigned char t;
	while (ms--)
	{
		for (t = 0; t < 113; t++)
			;
			;
	}
}

void display(unsigned int num)
{
	unsigned a = num % 10;
	unsigned b = num / 10;

	tube1 = 0;
	tube2 = 1;
	P0 = table[a];
	delay_ms(5);

	tube1 = 1;
	tube2 = 0;
	P0 = table[b];
	delay_ms(5);
}

void main()
{
	int last = 0;
	while (1)
	{
		A2 = 0, A1 = 0, A0 = 0; //irT1
		delay_ms(5);
		if (irR1_C)
		{
			irC = 0;
			// display(last);
			Beep = 1;
		}
		else
		{
			irC = 1;
			// last = irC;
			// display(irC);
			Beep = 0;
		}

		A2 = 0, A1 = 0, A0 = 1; //irT2
		delay_ms(5);
		if (irR2_LU)
		{
			irLU = 0;
			display(last);
			Beep = 1;
		}
		else
		{
			irLU = 2;
			last = irLU;
			display(irLU);
			Beep = 0;
		}

		A2 = 0, A1 = 1, A0 = 0; //irT3
		delay_ms(5);
		if (irR3_L)
		{
			irL = 0;
			// display(last);
			Beep = 1;
		}
		else
		{
			irL = 3;
			// last = irL;
			// display(irL);
			Beep = 0;
		}

		A2 = 0, A1 = 1, A0 = 1; //irT4
		delay_ms(5);
		if (irR4_R)
		{
			irR = 0;
			// display(last);
			Beep = 1;
		}
		else
		{
			irR = 4;
			// last = irR;
			// display(irR);
			Beep = 0;
		}

		A2 = 1, A1 = 0, A0 = 0; //irT5
		delay_ms(5);
		if (irR5_RU)
		{
			irRU = 0;
			display(last);
			Beep = 1;
		}
		else
		{
			irRU = 5;
			last = irRU;
			display(irRU);
			Beep = 0;
		}

		A2 = 1, A1 = 1, A0 = 1;
		delay_ms(30);
	}
}