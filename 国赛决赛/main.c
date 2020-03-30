#include<stc15.h>
#include<iic.h>
#include<ds1302.h>
#include<intrins.h>
typedef unsigned int uint;
typedef unsigned char uchar;

sbit TX=P1^0;
sbit RX=P1^1;

uchar code tab[]={0xc0,0xf9,0xa4,0xb0,0x99,0x92,0x82,0xf8,0x80,0x90,0xFF};
uchar menu1[8];
uchar menu2[8];
uchar menu3[8];
uchar num[6]={10,10,10,10,10,10};
uchar psw[]={6,5,4,3,2,1};

uchar menu_index=1;
bit key_flag;
bit relay=0,buzz=0;
uchar error_times=0;
uchar mode;
bit set_flag=0,sonic_flag;

#define set(x) P4=(x>>3|x>>4);P3=x
#define get()  ((P4&0x10)<<3)|((P4&0x04)<<4)|(P3&0x3f)
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


void AllInit()
{
	P2=0x80;P0=0xFF;P2=0;
	P2=0xA0;P0=0x00;P2=0;
	P2=0xC0;P0=0xFF;P2=0;
	P2=0xE0;P0=0xFF;P2=0;
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
void Timer1Init(void)		//1毫秒@12.000MHz
{
	AUXR &= 0xBF;		//定时器时钟12T模式
	TMOD &= 0x0F;		//设置定时器模式
}


void main()
{
	uint distance;
	uchar hour,min,sec;
	uchar temp=10,j=0;
	uchar k;
	AllInit();
	for(k=0;k<6;k++)
	{
		psw[k]=read_rom(k);
	}
	set_time(6,59,0);
	Timer0Init();
	Timer1Init();
	while(1)
	{
		//获取时间
		EA=0;
		sec=Read_Ds1302_Byte(0x81);
		min=Read_Ds1302_Byte(0x83);
		hour=Read_Ds1302_Byte(0x85);
		EA=1;
		Delay20ms();
		if(hour >=7 && hour <22)
		{
			mode=1;
		}else mode=2;
		//超声波单元
		if(mode==1)
		{
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
				if(TF1==1){TF1=0;distance=999;}
				else 
				{
					distance=(TH1<<8|TL1)*0.017;
				}
			}
			if(distance<30)relay=1;
		}
		//按键单元
		if(key_flag)
		{
			key_flag=0;
			KBD();
			switch(Trg ^ 0xFF)
			{
				case 0x7b:
					temp=8;
					break;
				case 0x7d:
					temp=4;
					break;
				case 0x7e:
					temp=0;
					break;
				case 0xbb:
					temp=9;
					break;
				case 0xbd:
					temp=5;
					break;
				case 0xbe:
					temp=1;
					break;
				case 0xd7://确认
					if(menu_index==1)
					{
						for(k=0;k<6;k++)
						{
							if(num[k]!=psw[k])
							break;
						}
						if(k==6){relay=1;error_times=0;}
						else 
						{
							error_times++;
							if(error_times==3)
							{
								error_times=0;
								buzz=1;
							}
						}
						for(k=0;k<6;k++)
						{
							num[k]=10;
						}
						j=0;
					}else if(menu_index==2 && set_flag==0)
					{
						for(k=0;k<6;k++)
						{
							if(num[k]!=psw[k])
							break;
						}
						if(k==6){set_flag=1;error_times=0;}
						else 
						{
							error_times++;
							if(error_times==3)
							{
								error_times=0;
								buzz=1;
							}
						}
						for(k=0;k<6;k++)
						{
							num[k]=10;
						}
						j=0;
					}else if(menu_index==2 && set_flag==1)
					{
						menu_index=1;
						for(k=0;k<6;k++)
						{
							psw[k]=num[k];
							num[k]=10;
						}
						for(k=0;k<6;k++)
						{
							write_rom(k,psw[k]);
							Delay20ms();
						}
						j=0;
					}
					break;
				case 0xdb://设置
					if(menu_index==1)menu_index=2;
					for(k=0;k<6;k++)
					{
						num[k]=10;
					}
					j=0;
					break;
				case 0xdd:
					temp=6;
					break;
				case 0xde:
					temp=2;
					break;
				case 0xe7://退出
					if(menu_index==2)
					{
						for(k=0;k<6;k++)
						{
							num[k]=10;
						}
						j=0;
						menu_index=1;
					}
					break;
				case 0xeb://复位
					for(k=0;k<6;k++)
					{
						psw[k]=6-k;
					}
					for(k=0;k<6;k++)
					{
						write_rom(k,psw[k]);
						Delay20ms();
					}
					break;
				case 0xed:
					temp=7;
					break;
				case 0xee:
					temp=3;
					break;
			}
			if(temp!=10 && j<6)
			{
				num[j]=temp;
				temp=10;
				j++;
			}else temp=10;
		}
		//数码管单元
		if(menu_index==1)
		{
			if(mode==1)
			{
				menu1[2]=0xbf;menu1[5]=0xbf;
				menu1[0]=tab[hour/10];
				menu1[1]=tab[hour%10];
				menu1[3]=tab[min/10];
				menu1[4]=tab[min%10];
				menu1[6]=tab[sec/10];
				menu1[7]=tab[sec%10];
			}else if(mode==2)
			{
				menu1[0]=0xbf;menu1[1]=0xbf;
				menu1[2]=tab[num[0]];
				menu1[3]=tab[num[1]];
				menu1[4]=tab[num[2]];
				menu1[5]=tab[num[3]];
				menu1[6]=tab[num[4]];
				menu1[7]=tab[num[5]];
			}
		}
		if(menu_index==2)
		{
			if(set_flag==0)
			{
				menu2[0]=0xff;menu2[1]=0xbf;
				menu2[2]=tab[num[0]];
				menu2[3]=tab[num[1]];
				menu2[4]=tab[num[2]];
				menu2[5]=tab[num[3]];
				menu2[6]=tab[num[4]];
				menu2[7]=tab[num[5]];
			}else if(set_flag==1)
			{
				menu2[0]=0xbf;menu2[1]=0xff;
				menu2[2]=tab[num[0]];
				menu2[3]=tab[num[1]];
				menu2[4]=tab[num[2]];
				menu2[5]=tab[num[3]];
				menu2[6]=tab[num[4]];
				menu2[7]=tab[num[5]];
			}
		}
		//继电器和蜂鸣器单元
		if(relay==1 && buzz==1)
		{
			P0=0x50;
			P2=0xA0;P0=0x50;P2=0;
		}else if(relay==1 && buzz==0)
		{
			P0=0x10;
			P2=0xA0;P0=0x10;P2=0;
		}else if(relay==0 && buzz==1)
		{
			P0=0x40;
			P2=0xA0;P0=0x40;P2=0;
		}else 
		{
			P0=0x00;
			P2=0xA0;P0=0x00;P2=0;
		}
	}
}
uchar key_cnt,smg_cnt,i;
uchar sonic_cnt;
uint relay_time,buzz_time;
void Timer0()interrupt 1
{
	key_cnt++;smg_cnt++;sonic_cnt++;
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
		if(menu_index==1){P2=0xe0;P0=menu1[i];P2=0;}
		if(menu_index==2){P2=0xe0;P0=menu2[i];P2=0;}
		i++;
		if(i==8)i=0;
	}
	if(relay)
	{
		relay_time++;
		if(relay_time==5000){relay_time=0;relay=0;}
	}else relay_time=0;
	if(buzz)
	{
		buzz_time++;
		if(buzz_time==3000){buzz_time=0;buzz=0;}
	}else buzz_time=0;
}