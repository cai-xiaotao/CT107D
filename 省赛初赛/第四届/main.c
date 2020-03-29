#include<stc15.h>
#include<ds1302.h>
#include<iic.h>
#include<intrins.h>
typedef unsigned int uint;
typedef unsigned char uchar;

uchar code tab[]={0xc0,0xf9,0xa4,0xb0,0x99,0x92,0x82,0xf8,0x80,0x90};
uchar menu1[8];
uchar menu2[8];
uchar menu_flag=1;
uchar key_cnt;
bit key_flag=0;
bit work_mode=0;
uchar shidu_th=50;
bit relay,buzz;
bit buzz_flag=1;

uchar Trg=0,Cont=0;
void BTN()
{
	uchar dat=P3^0xFF;
	Trg=dat&(dat ^ Cont);
	Cont=dat;
}	
	
void Delay5ms()		//@11.0592MHz
{
	unsigned char i, j;

	i = 54;
	j = 199;
	do
	{
		while (--j);
	} while (--i);
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
	uchar hour,min,shidu;
	AllInit();
	set_time(8,30);
	Delay5ms();
	shidu_th=read_rom(0x00);
	if(shidu_th>99 || shidu_th<0)
	{shidu_th=50;}
	Timer0Init();
	while(1)
	{
		//获取时间
		EA=0;
		hour=Read_Ds1302_Byte(0x85);
		min=Read_Ds1302_Byte(0x83);
		EA=1;
		Delay20ms();
		//按键单元
		if(key_flag==1)
		{
			key_flag=0;
			BTN();
			if(Trg & 0x08)//s4
			{
				if(work_mode==1)relay=0;
				if(menu_flag==2)
				{
					if(shidu_th>0)shidu_th--;
				}				
			}
			if(Trg & 0x04)//s5
			{
				if(work_mode==1)relay=1;
				if(menu_flag==2)
				{
					if(shidu_th<99)shidu_th++;
				}
			}
			if(Trg & 0x02)//s6
			{
				if(work_mode==1)
					buzz_flag=~buzz_flag;
				else if(work_mode==0 && menu_flag==1)
				{
					menu_flag=2;
				}
				else if(work_mode==0 && menu_flag==2)
				{
					menu_flag=1;
					EA=0;
					write_rom(0x00,shidu_th);
					EA=1;
					Delay20ms();
				}
			}
			if(Trg & 0x01)//s7
			{
				work_mode=~work_mode;
			}
		}
		//获取湿度
		EA=0;
		shidu=read_adc(0x03);
		EA=1;
		shidu=shidu/2.575;
		Delay20ms();
		//数码管单元
		menu1[0]=tab[hour/10];
		menu1[1]=tab[hour%10];
		menu1[2]=0xBF;menu1[5]=0xFF;
		menu1[3]=tab[min/10];
		menu1[4]=tab[min%10];
		menu1[6]=tab[shidu/10];
		menu1[7]=tab[shidu%10];
		menu2[0]=0xBF;menu2[1]=0xBF;
		menu2[2]=0xFF;menu2[3]=0xFF;
		menu2[4]=0xFF;menu2[5]=0xFF;
		menu2[6]=tab[shidu_th/10];
		menu2[7]=tab[shidu_th%10];
		//工作模式
		if(work_mode==0)
		{
			P2=0x80;P0=0xFE;P2=0;
			if(shidu<shidu_th)relay=1;
			else relay=0;
		}else 
		{
			P2=0x80;P0=0xFD;P2=0;
			if(shidu<shidu_th && buzz_flag==1)buzz=1;
			else buzz=0;
		}
		//继电器和蜂鸣器单元
		if(relay==1 && buzz==1)
		{
			P2=0xA0;P0=0x50;P2=0;
		}else if(relay==0 && buzz==1)
		{
			P2=0xA0;P0=0x40;P2=0;
		}else if(relay==1 && buzz==0)
		{
			P2=0xA0;P0=0x10;P2=0;
		}else {P2=0xA0;P0=0x00;P2=0;}
	}
}
uchar smg_cnt,i;
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
		P2=0xC0;P0=0x01<<i;P2=0;
		if(menu_flag==1){P2=0xE0;P0=menu1[i];P2=0;}
		if(menu_flag==2){P2=0xE0;P0=menu2[i];P2=0;}
		i++;
		if(i==8)i=0;
	}
}