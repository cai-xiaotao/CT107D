#include<stc15.h>
#include<intrins.h>
typedef unsigned int uint;
typedef unsigned char uchar;

bit key_flag,direction=1;
uchar i=0;

uchar Trg=0,Cont=0;
void BTN()
{
	uchar dat=P3^0xFF;
	Trg=dat&(dat^Cont);
	Cont=dat;
}

void AllInit()
{
	P2=0x80;P0=0xFF;P2=0;
	P2=0xA0;P0=0x00;P2=0;
	P2=0xC0;P0=0xFF;P2=0;
	P2=0xE0;P0=0xFF;P2=0;
}

void Timer0Init(void)		//1毫秒@11.0592MHz
{
	AUXR |= 0x80;		//定时器时钟1T模式
	TMOD &= 0xF0;		//设置定时器模式
	TL0 = 0xCD;		//设置定时初值
	TH0 = 0xD4;		//设置定时初值
	TF0 = 0;		//清除TF0标志
	TR0 = 1;		//定时器0开始计时
	ET0=1;
	EA=1;
}
void main()
{
	AllInit();
	Timer0Init();
	while(1)
	{
		if(key_flag)
		{
			key_flag=0;
			BTN();
			if(Trg & 0x08)//s4
			{
				direction=~direction;
			}
		}
		//LED流水灯
		P0=~(0x01<<i);
		P2=0x80;P0=~(0x01<<i);P2=0;
	}
}

uchar key_cnt;
uint time_cnt;
void Timer0()interrupt 1
{
	key_cnt++;time_cnt++;
	if(key_cnt==10)
	{
		key_cnt=0;
		key_flag=1;
	}
	if(time_cnt==1000)
	{
		time_cnt=0;
		if(direction)
		{
			i++;
			if(i==7)direction=0;
		}else 
		{
			i--;
			if(i==0)direction=1;
		}
	}
}