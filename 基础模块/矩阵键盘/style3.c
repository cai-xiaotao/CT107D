#include<stc15.h>
#include<intrins.h>
typedef unsigned int uint;
typedef unsigned char uchar;

uchar code tab[]={0xc0,0xf9,0xa4,0xb0,0x99,0x92,0x82,0xf8,0x80,0x90,0xFF};
uchar smg[8];

bit key_flag;

uchar state;
uchar KBD()
{
	uchar key_val=0,key_temp=0;
	uchar key1,key2;
	
	P32=1;P33=1;P34=0;P35=0;P42=0;P44=0;
	if(P33==0)key1=0x07;
	if(P32==0)key1=0x0b;
	if(P33==1 && P32==1)key1=0x0f;
	
	P32=0;P33=0;P34=1;P35=1;P42=1;P44=1;
	if(P44==0)key2=0x70;
	if(P42==0)key2=0xb0;
	if(P35==0)key2=0xd0;
	if(P34==0)key2=0xe0;
	if(P44==1 && P42==1 && P35==1 && P34==1)key2=0xf0;
	key_temp=key1|key2;
	
	switch(state)
	{
		case 0:
			if(key_temp!=0xff)state=1;
			break;
		case 1:
			if(key_temp==0xff)state=0;
			else 
			{
				switch(key_temp)
				{
					case 0x77:key_val=4;break;
					case 0x7b:key_val=5;break;
					case 0xb7:key_val=8;break;
					case 0xbb:key_val=9;break;
				}
				state=2;
			}
			break;
		case 2:
			if(key_temp==0xff)
			{
				state=0;
			}	
			break;
	}
	return key_val;
}
void AllInit()
{
	P2=0x80;P0=0xFF;
	P2=0xA0;P0=0x00;
	P2=0xC0;P0=0xFF;
	P2=0xE0;P0=0xFF;
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
	uchar key,num;
	AllInit();
	Timer0Init();
	while(1)
	{
		//按键单元以S4、5、7、8举例
		if(key_flag)
		{
			key_flag=0;
			key=KBD();
			if(key==4)
			{
				num=4;
			}else if(key==5)
			{
				num=5;
			}else if(key==8)
			{
				num=8;
			}else if(key==9)
			{
				num=9;
			}
		}
		//数码管单元
		smg[0]=0xff;smg[1]=0xff;
		smg[2]=0xff;smg[3]=0xff;
		smg[4]=0xff;smg[5]=0xff;
		smg[6]=0xff;smg[7]=tab[num];
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
		P2=0xE0;P0=0xFF;P2=0;
		P2=0xC0;P0=0x01<<i;P2=0;
		P2=0xE0;P0=smg[i];P2=0;
		i++;
		if(i==8)i=0;
	}
}