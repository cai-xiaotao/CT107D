#include<stc15.h>
#include<onewire.h>
#include<intrins.h>
typedef unsigned int uint;
typedef unsigned char uchar;

uchar code tab[]={0xc0,0xf9,0xa4,0xb0,0x99,0x92,0x82,0xf8,0x80,0x90,0xFF};
uchar menu1[8];
uchar menu2[8]; 
uchar num[4]={10,10,10,10};

uchar temp_max=30,temp_min=20;
uchar smg_cnt,i;
uchar key_cnt;
uchar menu_flag=1;
bit key_flag;
bit fault,relay=0;
uchar flash;

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
uchar led_temp;
void main()
{
	uchar temp,tmp,j;
	P2=0xa0; P0=0; P2=0;
	P2=0x80; P0=0xff; P2=0;
	rd_temperature();
	Delay750ms();
	Timer0Init();
	while(1)
	{
		//按键单元
		if(key_flag==1)
		{
			key_flag=0;
			KBD();
			switch(Trg ^ 0xFF)
			{
				case 0x77:
					tmp=9;
					break;
				case 0x7b:
					tmp=6;
					break;
				case 0x7d:
					tmp=3;
					break;
				case 0x7e:
					tmp=0;
					break;
				case 0xb7:
					if(menu_flag==1)menu_flag=2;
					else 
					{
						j=0;
						temp_max=num[0]*10+num[1];
						temp_min=num[2]*10+num[3];
						if(temp_max<temp_min)fault=1;
						else fault=0;
						num[0]=10;num[1]=10;
						num[2]=10;num[3]=10;
						menu_flag=1;
					}
					break;
				case 0xbb:
					tmp=7;
					break;
				case 0xbd:
					tmp=4;
					break;
				case 0xbe:
					tmp=1;
					break;
				case 0xd7:
					if(menu_flag==2)
					{
						j=0;
						num[0]=10;num[1]=10;
						num[2]=10;num[3]=10;
					}
					break;
				case 0xdb:
					tmp=8;
					break;
				case 0xdd:
					tmp=5;
					break;
				case 0xde:
					tmp=2;
					break;
			}
			if(tmp!=10 && j<4 && menu_flag==2)
			{
				num[j]=tmp;
				tmp=10;
				j++;
			}else tmp=10;
		}
		//数码管单元
		if(menu_flag==1)
		{
			temp=rd_temperature();
			menu1[0]=0xbf;menu1[2]=0xbf;
			if(temp < temp_min)menu1[1]=tab[0];
			else if(temp > temp_max)menu1[1]=tab[2];
			else menu1[1]=tab[1];
			menu1[3]=0xff;menu1[4]=0xff;menu1[5]=0xff;
			menu1[6]=tab[temp/10];
			menu1[7]=tab[temp%10];
		}else if(menu_flag==2)
		{
			menu2[0]=0xbf;menu2[3]=0xff;
			menu2[4]=0xff;menu2[5]=0xbf;
			menu2[1]=tab[num[0]];
			menu2[2]=tab[num[1]];
			menu2[6]=tab[num[2]];
			menu2[7]=tab[num[3]];
		}
		//led和继电器单元
		if(temp < temp_min){relay=0;flash=1;}
		else if(temp > temp_max){relay=1;flash=3;}
		else {relay=0;flash=2;}
		if(relay)
		{
			P0=0x10;
			P2=0xa0;P0=0x10;P2=0;
		}else {P0=0x00;P2=0xa0;P0=0x00;P2=0;}
		if(fault)led_temp=0x02;
		else led_temp=0x00;
	}
}
uint led_cnt;
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
		P2=0xc0;P0=0x01<<i;P2=0;
		if(menu_flag==1){P2=0xe0;P0=menu1[i];P2=0;}
		if(menu_flag==2){P2=0xe0;P0=menu2[i];P2=0;}
		i++;
		if(i==8)i=0;
	}
	if(flash==1)
	{
		led_cnt++;
		if(led_cnt<800)
		{P2=0x80;P0=~(0x01|led_temp);P2=0;}
		else 
		{
			if(led_cnt==1600)led_cnt=0;
			P2=0x80;P0=~led_temp;P2=0;
		}
	}else if(flash==2)
	{
		led_cnt++;
		if(led_cnt<400)
		{P2=0x80;P0=~(0x01|led_temp);P2=0;}
		else 
		{
			if(led_cnt==800)led_cnt=0;
			P2=0x80;P0=~led_temp;P2=0;
		}
	}else if(flash==3)
	{
		led_cnt++;
		if(led_cnt<200)
		{P2=0x80;P0=~(0x01|led_temp);P2=0;}
		else 
		{
			if(led_cnt==400)led_cnt=0;
			P2=0x80;P0=~led_temp;P2=0;
		}
	}
}