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
uchar menu4[8]; 

bit key_flag=0;
uchar menu_index=1;
bit alarm=0;
uchar alarm_set_cnt=0;
uchar set_cnt=0;

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

uchar alarm_hour=0,alarm_min=0,alarm_sec=0;
bit flash=0;
void main()
{
	uchar hour,min,sec,temp;
	uchar temp_hour,temp_min,temp_sec;
	AllInit();
	rd_temperature();
	Delay750ms();
	set_time(23,59,50);
	Delay20ms();
	Timer0Init();
	while(1)
	{
		//获取时间
		EA=0;
		sec=Read_Ds1302_Byte(0x81);
		min=Read_Ds1302_Byte(0x83);
		hour=Read_Ds1302_Byte(0x85);
		EA=1;
		Delay20ms();
		if(menu_index==2)
		{
			temp=rd_temperature();
		}
		//按键单元
		if(key_flag)
		{
			key_flag=0;
			BTN();
			if(Trg & 0x08)//s4
			{
				if(alarm)alarm=0;
				if(menu_index==3)
				{
					if(set_cnt==1 && temp_hour>0)
					{
						temp_hour--;
					}else if(set_cnt==2 && temp_min>0)
					{
						temp_min--;
					}else if(set_cnt==3 && temp_sec>0)
					{
						temp_sec--;
					}
				}else if(menu_index==4)
				{
					if(alarm_set_cnt==1 && alarm_hour>0)
					{
						alarm_hour--;
					}else if(alarm_set_cnt==2 && alarm_min>0)
					{
						alarm_min--;
					}else if(alarm_set_cnt==3 && alarm_sec>0)
					{
						alarm_sec--;
					}
				}
			}
			if(Trg & 0x04)//s5
			{
				if(alarm)alarm=0;
				if(menu_index==3)
				{
					if(set_cnt==1 && temp_hour<23)
					{
						temp_hour++;
					}else if(set_cnt==2 && temp_min<59)
					{
						temp_min++;
					}else if(set_cnt==3 && temp_sec<59)
					{
						temp_sec++;
					}
				}else if(menu_index==4)
				{
					if(alarm_set_cnt==1 && alarm_hour<23)
					{
						alarm_hour++;
					}else if(alarm_set_cnt==2 && alarm_min<59)
					{
						alarm_min++;
					}else if(alarm_set_cnt==3 && alarm_sec<59)
					{
						alarm_sec++;
					}
				}
			}
			if(Trg & 0x02)//s6
			{
				if(alarm)alarm=0;
				if(menu_index==1)
				{
					menu_index=4;
					alarm_set_cnt=1;
				}else if(menu_index==4 && alarm_set_cnt==3)
				{
					menu_index=1;
					alarm_set_cnt=0;
				}else if(menu_index==4 && alarm_set_cnt<3)
				{
					alarm_set_cnt++;
				}
			}
			if(Trg & 0x01)//s7
			{
				if(alarm)alarm=0;
				if(menu_index==1)
				{
					temp_hour=hour;
					temp_min=min;
					temp_sec=sec;
					menu_index=3;
					set_cnt=1;
				}else if(menu_index==3 && set_cnt==3)
				{
					menu_index=1;
					set_cnt=0;
					EA=0;
					set_time(temp_hour,temp_min,temp_sec);
					EA=1;
					Delay20ms();
				}else if(menu_index==3 && set_cnt<3)
				{
					set_cnt++;
				}
			}
		}
		if(menu_index==1 || menu_index==2)
		{
			if(Cont & 0x08)menu_index=2;
			else menu_index=1;
		}
		//数码管单元
		if(menu_index==1)
		{
			menu1[0]=tab[hour/10];
			menu1[1]=tab[hour%10];
			menu1[3]=tab[min/10];
			menu1[4]=tab[min%10];
			menu1[6]=tab[sec/10];
			menu1[7]=tab[sec%10];
			menu1[2]=0xbf;menu1[5]=0xbf;
		}else if(menu_index==2)
		{
			menu2[0]=0xff;menu2[1]=0xff;
			menu2[2]=0xff;menu2[3]=0xff;
			menu2[4]=0xff;menu2[7]=0xc6;
			menu2[5]=tab[temp/10];
			menu2[6]=tab[temp%10];
		}else if(menu_index==3)
		{
			menu3[2]=0xbf;menu3[5]=0xbf;
			menu3[0]=tab[temp_hour/10];
			menu3[1]=tab[temp_hour%10];
			menu3[3]=tab[temp_min/10];
			menu3[4]=tab[temp_min%10];
			menu3[6]=tab[temp_sec/10];
			menu3[7]=tab[temp_sec%10];
			if(sec%2==0)
			{
				if(set_cnt==1)
				{
					menu3[0]=0xff;menu3[1]=0xff;
				}else if(set_cnt==2)
				{
					menu3[3]=0xff;menu3[4]=0xff;
				}else if(set_cnt==3)
				{
					menu3[6]=0xff;menu3[7]=0xff;
				}
			}
		}else if(menu_index==4)
		{
			menu4[2]=0xbf;menu4[5]=0xbf;
			menu4[0]=tab[alarm_hour/10];
			menu4[1]=tab[alarm_hour%10];
			menu4[3]=tab[alarm_min/10];
			menu4[4]=tab[alarm_min%10];
			menu4[6]=tab[alarm_sec/10];
			menu4[7]=tab[alarm_sec%10];
		}
		//闹钟单元
		if(sec==alarm_sec && min==alarm_min && hour==alarm_hour)
		{
			alarm=1;
		}
		if(flash)
		{
			P0=0xfe;
			P2=0x80;P0=0xfe;P2=0;
		}else 
		{
			P0=0xff;
			P2=0x80;P0=0xff;P2=0;
		}
	}
}
uchar key_cnt;
uchar i,smg_cnt;
uint alarm_cnt;
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
		if(menu_index==1){P2=0xE0;P0=menu1[i];P2=0;}
		if(menu_index==2){P2=0xE0;P0=menu2[i];P2=0;}
		if(menu_index==3){P2=0xE0;P0=menu3[i];P2=0;}
		if(menu_index==4){P2=0xE0;P0=menu4[i];P2=0;}
		i++;
		if(i==8)i=0;
	}
	if(alarm==1)
	{
		alarm_cnt++;
		if(alarm_cnt%200==0)
		{
			flash=~flash;
		}
		if(alarm_cnt==5000)
		{
			alarm=0;
			alarm_cnt=0;
		}
	}else if(alarm==0){flash=0;alarm_cnt=0;}
}