#include<stc15f2k60s2.h>
typedef unsigned int uint;
typedef unsigned char uchar;

uchar code tab[] = {0xc0,0xf9,0xa4,0xb0,0x99,0x92,0x82,0xf8,0x80,0x90,0xFF,0xc0,0xf9,0xa4,0xb0,0x99,0x92,0x82};

void AllInit()
{
	P2=0x80;
	P0=0xFF;
	P2=0xA0;
	P0=0x00;
	P2=0xC0;
	P0=0xFF;
	P2=0xE0;
	P0=0xFF;
}

void delay(uint a)
{
	uchar i;
	while(a--)
		for(i=0;i<12;i++);
}

uchar KeyPress()
{
	uchar KeyValue;
	P3=0x0F;
	P42=0;P44=0;
	if(P3!=0x0F)
	{
		delay(1);
		if(P3!=0x0F)
		{
			switch(P3)
			{
				case 0x07:KeyValue=1;break;
				case 0x0B:KeyValue=2;break;
				case 0x0D:KeyValue=3;break;
				case 0x0E:KeyValue=4;break;
			}
	
		}
	}
	P3=0xF0;
	P42=1;P44=1;
	if(P3!=0xF0||P42!=1||P44!=1)
	{
		delay(1);
		if(P3!=0xF0||P42!=1||P44!=1)
		{
			if(P44==0)KeyValue=KeyValue;
			if(P42==0)KeyValue+=4;
			if(P35==0)KeyValue+=8;
			if(P34==0)KeyValue+=12;
		}
	}
	return KeyValue;
}

void Display(uchar key)
{
	P2=0xC0;
	P0=0x01;
	P2=0xE0;
	P0=tab[key-1];
}

void main()
{
	AllInit();
	while(1)
	{
		Display(KeyPress());
	}
}