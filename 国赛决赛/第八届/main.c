#include<stc15.h>
#include<iic.h>
#include<intrins.h>
typedef unsigned int uint;
typedef unsigned char uchar;

uchar code tab[]={0xc0,0xf9,0xa4,0xb0,0x99,0x92,0x82,0xf8,0x80,0x90,0xFF};
uchar menu1[8];
uchar menu2[8];
uchar menu3[8];
uint dist[4];

sbit TX=P1^0;
sbit RX=P1^1;

uchar dist_th;
uchar addr=0;
uchar page_index=1;
uchar dist_index=0;
bit key_flag=0;
bit start;
uchar menu_index=1;

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


bit finish,l1_blink;
void main()
{
	uchar dist_temp;
	uchar volt;
	uint distance,last_distance;
	AllInit();
	for(addr=0;addr<4;addr++)
	{
		dist[addr]=read_adc(addr);
		if(dist[addr]==255)dist[addr]=999;
	}
	dist_th=read_adc(4);
	if(dist_th<0 || dist_th>30 || dist_th%10!=0)
	{
		dist_th=20;
	}
	dist_temp=dist_th;
	distance=read_adc(5);
	if(distance==255)distance=999;
	last_distance=read_adc(6);
	if(last_distance==255)last_distance=999;
	addr=0;
	Timer0Init();
	Timer1Init();
	while(1)
	{
		//按键单元
		if(key_flag)
		{
			key_flag=0;
			BTN();
			if(Trg & 0x08)//s4
			{
				if(menu_index==1)
				{
					start=1;
				}
			}
			if(Trg & 0x04)//s5
			{
				if(menu_index!=2)menu_index=2;
				else menu_index=1;
			}
			if(Trg & 0x02)//s6
			{
				if(menu_index!=3)menu_index=3;
				else 
				{
					menu_index=1;
					dist_th=dist_temp;
					write_rom(4,dist_th);
					Delay20ms();
				}
			}
			if(Trg & 0x01)//s7
			{
				if(menu_index==2)
				{
					page_index++;
					if(page_index==5)page_index=1;
				}else if(menu_index==3)
				{
					dist_temp+=10;
					if(dist_temp==40)dist_temp=0;
				}
			}
		}
		//DAC
		if(distance<=dist_th)volt=0;
		else 
		{
			volt=(distance-dist_th)*0.02*51;
			if(volt>255)volt=255;
		}
		EA=0;
		write_adc(volt);
		EA=1;
		Delay20ms();
		//超声波单元
		if(start)
		{
			start=0;
			last_distance=distance;
			TX=1;
			Delay20us();
			TX=0;
			while(!RX);
			TH1=0;TL1=0;
			TF1=0;TR1=1;
			while(RX==1 && TF1==0);
			TR1=0;
			if(TF1==1){TF1=0;distance=999;}
			else 
			{
				distance=(TH1<<8|TL1)*0.017;
			}
			dist[dist_index]=distance;
			dist_index++;
			if(dist_index==4)dist_index=0;
			if(distance==999)
			{
				write_rom(addr++,255);Delay20ms();
				write_rom(5,255);Delay20ms();
			}else 
			{
				write_rom(addr++,distance);Delay20ms();
				write_rom(5,distance);Delay20ms();
			}
			if(addr==4)addr=0;
			if(last_distance==999){write_rom(6,255);Delay20ms();}
			else {write_rom(6,last_distance);Delay20ms();}
			finish=1;
		}
		//数码管单元
		menu1[0]=0xc6;menu1[1]=0xff;
		menu1[2]=tab[distance/100];
		menu1[3]=tab[distance%100/10];
		menu1[4]=tab[distance%10];
		menu1[5]=tab[last_distance/100];
		menu1[6]=tab[last_distance%100/10];
		menu1[7]=tab[last_distance%10];
		if(menu_index==2)
		{
			menu2[0]=tab[page_index];
			menu2[1]=0xff;menu2[2]=0xff;
			menu2[3]=0xff;menu2[4]=0xff;
			menu2[5]=tab[dist[page_index-1]/100];
			menu2[6]=tab[dist[page_index-1]%100/10];
			menu2[7]=tab[dist[page_index-1]%10];
		}else if(menu_index==3)
		{
			menu3[0]=0x8e;menu3[1]=0xff;
			menu3[2]=0xff;menu3[3]=0xff;
			menu3[4]=0xff;
			menu3[5]=0xff;
			menu3[6]=tab[dist_temp/10];
			menu3[7]=tab[dist_temp%10];
		}
		//LED
		if(l1_blink)
		{
			P0=0xfe;
			P2=0x80;P0=0xfe;P2=0;
		}else if(menu_index==3)
		{
			P0=0xbf;
			P2=0x80;P0=0xbf;P2=0;
		}else if(menu_index==2)
		{
			P0=0x7f;
			P2=0x80;P0=0x7f;P2=0;
		}else 
		{
			P0=0xff;
			P2=0x80;P0=0xff;P2=0;
		}
	}
}
uint l1_cnt;
uchar times;
uchar smg_cnt,i,key_cnt;
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
		P2=0xE0;P0=0xFF;P2=0;
		P2=0xC0;P0=0x01<<i;P2=0;
		if(menu_index==1){P2=0xE0;P0=menu1[i];P2=0;}
		if(menu_index==2){P2=0xE0;P0=menu2[i];P2=0;}
		if(menu_index==3){P2=0xE0;P0=menu3[i];P2=0;}
		i++;
		if(i==8)i=0;
	}
	if(finish)
	{
		l1_cnt++;
		if(l1_cnt==1000)
		{
			l1_cnt=0;
			l1_blink=~l1_blink;
			times++;
			if(times==6){times=0;finish=0;}
		}
	}else
	{
		l1_blink=0;
		l1_cnt=0;
	}
}