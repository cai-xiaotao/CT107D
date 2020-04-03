#include<stc15.h>
#include<ds1302.h>
#include<iic.h>
#include<intrins.h>
typedef unsigned int uint;
typedef unsigned char uchar;

uchar code tab[]={0xc0,0xf9,0xa4,0xb0,0x99,0x92,0x82,0xf8,0x80,0x90,0xFF};
uchar menu1[8];
uchar menu2[8];
uchar menu3[8];
uchar menu4[8];

uchar volt_max,volt_min;
uint freq_cnt;
uint freq,t;
bit freq_flag=0;
uchar menu_index=0;
uchar time_set_index=0,volt_set_index=0;

#define set(x)	P4=(x>>3|x>>4);P3=x
#define get()	((P4&0x10)<<3)|((P4&0x04)<<4)|(P3&0x3f)
uchar Trg=0,Cont=0;
void KBD()
{
	uchar dat;
	set(0x0f);
	dat=get();
	set(0xf0);
	dat=(dat|get())^0xFF;
	Trg=dat&(dat^Cont);
	Cont=dat;
}


void freq_handle()
{
	if(freq_flag)
	{
		freq_flag=0;
		freq=freq_cnt;
		t=1000000/freq;
		freq_cnt=0;
		TR0=1;
		TR1=1;ET1=1;
	}
}

void AllInit()
{
	P2=0x80;P0=0xFF;
	P2=0xA0;P0=0x00;
	P2=0xC0;P0=0xFF;
	P2=0xE0;P0=0xFF;
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
	TL0 = 0xff;		//设置定时初值
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
void Timer2Init(void)		//1毫秒@12.000MHz
{
	AUXR &= 0xFB;		//定时器时钟12T模式
	T2L = 0x18;		//设置定时初值
	T2H = 0xFC;		//设置定时初值
	AUXR |= 0x10;		//定时器2开始计时
	IE2|=(1<<2);
	EA=1;
}

bit key_flag;
bit smg_blink=0;
bit event_menu=0;
bit f_t_flag;//频率周期标志位
void main()
{
	uchar event,event_hour,event_min,event_sec;
	uchar volt;
	uchar hour,min,sec;
	uchar temp_hour,temp_min,temp_sec;
	AllInit();
	volt_max=read_rom(0);
	volt_min=read_rom(1);
	if(volt_max<0 || volt_max>50 || volt_min<0 || volt_min>50 || volt_min%10!=0 || volt_max%10!=0)
	{
		volt_max=20;
		volt_min=20;
	}
	event=read_rom(2);
	event_hour=read_rom(3);
	event_min=read_rom(4);
	event_sec=read_rom(5);
	set_time(23,59,55);
	Timer0Init();
	Timer1Init();
	Timer2Init();
	while(1)
	{
		//RTC 和 电压
		EA=0;
		sec=Read_Ds1302_Byte(0x81);
		min=Read_Ds1302_Byte(0x83);
		hour=Read_Ds1302_Byte(0x85);
		EA=1;
		Delay20ms();
		EA=0;
		volt=read_adc(0x03)/5.1;
		EA=1;
		Delay20ms();
		//频率
		if(menu_index==3)freq_handle();
		//判断事件
		if(volt>=volt_max || volt <= volt_min)
		{
			if(volt >= volt_max)event=1;
			else if(volt <= volt_min)event=2;
			event_hour=hour;event_min=min;event_sec=sec;
			write_rom(2,event);Delay20ms();
			write_rom(3,event_hour);Delay20ms();
			write_rom(4,event_min);Delay20ms();
			write_rom(5,event_sec);Delay20ms();
		}
		//按键单元
		if(key_flag)
		{
			key_flag=0;
			KBD();
			switch(Trg ^ 0xFF)
			{
				case 0x77://s4
					if(menu_index==1 && time_set_index==0)
					{
						temp_hour=hour;temp_min=min;temp_sec=sec;
						time_set_index=1;
					}else if(menu_index==1 && time_set_index!=0)
					{
						time_set_index++;
						if(time_set_index==4)time_set_index=1;
					}
					if(menu_index==2 && volt_set_index==0)
					{
						volt_set_index=1;
					}else if(menu_index==2 && volt_set_index!=0)
					{
						volt_set_index++;
						if(volt_set_index==3)volt_set_index=1;
					}
					if(menu_index==3)f_t_flag=~f_t_flag;
					break;
				case 0x7b://s5
					if(menu_index!=3)menu_index=3;
					break;
				case 0x7d://s6
					if(menu_index!=2 || volt_set_index!=0)
					{
						menu_index=2;
						if(volt_set_index!=0)
						{
							volt_set_index=0;
							if(volt_max >= volt_min)
							{
								write_rom(0,volt_max);Delay20ms();
								write_rom(1,volt_min);Delay20ms();
							}
						}
					}
					break;
				case 0x7e://s7
					if(menu_index!=1 || time_set_index!=0)
					{
						menu_index=1;
						if(time_set_index!=0)
						{
							time_set_index=0;
							set_time(temp_hour,temp_min,temp_sec);
							Delay20ms();
						}
					}
					break;
				case 0xbb://s9
					if(menu_index!=4)
					{
						menu_index=4;
					}else event_menu=~event_menu;
					break;
				case 0xbd://s10
					if(time_set_index==1 && temp_hour>0)temp_hour--;
					else if(time_set_index==2 && temp_min>0)temp_min--;
					else if(time_set_index==3 && temp_sec>0)temp_sec--;
					if(volt_set_index==1 && volt_max>0)volt_max-=5;
					else if(volt_set_index==2 && volt_min>0)volt_min-=5;
					break;
				case 0xbe://s11
					if(time_set_index==1 && temp_hour<23)temp_hour++;
					else if(time_set_index==2 && temp_min<59)temp_min++;
					else if(time_set_index==3 && temp_sec<59)temp_sec++;
					if(volt_set_index==1 && volt_max<50)volt_max+=5;
					else if(volt_set_index==2 && volt_min<50)volt_min+=5;
					break;
			}
		}
		//数码管单元
		if(menu_index==1)
		{
			if(time_set_index==0)
			{
				menu1[0]=tab[hour/10];menu1[1]=tab[hour%10];
				menu1[2]=0xbf;menu1[5]=0xbf;
				menu1[3]=tab[min/10];menu1[4]=tab[min%10];
				menu1[6]=tab[sec/10];menu1[7]=tab[sec%10];
			}else if(time_set_index==1)
			{
				if(sec%2==0){menu1[0]=0xff;menu1[1]=0xff;}
				else {menu1[0]=tab[temp_hour/10];menu1[1]=tab[temp_hour%10];}
				menu1[2]=0xbf;menu1[5]=0xbf;
				menu1[3]=tab[temp_min/10];menu1[4]=tab[temp_min%10];
				menu1[6]=tab[temp_sec/10];menu1[7]=tab[temp_sec%10];
			}else if(time_set_index==2)
			{
				if(sec%2==0){menu1[3]=0xff;menu1[4]=0xff;}
				else {menu1[3]=tab[temp_min/10];menu1[4]=tab[temp_min%10];}
				menu1[0]=tab[temp_hour/10];menu1[1]=tab[temp_hour%10];
				menu1[2]=0xbf;menu1[5]=0xbf;
				menu1[6]=tab[temp_sec/10];menu1[7]=tab[temp_sec%10];
			}else if(time_set_index==3)
			{
				if(sec%2==0){menu1[6]=0xff;menu1[7]=0xff;}
				else {menu1[6]=tab[temp_sec/10];menu1[7]=tab[temp_sec%10];}
				menu1[0]=tab[temp_hour/10];menu1[1]=tab[temp_hour%10];
				menu1[2]=0xbf;menu1[5]=0xbf;
				menu1[3]=tab[temp_min/10];menu1[4]=tab[temp_min%10];
			}
		}else if(menu_index==2)
		{
			if(volt_set_index==0)
			{
				menu2[0]=0xbf;menu2[1]=tab[1];
				menu2[2]=0xbf;menu2[3]=0xff;
				menu2[4]=tab[volt/10];
				menu2[5]=tab[volt%10];
				menu2[6]=tab[0];menu2[7]=tab[0];
			}else if(volt_set_index==1)
			{
				if(sec%2==0){menu2[0]=0xff;menu2[1]=0xff;menu2[2]=0xff;menu2[3]=0xff;}
				else {menu2[0]=tab[volt_max/10];menu2[1]=tab[volt_max%10];menu2[2]=tab[0];menu2[3]=tab[0];}
				menu2[4]=tab[volt_min/10];
				menu2[5]=tab[volt_min%10];
				menu2[6]=tab[0];menu2[7]=tab[0];
			}else if(volt_set_index==2)
			{
				if(sec%2==0){menu2[4]=0xff;menu2[5]=0xff;menu2[6]=0xff;menu2[7]=0xff;}
				else {menu2[4]=tab[volt_min/10];menu2[5]=tab[volt_min%10];menu2[6]=tab[0];menu2[7]=tab[0];}
				menu2[0]=tab[volt_max/10];
				menu2[1]=tab[volt_max%10];
				menu2[2]=tab[0];menu2[3]=tab[0];
			}
		}else if(menu_index==3)
		{
			menu3[0]=0xbf;menu3[1]=tab[2];menu3[2]=0xbf;
			if(f_t_flag==0)
			{
				menu3[3]=tab[freq/10000];
				menu3[4]=tab[freq%10000/1000];
				menu3[5]=tab[freq%1000/100];
				menu3[6]=tab[freq%100/10];
				menu3[7]=tab[freq%10];
			}else 
			{
				menu3[3]=tab[t/10000];
				menu3[4]=tab[t%10000/1000];
				menu3[5]=tab[t%1000/100];
				menu3[6]=tab[t%100/10];
				menu3[7]=tab[t%10];
			}
		}else if(menu_index==4)
		{
			if(event_menu==0)
			{
				menu4[0]=0xff;menu4[1]=0xff;menu4[2]=0xff;
				menu4[3]=0xff;menu4[4]=0xff;menu4[5]=0xff;
				menu4[6]=tab[0];
				menu4[7]=tab[event];
			}else 
			{
				menu4[0]=tab[event_hour/10];
				menu4[1]=tab[event_hour%10];
				menu4[2]=0xbf;menu4[5]=0xbf;
				menu4[3]=tab[event_min/10];
				menu4[4]=tab[event_min%10];
				menu4[6]=tab[event_sec/10];
				menu4[7]=tab[event_sec%10];
			}
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
		TR0=0;
		TR1=0;ET1=0;
	}
}
uchar key_cnt;
uchar smg_cnt,i;
uint blink_cnt;
void Timer2()interrupt 12
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
		P2=0xE0;P0=0xff;P2=0;
		P2=0xC0;P0=0x01<<i;P2=0;
		if(menu_index==1){P2=0xE0;P0=menu1[i];P2=0;}
		if(menu_index==2){P2=0xE0;P0=menu2[i];P2=0;}
		if(menu_index==3){P2=0xE0;P0=menu3[i];P2=0;}
		if(menu_index==4){P2=0xE0;P0=menu4[i];P2=0;}
		i++;
		if(i==8)i=0;
	}
}