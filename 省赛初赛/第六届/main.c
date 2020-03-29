#include<stc15.h>
#include<onewire.h>
#include<ds1302.h>
#include<intrins.h>
typedef unsigned int uint;
typedef unsigned char uchar;

uchar code tab[]={0xc0,0xf9,0xa4,0xb0,0x99,0x92,0x82,0xf8,0x80,0x90,0xFF};
uchar menu1[8];
uchar menu2[8]; 
uchar menu3[8]; 
uchar temp[10];
uchar code time[4]={1,5,30,60};
uchar time_index=0;
uchar smg_cnt,i;
uchar key_cnt;
bit key_flag,start_flag=0,finish;
bit temp_flag=0;
uchar menu_flag=1;
uchar temp_index;
uchar Trg=0,Cont=0;
void BTN()
{
	uchar dat=P3^0xFF;
	Trg=dat&(dat^Cont);
	Cont=dat;
}
void Delay750ms()		//@11.0592MHz
{
	unsigned char i, j, k;

	_nop_();
	_nop_();
	i = 32;
	j = 133;
	k = 87;
	do
	{
		do
		{
			while (--k);
		} while (--j);
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
	uchar hour,min,sec;
	AllInit();
	rd_temperature();
	Delay750ms();
	set_time(23,59,50);
	Delay20ms();
	Timer0Init();
	while(1)
	{
		//按键单元
		if(key_flag==1)
		{
			key_flag=0;
			BTN();
			if(Trg & 0x08)//s4
			{
				if(menu_flag==1)
				{
					time_index++;
					if(time_index==4)time_index=0;
				}
			}
			if(Trg & 0x04)//s5
			{
				if(menu_flag==1)
				{
					menu_flag=2;
					start_flag=1;
				}
			}
			if(Trg & 0x02)//s6
			{
				if(menu_flag==3 && finish==1)
				{
					finish=0;
				}else if(menu_flag==3 && finish==0)
				{
					temp_index++;
					if(temp_index==10)temp_index=0;
				}
			}
			if(Trg & 0x01)//s7
			{
				if(menu_flag==3){time_index=0;menu_flag=1;}
			}
		}
		//数码管单元 
		if(menu_flag==1)
		{
			menu1[0]=0xff;menu1[1]=0xff;
			menu1[2]=0xff;menu1[3]=0xff;
			menu1[4]=0xff;menu1[5]=0xbf;
			menu1[6]=tab[time[time_index]/10];
			menu1[7]=tab[time[time_index]%10];
		}
		else if(menu_flag==2)
		{
			EA=0;
			sec=Read_Ds1302_Byte(0x81);
			min=Read_Ds1302_Byte(0x83);
			hour=Read_Ds1302_Byte(0x85);
			EA=1;
			Delay20ms();
			if(sec%2==0){menu2[2]=0xbf;menu2[5]=0xbf;}
			else {menu2[2]=0xff;menu2[5]=0xff;}
			menu2[0]=tab[hour/10];
			menu2[1]=tab[hour%10];
			menu2[3]=tab[min/10];
			menu2[4]=tab[min%10];
			menu2[6]=tab[sec/10];
			menu2[7]=tab[sec%10];
		}
		else if(menu_flag==3)
		{
			menu3[0]=0xbf;menu3[5]=0xbf;
			menu3[3]=0xff;menu3[4]=0xff;
			menu3[1]=tab[temp_index/10];
			menu3[2]=tab[temp_index%10];
			menu3[6]=tab[temp[temp_index]/10];
			menu3[7]=tab[temp[temp_index]%10];
		}
		//采集温度
		if(temp_flag==1)
		{
			temp_flag=0;
			temp[temp_index]=rd_temperature();
			temp_index++;
			if(temp_index==10){finish=1;temp_index=0;start_flag=0;menu_flag=3;}
		}
	}
}
uint temp_cnt;
uint led_cnt;
void Timer0()interrupt 1
{
	smg_cnt++;key_cnt++;
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
		if(menu_flag==3){P2=0xE0;P0=menu3[i];P2=0;}
		i++;
		if(i==8)i=0;
	}
	if(start_flag)
	{
		temp_cnt++;
		if(temp_cnt==time[time_index]*1000)
		{
			temp_cnt=0;
			temp_flag=1;
		}
	}else temp_cnt=0;
	if(finish)
	{
		led_cnt++;
		if(led_cnt<500)
		{P2=0x80;P0=0xFE;P2=0;}
		else {P2=0x80;P0=0xFF;P2=0;}
		if(led_cnt==1000)led_cnt=0;
	}else {P2=0x80;P0=0xFF;P2=0;}
}