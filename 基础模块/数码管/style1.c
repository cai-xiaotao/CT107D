#include<stc15.h>
#include<intrins.h>
typedef unsigned int uint;
typedef unsigned char uchar;

uchar code tab[]={0xC0,0xF9,0xA4,0xB0,0x99,0x92,0x82,0xF8,0x80,0x90};
uchar smg[8];

void Delay2ms()		//@11.0592MHz
{
	unsigned char i, j;

	_nop_();
	_nop_();
	i = 22;
	j = 128;
	do
	{
		while (--j);
	} while (--i);
}

void AllInit()
{
	P2=0x80;P0=0xFF;
	P2=0xA0;P0=0x00;
	P2=0xC0;P0=0xFF;
	P2=0xE0;P0=0xFF;
}

void Display()
{
	uchar i,temp;
	temp=0x01;
	for(i=0;i<8;i++)
	{
		P2=0xC0;
		P0=temp;
		temp<<=1;
		P2=0xE0;
		P0=smg[i];
		Delay2ms();
		P0=0xFF;
	}
}

void main()
{
	AllInit();
	while(1)
	{
		smg[0]=tab[0];smg[1]=tab[1];
		smg[2]=tab[2];smg[3]=tab[3];
		smg[4]=tab[4];smg[5]=tab[5];
		smg[6]=tab[6];smg[7]=tab[7];
		Display();
	}
}