#include<stc15.h>
typedef unsigned int uint;
typedef unsigned char uchar;

uchar led[8];
uchar time_cnt;
uchar times=0,mode=0;

void AllInit()
{
	P2=0x80;P0=0xFF;P2=0;
	P2=0xA0;P0=0x00;P2=0;
	P2=0xC0;P0=0xFF;P2=0;
	P2=0xE0;P0=0xFF;P2=0;
}
void Timer0Init(void)		//5毫秒@11.0592MHz
{
	AUXR |= 0x80;		//定时器时钟1T模式
	TMOD &= 0xF0;		//设置定时器模式
	TL0 = 0x00;		//设置定时初值
	TH0 = 0x28;		//设置定时初值
	TF0 = 0;		//清除TF0标志
	TR0 = 1;		//定时器0开始计时
	ET0=1;
	EA=1;
}

void main()
{
	uchar LED_temp,i,j,temp;
	led[0]=0x01;
	for(i=1;i<8;i++)
	{
		temp=led[0];
		for(j=0;j<i;j++)
		{
			temp |= (led[j]<<1);
			led[i]=temp;
		}
	}
	AllInit();
	Timer0Init();
	while(1)
	{
		LED_temp=0xff;
		//LED流转单元
		LED_temp = ~(led[mode]<<times);
		P0=LED_temp;
		P2=0x80;P0=LED_temp;P2=0;
	}
}
void Timer0()interrupt 1
{
	time_cnt++;
	if(time_cnt==60)
	{
		time_cnt=0;
		times++;
		if(times > 7-mode)
		{
			times=0;
			mode++;
			if(mode==8)mode=0;
		}
	}
}