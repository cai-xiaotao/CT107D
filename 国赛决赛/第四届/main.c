#include<stc15.h>
#include<ds1302.h>
#include<iic.h>
#include<intrins.h>
typedef unsigned int uint;
typedef unsigned char uchar;

sbit TX=P1^0;
sbit RX=P1^1;

uchar code tab[]={0xc0,0xf9,0xa4,0xb0,0x99,0x92,0x82,0xf8,0x80,0x90,0xFF};
uchar menu1[8];
uchar menu2[8];
uchar menu3[8];
uchar menu4[8];

uchar menu_index=0;
uchar init_flag=0;
bit key_flag,sonic_flag=0;
uchar time_set_index=0;
uint dist_th=30;

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

void Delay20us()		//@11.0592MHz
{
	unsigned char i;

	_nop_();
	_nop_();
	_nop_();
	i = 52;
	while (--i);
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
void Timer1Init(void)		
{
	AUXR &= 0xBF;		//定时器时钟12T模式
	TMOD &= 0x0F;		//设置定时器模式
}


bit led_flag=0,flash;
void main()
{
	uchar hour,min,sec;
	uchar temp_hour,temp_min;
	uint distance;
	Timer0Init();
	Timer1Init();
	while(1)
	{
		//初始化单元
		if(init_flag==0)
		{
			P0=0xfe;
			P2=0x80;P0=0xfe;P2=0;
			P0=0x40;
			P2=0xa0;P0=0x40;P2=0;
		}else if(init_flag==1)
		{
			init_flag=2;
			AllInit();
			menu_index=1;
			set_time(11,50,59);
		}
		//读取时间单元
		if(menu_index==1)
		{
			EA=0;
			sec=Read_Ds1302_Byte(0x81);
			min=Read_Ds1302_Byte(0x83);
			hour=Read_Ds1302_Byte(0x85);
			EA=1;
			Delay20ms();
		}
		//按键单元
		if(key_flag)
		{
			key_flag=0;
			BTN();
			if(Trg & 0x01)//s7
			{
				if(menu_index==1)menu_index=2;
				else if(menu_index==2)menu_index=1;
			}
			if(Trg & 0x02)//s6
			{
				if(menu_index==1)
				{
					menu_index=3;
					time_set_index=1;
					temp_hour=hour;
					temp_min=min;
				}
				else if(menu_index==2)
				{
					menu_index=4;
				}else if(menu_index==3)
				{
					time_set_index++;
					if(time_set_index==3)
					{
						menu_index=1;
						time_set_index=0;
						set_time(temp_hour,temp_min,sec);
					}
				}else if(menu_index==4)
				{
					menu_index=2;
					write_rom(0,dist_th>>8);
					Delay20ms();
					write_rom(1,dist_th);
					Delay20ms();
				}
			}
			if(Trg & 0x04)//s5
			{
				if(time_set_index==1 && temp_hour<23)
				{
					temp_hour++;
				}else if(time_set_index==2 && temp_min<59)
				{
					temp_min++;
				}
				if(menu_index==4 && dist_th<999)
				{
					dist_th++;
				} 
			}
			if(Trg & 0x08)//s4
			{
				if(time_set_index==1 && temp_hour>0)
				{
					temp_hour--;
				}else if(time_set_index==2 && temp_min>0)
				{
					temp_min--;
				}
				if(menu_index==4 && dist_th>0)
				{
					dist_th--;
				} 
			}
		}
		//超声波单元
		if(sonic_flag)
		{
			sonic_flag=0;
			TX=1;
			Delay20us();
			TX=0;
			while(!RX);
			TH1=0;TL1=0;
			TF1=0;TR1=1;
			while(RX==1 && TF1==0);
			TR1=0;
			if(TF1==1)
			{
				TF1=0;
				distance=999;
			}else 
			{
				distance=(TH1<<8|TL1)*0.017;
			}
		}
		//报警模块
		if(init_flag==2)
		{
			if(distance < dist_th)
			{
				P0=0x40;
				P2=0xa0;P0=0x40;P2=0;
			}else 
			{
				P0=0x00;
				P2=0xa0;P0=0x00;P2=0;
			}
			if(distance < 1.2*dist_th)
			{
				led_flag=1;
			}else led_flag=0;
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
		//数码管单元
		if(menu_index==1)
		{
			menu1[0]=tab[hour/10];
			menu1[1]=tab[hour%10];
			menu1[2]=0xbf;menu1[5]=0xbf;
			menu1[3]=tab[min/10];
			menu1[4]=tab[min%10];
			menu1[6]=tab[sec/10];
			menu1[7]=tab[sec%10];
		}else if(menu_index==2)
		{
			menu2[0]=0xff;menu2[1]=0xff;
			menu2[2]=0xff;menu2[3]=0xff;
			menu2[4]=0xff;
			menu2[5]=tab[distance/100];
			menu2[6]=tab[distance%100/10];
			menu2[7]=tab[distance%10];
		}else if(menu_index==3)
		{
			EA=0;
			sec=Read_Ds1302_Byte(0x81);
			EA=1;
			if(time_set_index==1)
			{
				if(sec%2==0){menu3[0]=0xff;menu3[1]=0xff;}
				else {menu3[0]=tab[temp_hour/10];menu3[1]=tab[temp_hour%10];}
				menu3[2]=0xbf;menu3[5]=0xbf;
				menu3[3]=tab[temp_min/10];
				menu3[4]=tab[temp_min%10];
				menu3[6]=tab[sec/10];
				menu3[7]=tab[sec%10];
			}else if(time_set_index==2)
			{
				if(sec%2==0){menu3[3]=0xff;menu3[4]=0xff;}
				else {menu3[3]=tab[temp_min/10];menu3[4]=tab[temp_min%10];}
				menu3[2]=0xbf;menu3[5]=0xbf;
				menu3[0]=tab[temp_hour/10];
				menu3[1]=tab[temp_hour%10];
				menu3[6]=tab[sec/10];
				menu3[7]=tab[sec%10];
			}
		}else if(menu_index==4)
		{
			menu4[0]=0xff;menu4[1]=0xff;
			menu4[2]=0xff;menu4[3]=0xff;
			menu4[4]=0xff;
			menu4[5]=tab[dist_th/100];
			menu4[6]=tab[dist_th%100/10];
			menu4[7]=tab[dist_th%10];
		}
	}
}

uint init_cnt;
uchar smg_cnt,i;
uchar key_cnt,sonic_cnt;
uint led_cnt;
void Timer0()interrupt 1
{
	smg_cnt++;key_cnt++;sonic_cnt++;
	if(key_cnt==10)
	{
		key_cnt=0;
		key_flag=1;
	}
	if(sonic_cnt==200)
	{
		sonic_cnt=0;
		sonic_flag=1;
	}
	if(smg_cnt==2)
	{
		smg_cnt=0;
		P2=0xc0;P0=0x01<<i;P2=0;
		if(menu_index==0){P2=0xe0;P0=0x00;P2=0;}
		if(menu_index==1){P2=0xe0;P0=menu1[i];P2=0;}
		if(menu_index==2){P2=0xe0;P0=menu2[i];P2=0;}
		if(menu_index==3){P2=0xe0;P0=menu3[i];P2=0;}
		if(menu_index==4){P2=0xe0;P0=menu4[i];P2=0;}
		i++;
		if(i==8)i=0;
	}
	if(init_flag==0)
	{
		init_cnt++;
		if(init_cnt==1000)
		{
			init_cnt=0;
			init_flag=1;
		}
	}
	if(led_flag)
	{
		led_cnt++;
		if(led_cnt==500)
		{
			led_cnt=0;
			flash=1;
		}
	}else
	{
		led_cnt=0;
		flash=0;
	}
}