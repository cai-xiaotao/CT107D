#include<stc15.h>
#include<iic.h>
#include<intrins.h>
typedef unsigned int uint;
typedef unsigned char uchar;

uchar code tab[]={0xc0,0xf9,0xa4,0xb0,0x99,0x92,0x82,0xf8,0x80,0x90,0xFF};
uchar smg[8];

uchar key_cnt;
bit key_flag;
bit relay;
uint water,fee;

uchar Trg=0,Cont=0;
void BTN()
{
	uchar dat=P3^0xFF;
	Trg=dat&(dat^Cont);
	Cont=dat;
}

void Delay20ms()		//@11.0592MHz
{
	unsigned char i, j, k;

	_nop_();
	_nop_();
	i = 1;
	j = 216;
	k = 35;
	do
	{
		do
		{
			while (--k);
		} while (--j);
	} while (--i);
}

void AllInit()
{
	P2=0x80;P0=0xFF;
	P2=0xA0;P0=0x00;
	P2=0xC0;P0=0xFF;
	P2=0xE0;P0=0xFF;
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
	uchar light;
	AllInit();
	Timer0Init();
	while(1)
	{
		EA=0;
		light=read_adc(0x03);
		EA=1;
		Delay20ms();
		if(light<64){P2=0x80;P0=0xFE;P2=0;}
		else {P2=0x80;P0=0xFF;P2=0;}
		//按键单元
		if(key_flag==1)
		{
			key_flag=0;
			BTN();
			if(Trg & 0x01)//s7
			{
				if(!relay){relay=1;fee=0;};
			}
			if(Trg & 0x02)//s6
			{
				if(relay)
				{
					relay=0;
					water=0;
				}
			}
		}
		//数码管单元
		smg[0]=0xFF;smg[1]=tab[0]&0x7F;
		smg[2]=tab[5];smg[3]=tab[0];
		if(relay)
		{
			smg[4]=tab[water/1000];
			smg[5]=tab[water%1000/100]&0x7f;
			smg[6]=tab[water%100/10];
			smg[7]=tab[water%10];
		}else 
		{
			smg[4]=tab[fee/1000];
			smg[5]=tab[fee%1000/100]&0x7f;
			smg[6]=tab[fee%100/10];
			smg[7]=tab[fee%10];
		}
		//继电器单元
		if(relay)
		{
			P2=0xA0;P0=0x10;P2=0;
		}else
		{
			P2=0xA0;P0=0x00;P2=0;
		}
	}
}
uchar time_cnt,smg_cnt,i;
void Timer0()interrupt 1
{
	key_cnt++;smg_cnt++;
	if(key_cnt==10)
	{
		key_cnt=0;
		key_flag=1;
	}
	if(smg_cnt==2)
	{
		smg_cnt=0;
		P2=0xc0;P0=0x01<<i;P2=0;
		P2=0xe0;P0=smg[i];P2=0;
		i++;
		if(i==8)i=0;
	}
	if(relay)
	{
		time_cnt++;
		if(time_cnt==100)
		{
			time_cnt=0;
			water+=1;
			if(water%2==0)fee++;
			if(water==9999)relay=0;
		}
	}else time_cnt=0;
}