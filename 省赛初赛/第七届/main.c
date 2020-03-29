#include<stc15.h>
#include<onewire.h>
#include<intrins.h>
typedef unsigned int uint;
typedef unsigned char uchar;

uchar code tab[]={0xc0,0xf9,0xa4,0xb0,0x99,0x92,0x82,0xf8,0x80,0x90,0xFF};
uchar menu0[8];
uchar menu1[8]; 

bit key_flag,start_flag;
bit menu_flag=0;
uchar mode=1;
uint remin_time;
uchar LED_temp,pwm;

uchar Trg=0,Cont=0;
void BTN()
{
	uchar dat=P3^0xff;
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

void AllInit()
{
	P2=0x80;P0=0xFF;P2=0;
	P2=0xA0;P0=0x00;P2=0;
	P2=0xC0;P0=0xFF;P2=0;
	P2=0xE0;P0=0xFF;P2=0;
}
void Timer0Init(void)		//100微秒@11.0592MHz
{
	AUXR |= 0x80;		//定时器时钟1T模式
	TMOD &= 0xF0;		//设置定时器模式
	TL0 = 0xAE;		//设置定时初值
	TH0 = 0xFB;		//设置定时初值
	TF0 = 0;		//清除TF0标志
	TR0 = 1;		//定时器0开始计时
	ET0=1;
	EA=1;
}
void main()
{
	uchar temp;
	AllInit();
	rd_temperature();
	Delay750ms();
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
				mode++;
				if(mode==4)mode=1;
			}
			if(Trg & 0x04)//s5
			{
				if(remin_time<9939)
					remin_time+=60;
			}
			if(Trg & 0x02)//s6
			{
				remin_time=0;
			}
			if(Trg & 0x01)//s7
			{
				menu_flag=~menu_flag;
			}
		}
		//倒计时和LED单元
		if(remin_time>0)
			start_flag=1;
		else start_flag=0;
		if(mode==1){pwm=2;LED_temp=0xfe;}
		else if(mode==2){pwm=3;LED_temp=0xfd;}
		else if(mode==3){pwm=7;LED_temp=0xfb;}
		//数码管单元 
		if(menu_flag==0)
		{
			menu0[0]=0xbf;menu0[2]=0xbf;
			menu0[1]=tab[mode];menu0[3]=0xff;
			menu0[4]=tab[remin_time/1000];
			menu0[5]=tab[remin_time%1000/100];
			menu0[6]=tab[remin_time%100/10];
			menu0[7]=tab[remin_time%10];
		}
		else if(menu_flag==1)
		{
			temp=rd_temperature();
			menu1[0]=0xbf;menu1[2]=0xbf;
			menu1[3]=0xff;menu1[4]=0xff;
			menu1[1]=tab[4];
			menu1[5]=tab[temp/10];
			menu1[6]=tab[temp%10];
			menu1[7]=0xc6;
		}
	}
}
uchar smg_cnt,i;
uchar key_cnt;
uint time_cnt;
uchar pwm_cnt;
void Timer0()interrupt 1
{
	key_cnt++;smg_cnt++;
	if(key_cnt==100)
	{
		key_cnt=0;
		key_flag=1;
	}
	if(smg_cnt==20)
	{
		smg_cnt=0;
		P2=0xC0;P0=0x01<<i;P2=0;
		if(menu_flag==0){P2=0xE0;P0=menu0[i];P2=0;}
		if(menu_flag==1){P2=0xE0;P0=menu1[i];P2=0;}
		i++;
		if(i==8)i=0;
	}
	if(start_flag)
	{
		time_cnt++;
		if(time_cnt==10000)
		{
			time_cnt=0;
			remin_time--;
		}
	}else time_cnt=0;
	if(start_flag)
	{
		pwm_cnt++;
		if(pwm_cnt==10)
		{
			pwm_cnt=0;
		}
		if(pwm_cnt < pwm)
		{
			P0=LED_temp;P2=0x80;P0=LED_temp;P2=0;
		}else {P0=0xff;P2=0x80;P0=0xff;P2=0;}
	}else {pwm_cnt=0;P0=0xff;P2=0x80;P0=0xff;P2=0;}
}