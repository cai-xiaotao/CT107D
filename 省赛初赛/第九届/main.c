#include<stc15.h>
#include<iic.h>
#include<intrins.h>
typedef unsigned int uint;
typedef unsigned char uchar;

uchar code tab[]={0xc0,0xf9,0xa4,0xb0,0x99,0x92,0x82,0xf8,0x80,0x90,0xFF};
uchar menu1[8];
uchar menu2[8];

uint time[4];
uchar mode3[4]={0x01|0x80,0x02|0x40,0x04|0x20,0x08|0x10};
uchar mode4[4]={0x08|0x10,0x04|0x20,0x02|0x40,0x01|0x80};

bit key_flag,start_flag=0;
uchar mode=1;
uint time_cnt;
uchar led_index=0;
uchar level;
uchar set_flag=0;
uchar menu_flag=0;

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

uchar light;
uchar LED_temp;
bit smg_flash=0;
uint flash_cnt;
void main()
{
	uchar i,num1,num2;
	AllInit();
	for(i=0;i<4;i++)
	{
		num1=read_rom(i);
		num2=read_rom(i+4);
		if( (num1<<8|num2)>=400 && (num1<<8|num2)<=1200 && (num1<<8|num2)%100==0)
		{
			time[i]=(num1<<8|num2);
		}else time[i]=400;
	}
	Delay20ms();
	Timer0Init();
	while(1)
	{
		EA=0;
		light=read_adc(0x03)/2.55;
		EA=1;
		Delay20ms();
		if(light>75)level=4;
		else if(light<25)level=1;
		else if(light>=25 && light<=50)level=2;
		else level=3;
		//按键单元
		if(key_flag)
		{
			key_flag=0;
			BTN();
			if(Trg & 0x08)//s4
			{
				if(set_flag==1 && mode>1)
				{
					mode--;
				}
				if(set_flag==2 && time[mode-1]>400)
				{
					time[mode-1]-=100;
				}
			}
			if(Trg & 0x04)//s5
			{
				if(set_flag==1 && mode<4)
				{
					mode++;
				}
				if(set_flag==2 && time[mode-1]<1200)
				{
					time[mode-1]+=100;
				}
			}
			if(Trg & 0x02)//s6
			{
				if(menu_flag==0)
				{
					mode=1;
					menu_flag=1;
					set_flag=1;
				}else 
				{
					set_flag++;
					if(set_flag==3)
					{
						set_flag=0;
						menu_flag=0;
						for(i=0;i<4;i++)
						{
							num1=time[i]>>8;
							num2=time[i];
							EA=0;
							write_rom(i,num1);
							write_rom(i+4,num2);
							EA=1;
							Delay20ms();
						}
					}
				}
			}
			if(Trg & 0x01)//s7
			{
				start_flag=~start_flag;
				mode=1;
			}
			if(set_flag==0)
			{
				if(Cont & 0x08)menu_flag=2;
				else menu_flag=0;
			}
		}
		//数码管单元
		if(menu_flag==1)
		{
			menu1[3]=0xff;
			if(smg_flash==0)
			{
				if(set_flag==1)
				{
					menu1[0]=0xff;menu1[1]=0xff;menu1[2]=0xff;
					menu1[4]=tab[time[mode-1]/1000];
					menu1[5]=tab[time[mode-1]%1000/100];
					menu1[6]=tab[time[mode-1]%100/10];
					menu1[7]=tab[time[mode-1]%10];
				}
				else if(set_flag==2)
				{
					menu1[0]=0xbf;
					menu1[1]=tab[mode];menu1[2]=0xbf;
					menu1[4]=0xff;
					menu1[5]=0xff;
					menu1[6]=0xff;
					menu1[7]=0xff;
				}
			}else 
			{
				menu1[0]=0xbf;
				menu1[1]=tab[mode];menu1[2]=0xbf;
				menu1[4]=tab[time[mode-1]/1000];
				menu1[5]=tab[time[mode-1]%1000/100];
				menu1[6]=tab[time[mode-1]%100/10];
				menu1[7]=tab[time[mode-1]%10];
			}
		}else if(menu_flag==2)
		{
			menu2[0]=0xff;menu2[1]=0xff;
			menu2[2]=0xff;menu2[3]=0xff;
			menu2[4]=0xff;menu2[5]=0xff;
			menu2[6]=0xbf;menu2[7]=tab[level];
		}
		//LED单元
		if(mode==1)
		{
			LED_temp=~(0x01<<led_index);
		}else if(mode==2)
		{
			LED_temp=~(0x80>>led_index);
		}else if(mode==3)
		{
			LED_temp=~mode3[led_index];
		}else if(mode==4)
		{
			LED_temp=~mode4[led_index];
		}
	}
}
uchar pwm_cnt;
uchar key_cnt;
uchar smg_cnt,i;
void Timer0()interrupt 1
{
	key_cnt++;
	if(key_cnt==100)
	{
		key_cnt=0;
		key_flag=1;
	}
	if(menu_flag!=0)
	{
		smg_cnt++;
		if(smg_cnt==20)
		{
			smg_cnt=0;
			P2=0xC0;P0=0x01<<i;P2=0;
			if(menu_flag==1){P2=0xE0;P0=menu1[i];P2=0;}
			if(menu_flag==2){P2=0xE0;P0=menu2[i];P2=0;}
			i++;
			if(i==8)i=0;
		}
	}else 
	{
		P0=0xFF;
		P2=0xC0;P0=0xFF;P2=0;
		P2=0xE0;P0=0xFF;P2=0;
		smg_cnt=0;
	}
	if(start_flag && set_flag==0)
	{
		time_cnt++;
		if(time_cnt==time[mode-1]*10)
		{
			time_cnt=0;
			if(mode==1)
			{
				led_index++;
				if(led_index==8){led_index=0;mode=2;}
			}else if(mode==2)
			{
				led_index++;
				if(led_index==8){led_index=0;mode=3;}
			}else if(mode==3)
			{
				led_index++;
				if(led_index==4){led_index=0;mode=4;}
			}else if(mode==4)
			{
				led_index++;
				if(led_index==4){led_index=0;mode=1;}
			}
		}
		pwm_cnt++;
		if(pwm_cnt==100)pwm_cnt=0;
		if(pwm_cnt < light )
		{
			P0=LED_temp;
			P2=0x80;P0=LED_temp;P2=0;
		}
		else 
		{
			P0=0xFF;
			P2=0x80;P0=0xFF;P2=0;
		}
	}else 
	{
		P0=0xFF;
		P2=0x80;P0=0xFF;P2=0;
		time_cnt=0;
		pwm_cnt=0;
	}
	if(set_flag!=0)
	{
		flash_cnt++;
		if(flash_cnt==8000)
		{
			flash_cnt=0;
			smg_flash=~smg_flash;
		}
	}else 
	{	
		flash_cnt=0;
	}
}