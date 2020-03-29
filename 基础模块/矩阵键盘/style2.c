#include<stc15.h>
typedef unsigned int uint;
typedef unsigned char uchar;
uchar code Bit[]={0x01,0x02,0x04,0x08,0x10,0x20,0x40,0x80};
uchar code tab[]={0xc0,0xf9,0xa4,0xb0,0x99,0x92,0x82,0xf8,0x80,0x90};
uchar smg[8];
uchar i,key_cnt,smg_cnt;
bit key_flag;

#define set(x) P4=(x>>3|x>>4);P3=x
#define get()  ((P4&0x10)<<3)|((P4&0x04)<<4)|(P3&0x3F)
uchar Trg=0,Cont=0;
void KBD()
{
	uchar dat;
	set(0x0F);
	dat=get();
	set(0xF0);
	dat=(dat|get())^0xFF;
	Trg=dat&(dat ^ Cont);
	Cont=dat;
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
uchar num;
void main()
{
	P2=0xA0;P0=0;
	Timer0Init();
	while(1)
	{
		if(key_flag==1)
		{
			key_flag=0;
			KBD();
			switch(Trg ^ 0xFF)
			{
				case 0x77:
					num=4;break;
				case 0x7B:
					num=5;break;
				case 0x7D:
					num=6;break;
				case 0x7E:
					num=7;break;
				case 0xB7:
					num=8;break;
				case 0xBB:
					num=9;break;
				case 0xBD:
					num=10;break;
				case 0xBE:
					num=11;break;
			}
		}
		//数码管
		smg[0]=tab[num/10];
		smg[1]=tab[num%10];
		smg[2]=0xFF;smg[3]=0xFF;smg[4]=0xFF;
		smg[5]=0xFF;smg[6]=0xFF;smg[7]=0xFF;
	}
}

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
		P2=0xC0;P0=Bit[i];P2=0;
		P2=0xE0;P0=smg[i];P2=0;
		i++;
		if(i==8)i=0;
	}
}
