#include<stc15.h>
#include<intrins.h>

typedef unsigned int uint;
typedef unsigned char uchar;

void AllInit()
{
	P2=0x80;P0=0xFF;
	P2=0xA0;P0=0x00;
	P2=0xC0;P0=0xFF;
	P2=0xE0;P0=0xFF;
}
void Delay1000ms()		//@11.0592MHz
{
	uchar i, j, k;

	_nop_();
	_nop_();
	i = 43;
	j = 6;
	k = 203;
	do
	{
		do
		{
			while (--k);
		} while (--j);
	} while (--i);
}
 
void main()
{
	uchar i;
	AllInit();
	while(1)
	{
		P2=0x80;P0=0x7f;
		Delay1000ms();
		for(i=0;i<7;i++)
		{
			P0=_cror_(P0,1);
			Delay1000ms();
		}
		for(i=0;i<7;i++)
		{
			P0=_crol_(P0,1);
			Delay1000ms();
		}
	}
}