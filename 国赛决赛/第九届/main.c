#include<stc15.h>
#include<onewire.h>
#include<iic.h>
#include<intrins.h>
typedef unsigned int uint;
typedef unsigned char uchar;

uchar code tab[]={0xc0,0xf9,0xa4,0xb0,0x99,0x92,0x82,0xf8,0x80,0x90,0xFF};
uchar menu1[8];
uchar menu2[8];
uchar menu3[8];

uchar menu_index=1;
uchar disp_index=1;
bit key_flag=0;
bit press=0,press_en=0;
bit l8_flag=0,l8_blink=0;
uint press_cnt;

uint freq_cnt=0,freq=0;
bit freq_flag=0;
void freq_handle()
{
	if(freq_flag)
	{
		freq_flag=0;
		freq=freq_cnt;
		freq_cnt=0;
		ET0=1;
		ET1=1;TR1=1;
	}
}


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

void Timer0Init(void)		
{
	AUXR &= 0x7F;		//定时器时钟12T模式
	TMOD = 0x04;		
	TL0 = 0xFF;		//设置定时初值
	TH0 = 0xFF;		//设置定时初值
	TF0 = 0;		//清除TF0标志
	TR0 = 1;		//定时器0开始计时
	ET0=1;
}

void Timer1Init(void)		//50毫秒@12.000MHz
{
	AUXR &= 0xBF;		//定时器时钟12T模式
	TMOD &= 0x0F;		//设置定时器模式
	TL1 = 0xB0;		//设置定时初值
	TH1 = 0x3C;		//设置定时初值
	TF1 = 0;		//清除TF1标志
	TR1 = 1;		//定时器1开始计时
	ET1=1;
}

void Timer2Init(void)		//1毫秒@11.0592MHz
{
	AUXR |= 0x04;		//定时器时钟1T模式
	T2L = 0xCD;		//设置定时初值
	T2H = 0xD4;		//设置定时初值
	AUXR |= 0x10;		//定时器2开始计时
	IE2|=(1<<2);
	EA=1;
}

void main()
{
	uchar volt,tmp_volt,volt_th=10;
	uint temp,tmp_temp,tmp_freq;
	AllInit();
	
	tmp_volt=read_rom(0);
	tmp_freq=read_rom(1);
	tmp_freq<<=8;
	tmp_freq|=read_rom(2);
	tmp_temp=read_rom(3);
	tmp_temp<<=8;
	tmp_temp|=read_rom(4);
	volt_th=read_rom(5);
	
	rd_temperature();
	Delay750ms();
	Timer0Init();
	Timer1Init();
	Timer2Init();
	while(1)
	{
		if(menu_index==1)
		{
			EA=0;
			volt=read_adc(0x03)/5.1;
			EA=1;
			Delay20ms();
			if(volt>volt_th)l8_flag=1;
			else l8_flag=0;
			temp=rd_temperature()*100;
			freq_handle();
		}
		//按键单元
		if(key_flag)
		{
			key_flag=0;
			BTN();
			if(Trg & 0x08)//s4
			{
				if(menu_index==1 || menu_index==2)
				{
					disp_index++;
					if(disp_index==4)disp_index=1;
				}
			}
			if(Trg & 0x04)//s5
			{
				tmp_volt=volt;
				tmp_freq=freq;
				tmp_temp=temp;
				write_rom(0,tmp_volt);Delay20ms();
				write_rom(1,tmp_freq>>8);Delay20ms();
				write_rom(2,tmp_freq);Delay20ms();
				write_rom(3,tmp_temp>>8);Delay20ms();
				write_rom(4,tmp_temp);Delay20ms();
				write_rom(5,volt_th);Delay20ms();
			}
			if(Trg & 0x02)//s6
			{
				if(menu_index==1)menu_index=2;
				else if(menu_index==3)
				{
					volt_th++;
					press=1;
					if(volt_th==51)volt_th=1;
				}else menu_index=1;
			}
			if(Trg & 0x01)//s7
			{
				if(menu_index!=3)menu_index=3;
				else {menu_index=1;disp_index=1;}
			}
		}
		if(press_en)
		{
			if(Cont & 0x02)
			{
				volt_th++;
				if(volt_th==51)volt_th=1;
			}else 
			{
				press_cnt=0;
				press_en=0;
			}
		}
		//数码管单元
		if(menu_index==1)
		{
			if(disp_index==1)
			{
				menu1[0]=0xc1;menu1[1]=0xff;
				menu1[2]=0xff;menu1[3]=0xff;
				menu1[4]=0xff;menu1[5]=0xff;
				menu1[6]=tab[volt/10]&0x7f;
				menu1[7]=tab[volt%10];
			}else if(disp_index==2)
			{
				menu1[0]=0x8e;menu1[1]=0xff;
				menu1[2]=0xff;
				if(freq>=10000)
				{
					menu1[3]=tab[freq/10000];
					menu1[4]=tab[freq%10000/1000];
					menu1[5]=tab[freq%1000/100];
					menu1[6]=tab[freq%100/10];
					menu1[7]=tab[freq%10];
				}else if(freq>=1000)
				{
					menu1[3]=0xff;
					menu1[4]=tab[freq/1000];
					menu1[5]=tab[freq%1000/100];
					menu1[6]=tab[freq%100/10];
					menu1[7]=tab[freq%10];
				}else if(freq>=100)
				{
					menu1[3]=0xff;menu1[4]=0xff;
					menu1[5]=tab[freq/100];
					menu1[6]=tab[freq%100/10];
					menu1[7]=tab[freq%10];
				}else if(freq>=10)
				{
					menu1[3]=0xff;menu1[4]=0xff;menu1[5]=0xff;
					menu1[6]=tab[freq/10];menu1[7]=tab[freq%10];
				}
			}else if(disp_index==3)
			{
				menu1[0]=0xc6;menu1[1]=0xff;
				menu1[2]=0xff;menu1[3]=0xff;
				menu1[4]=tab[temp/1000];
				menu1[5]=tab[temp%1000/100]&0x7f;
				menu1[6]=tab[temp%100/10];
				menu1[7]=tab[temp%10];
			}
		}else if(menu_index==2)
		{
			if(disp_index==1)
			{
				menu2[0]=0x89;menu2[1]=0xc6;
				menu2[2]=0xff;menu2[3]=0xff;
				menu2[4]=tab[tmp_temp/1000];
				menu2[5]=tab[tmp_temp%1000/100]&0x7f;
				menu2[6]=tab[tmp_temp%100/10];
				menu2[7]=tab[tmp_temp%10];
			}else if(disp_index==2)
			{
				menu2[0]=0x89;menu2[1]=0xc1;
				menu2[2]=0xff;menu2[3]=0xff;
				menu2[4]=0xff;menu2[5]=0xff;
				menu2[6]=tab[tmp_volt/10]&0x7f;
				menu2[7]=tab[tmp_volt%10];
			}else if(disp_index==3)
			{
				menu2[0]=0x89;menu2[1]=0x8e;
				menu2[2]=0xff;
				if(tmp_freq>=10000)
				{
					menu2[3]=tab[tmp_freq/10000];
					menu2[4]=tab[tmp_freq%10000/1000];
					menu2[5]=tab[tmp_freq%1000/100];
					menu2[6]=tab[tmp_freq%100/10];
					menu2[7]=tab[tmp_freq%10];
				}else if(tmp_freq>=1000)
				{
					menu2[3]=0xff;
					menu2[4]=tab[tmp_freq/1000];
					menu2[5]=tab[tmp_freq%1000/100];
					menu2[6]=tab[tmp_freq%100/10];
					menu2[7]=tab[tmp_freq%10];
				}else if(tmp_freq>=100)
				{
					menu2[3]=0xff;menu2[4]=0xff;
					menu2[5]=tab[tmp_freq/100];
					menu2[6]=tab[tmp_freq%100/10];
					menu2[7]=tab[tmp_freq%10];
				}else if(tmp_freq>=10)
				{
					menu2[3]=0xff;menu2[4]=0xff;menu2[5]=0xff;
					menu2[6]=tab[tmp_freq/10];menu2[7]=tab[tmp_freq%10];
				}
			}
		}else if(menu_index==3)
		{
			menu3[0]=0x8c;menu3[1]=0xff;
			menu3[2]=0xff;menu3[3]=0xff;
			menu3[4]=0xff;menu3[5]=0xff;
			menu3[6]=tab[volt_th/10]&0x7f;
			menu3[7]=tab[volt_th%10];
		}
		//LED单元
		if(menu_index==1)
		{
			if(disp_index==1)
			{
				if(l8_blink)
				{
					P0=~(0x04|0x80);
					P2=0x80;P0=~(0x04|0x80);P2=0;
				}else 
				{
					P0=~0x04;
					P2=0x80;P0=~0x04;P2=0;
				}
			}else if(disp_index==2)
			{
				if(l8_blink)
				{
					P0=~(0x02|0x80);
					P2=0x80;P0=~(0x02|0x80);P2=0;
				}else 
				{
					P0=~0x02;
					P2=0x80;P0=~0x02;P2=0;
				}
			}else if(disp_index==3)
			{
				if(l8_blink)
				{
					P0=~(0x01|0x80);
					P2=0x80;P0=~(0x01|0x80);P2=0;
				}else 
				{
					P0=~0x01;
					P2=0x80;P0=~0x01;P2=0;
				}
			}
		}else 
		{
			P0=0xff;
			P2=0x80;P0=0xff;P2=0;
		}
	}
}

void Timer0()interrupt 1
{
	freq_cnt++;
}
uchar time_cnt;
void Timer1()interrupt 3
{
	time_cnt++;
	if(time_cnt==20)
	{
		time_cnt=0;
		freq_flag=1;
		ET0=0;
		ET1=0;TR1=0;
	}
}

uchar smg_cnt,i;
uchar key_cnt;
uchar l8_cnt;
void Timer2()interrupt 12
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
	if(press)
	{
		press_cnt++;
		if(press_cnt==800)
		{
			press_cnt=0;
			press_en=1;
		}
	}else
	{
		press_cnt=0;
		press_en=0;
	}
	if(l8_flag)
	{
		l8_cnt++;
		if(l8_cnt==200)
		{
			l8_cnt=0;
			l8_blink=~l8_blink;
		}
	}else
	{
		l8_cnt=0;
		l8_blink=0;
	}
}