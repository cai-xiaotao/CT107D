#include<stc15.h>
#include<iic.h>
#include<onewire.h>
#include<ds1302.h>
#include<intrins.h>
typedef unsigned int uint;
typedef unsigned char uchar;

uchar code tab[]={0xc0,0xf9,0xa4,0xb0,0x99,0x92,0x82,0xf8,0x80,0x90,0xFF};
uchar menu1[8];
uchar menu2[8];
uchar menu3[8];

uchar buf[6];
uchar send_buf[24];
bit mode=0;//自动传输
bit key_flag;
uchar menu_index=1;
uchar light_InitVal;
bit close=0;
bit stay_flag=0;

uchar rom_dat[6];
uchar addr=0;

uchar Trg=0,Cont=0;
void BTN()
{
	uchar dat=P3^0xFF;
	Trg=dat&(dat^Cont);
	Cont=dat;
}


bit busy=0;
void SendData(uchar dat)
{
	while(busy);
	busy=1;
	SBUF=dat;
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
void UartInit(void)		//1200bps@11.0592MHz
{
	SCON = 0x50;		//8位数据,可变波特率
	AUXR |= 0x40;		//定时器1时钟为Fosc,即1T
	AUXR &= 0xFE;		//串口1选择定时器1为波特率发生器
	TMOD &= 0x0F;		//设定定时器1为16位自动重装方式
	TL1 = 0x00;		//设定定时初值
	TH1 = 0xF7;		//设定定时初值
	ET1 = 0;		//禁止定时器1中断
	TR1 = 1;		//启动定时器1
	ES=1;
}



bit send_en=0;
void main()
{
	uchar last_sec,k,m,index;
	bit light_start=0;
	uchar temp,shidu,light;
	uchar hour,min,sec;
	uchar stay_hour,stay_min,stay_sec;
	uint stay_time;
	AllInit();
	rd_temperature();
	Delay750ms();
	set_time(23,59,55);
	light_InitVal=read_adc(0x01)-20;
	Delay20ms();
	Timer0Init();
	UartInit();
	while(1)
	{
		//获取温、湿、亮度
		temp=rd_temperature();
		
		EA=0;
		shidu=read_adc(0x03)*0.389 ;
		EA=1;
		Delay20ms();
		EA=0;
		shidu=read_adc(0x03)*0.389 ;
		EA=1;
		Delay20ms();
		
		EA=0;
		sec=Read_Ds1302_Byte(0x81);
		min=Read_Ds1302_Byte(0x83);
		hour=Read_Ds1302_Byte(0x85);
		EA=1;
		Delay20ms();
		
		EA=0;
		light=read_adc(0x01);
		EA=1;
		Delay20ms();
		EA=0;
		light=read_adc(0x01);
		EA=1;
		Delay20ms();
		
		if(light < light_InitVal)
		{
			close=1;
			light_start=1;
			if(stay_flag==0)
			{
				stay_sec=sec;
				stay_min=min;
				stay_hour=hour;
				stay_flag=1;
			}
		}
		else 
		{
			close=0;
		}
		if(close==0 && stay_flag==1)
		{
			stay_flag=0;
			stay_time=( (hour-stay_hour)*3600+(min-stay_min)*60+(sec-stay_sec) );
			rom_dat[0]=temp;rom_dat[1]=shidu;rom_dat[2]=hour;
			rom_dat[3]=min;rom_dat[4]=sec;rom_dat[5]=stay_time;
			for(k=0;k<6;k++)
			{
				write_rom(k+addr,rom_dat[k]);
				Delay20ms();
			}
			addr+=6;
			if(addr==30)addr=0;
		}
		if(light_start==0)stay_time=0;
		//按键单元
		if(key_flag)
		{
			key_flag=0;
			BTN();
			if(Trg & 0x08)//s4
			{
				mode=~mode;
			}
			if(Trg & 0x04)//s5
			{
				menu_index++;
				if(menu_index==4)menu_index=1;
			}
		}
		//发送数据
		if(send_en)
		{
			if(mode==0)
			{
				if(last_sec!=sec)
				{
					last_sec=sec;
					send_buf[0]='{';send_buf[1]=temp/10+'0';send_buf[2]=temp%10+'0';send_buf[3]='-';
					send_buf[4]=shidu/10+'0';send_buf[5]=shidu%10+'0';send_buf[6]='%';send_buf[7]='}';
					send_buf[8]='{';send_buf[9]=hour/10+'0';send_buf[10]=hour%10+'0';send_buf[11]='-';
					send_buf[12]=min/10+'0';send_buf[13]=min%10+'0';send_buf[14]='-';
					send_buf[15]=sec/10+'0';send_buf[16]=sec%10+'0';send_buf[17]='}';
					send_buf[18]='{';send_buf[20]='}';
					if(close)send_buf[19]='1';
					else send_buf[19]='0';
					send_buf[21]='\r';send_buf[22]='\n';
					for(k=0;k<23;k++)SendData(send_buf[k]);
				}
			}else if(mode==1)
			{
				send_en=0;
				for(m=0;m<5;m++)
				{
					for(k=0;k<6;k++)
					{
						rom_dat[k]=read_rom(k+m*6);
					}
					send_buf[0]='{';send_buf[1]=rom_dat[0]/10+'0';send_buf[2]=rom_dat[0]%10+'0';send_buf[3]='-';
					send_buf[4]=rom_dat[1]/10+'0';send_buf[5]=rom_dat[1]%10+'0';send_buf[6]='%';send_buf[7]='}';
					send_buf[8]='{';send_buf[9]=rom_dat[2]/10+'0';send_buf[10]=rom_dat[2]%10+'0';send_buf[11]='-';
					send_buf[12]=rom_dat[3]/10+'0';send_buf[13]=rom_dat[3]%10+'0';send_buf[14]='-';
					send_buf[15]=rom_dat[4]/10+'0';send_buf[16]=rom_dat[4]%10+'0';send_buf[17]='}';
					send_buf[18]='{';
					if(rom_dat[5]<10)
					{
						send_buf[19]=rom_dat[5]+'0';send_buf[20]='}';
						send_buf[21]='\r';send_buf[22]='\n';
						for(index=0;index<23;index++)SendData(send_buf[index]);
					}else 
					{
						send_buf[19]=rom_dat[5]/10+'0';
						send_buf[20]=rom_dat[5]%10+'0';
						send_buf[21]='}';
						send_buf[22]='\r';send_buf[23]='\n';
						for(index=0;index<24;index++)SendData(send_buf[index]);
					}
				}
			}
		}
		//数码管单元
		if(menu_index==1)
		{
			menu1[0]=tab[temp/10];
			menu1[1]=tab[temp%10];
			menu1[2]=0xc6;
			menu1[3]=0xff;menu1[4]=0xff;
			menu1[5]=tab[shidu/10];
			menu1[6]=tab[shidu%10];
			menu1[7]=0x89;
		}else if(menu_index==2)
		{
			menu2[0]=tab[hour/10];
			menu2[1]=tab[hour%10];
			menu2[3]=tab[min/10];
			menu2[4]=tab[min%10];
			menu2[6]=tab[sec/10];
			menu2[7]=tab[sec%10];
			if(sec%2==0){menu2[2]=0xff;menu2[5]=0xff;}
			else {menu2[2]=0xbf;menu2[5]=0xbf;}
		}else if(menu_index==3)
		{
			menu3[0]=0xff;menu3[1]=0xff;
			menu3[2]=0xff;menu3[3]=0xbf;
			menu3[4]=tab[stay_time/1000];
			menu3[5]=tab[stay_time%1000/100];
			menu3[6]=tab[stay_time%100/10];
			menu3[7]=tab[stay_time%10];
		}
		//LED
		if(mode==0 && close==0)
		{
			P0=~0x01;
			P2=0x80;P0=~0x01;P2=0;
		}else if(mode==0 && close==1)
		{
			P0=~0x05;
			P2=0x80;P0=~0x05;P2=0;
		}else if(mode==1 && close==0)
		{
			P0=~0x02;
			P2=0x80;P0=~0x02;P2=0;
		}else if(mode==1 && close==1)
		{
			P0=~0x06;
			P2=0x80;P0=~0x06;P2=0;
		}
	}
}

uchar key_cnt;
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
		P2=0xE0;P0=0xff;P2=0;
		P2=0xC0;P0=0x01<<i;P2=0;
		if(menu_index==1){P2=0xE0;P0=menu1[i];P2=0;}
		if(menu_index==2){P2=0xE0;P0=menu2[i];P2=0;}
		if(menu_index==3){P2=0xE0;P0=menu3[i];P2=0;}
		i++;
		if(i==8)i=0;
	}
}
uchar j;
void Uart()interrupt 4
{
	if(RI)
	{
		RI=0;
		buf[j]=SBUF;
		j++;
		if(j==6)
		{
			j=0;
			if(buf[0]=='A' && buf[1]=='A' && buf[2]=='A' && buf[3]=='S' && buf[4]=='S' && buf[5]=='S')
			{
				send_en=1;
			}else send_en=0;
		}
	}
	if(TI)
	{
		TI=0;
		busy=0;
	}
}