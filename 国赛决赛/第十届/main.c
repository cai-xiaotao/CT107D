#include<stc15.h>
#include<iic.h>
#include<onewire.h>
#include<intrins.h>
typedef unsigned int uint;
typedef unsigned char uchar;

sbit TX=P1^0;
sbit RX=P1^1;

uchar code tab[]={0xc0,0xf9,0xa4,0xb0,0x99,0x92,0x82,0xf8,0x80,0x90,0xFF};
uchar menu1[8];
uchar menu2[8];
uchar menu3[8];
uchar menu4[8];
uchar buf[6];
uchar st[11];
uchar para[8];
uchar error[7];

uchar menu_index=1;
bit set_index=1;
bit key_flag,sonic_flag=0;
bit k12=0,k13=0,k_flag=0,dac_flag=1;
uchar k;//buf[]的索引
bit finish=0;

bit busy=0;
void SendData(uchar dat)
{
	while(busy);
	busy=1;
	SBUF=dat;
}

uchar state;
uchar KBD()
{
	uchar key_temp=0,key_val=0;
	uchar key1,key2;
	
	P32=1;P33=1;P34=0;P35=0;P42=0;P44=0;
	if(P32==0)key1=0x0b;
	if(P33==0)key1=0x07;
	if(P32==1 && P33==1)key1=0x0f;
	
	P32=0;P33=0;P34=1;P35=1;P42=1;P44=1;
	if(P34==0)key2=0xe0;
	if(P35==0)key2=0xd0;
	if(P42==0)key2=0xb0;
	if(P44==0)key2=0x70;
	if(P34==1 && P35==1 && P42==1 && P44==1)key2=0xf0;
	key_temp=key1|key2;
	
	switch(state)
	{
		case 0:
			if(key_temp!=0xff)
				state=1;
			break;
		case 1:
			if(key_temp==0xff)state=0;
			else
			{
				switch(key_temp)
				{
					case 0xd7:key_val=12;break;
					case 0xdb:key_val=13;break;
					case 0xe7:key_val=16;break;
					case 0xeb:key_val=17;break;
				}
				state=2;
			}
			break;
		case 2:
			if(key_temp==0xff)
			{
				state=0;
				k12=0;k13=0;
			}
		break;
	}
	return key_val;
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

void Delay20us()		//@11.0592MHz
{
	unsigned char i;

	_nop_();
	_nop_();
	_nop_();
	i = 52;
	while (--i);
}

void Timer0Init(void)		//1毫秒@12.000MHz
{
	AUXR |= 0x80;		//定时器时钟1T模式
	TMOD &= 0xF0;		//设置定时器模式
	TL0 = 0x20;		//设置定时初值
	TH0 = 0xD1;		//设置定时初值
	TF0 = 0;		//清除TF0标志
	TR0 = 1;		//定时器0开始计时
	ET0=1;
	EA=1;
}

void Timer1Init(void)		//@12.000MHz
{
	AUXR &= 0xBF;		//定时器时钟12T模式
	TMOD &= 0x0F;		//设置定时器模式
}

void UartInit(void)		//4800bps@11.0592MHz
{
	SCON = 0x50;		//8位数据,可变波特率
	AUXR |= 0x01;		//串口1选择定时器2为波特率发生器
	AUXR |= 0x04;		//定时器2时钟为Fosc,即1T
	T2L = 0xC0;		//设定定时初值
	T2H = 0xFD;		//设定定时初值
	AUXR |= 0x10;		//启动定时器2
	ES=1;
}

void main()
{
	bit l1=0,l2=0,l3=0;
	uint n;
	uchar key,j;
	uchar vout;
	uchar last_temp=30,last_dist=35;
	uchar temp_val=30,dist_val=35;
	uchar distance;
	uint temp;
	AllInit();
	rd_temperature();
	Delay750ms();
	Timer0Init();
	Timer1Init();
	UartInit();
	while(1)
	{
		temp=rd_temperature()*100;
		//超声波单元
		if(sonic_flag)
		{
			sonic_flag=0;
			TX=1;
			Delay20us();
			TX=0;
			while(!RX);
			TH1=TL1=0;
			TF1=0;TR1=1;
			while(RX==1 && TF1==0);
			TR1=0;
			if(TF1==1){TF1=0;distance=99;}
			else 
			{
				distance=(TH1<<8|TL1)*0.017;
				if(distance>99)distance=99;
			}
		}
		//按键单元
		if(key_flag)
		{
			key_flag=0;
			key=KBD();
			if(key==12)
			{
				k12=1;
				if(menu_index!=4)
				{
					menu_index++;
					if(menu_index==4)menu_index=1;
				}else set_index=~set_index;
			}else if(key==13)
			{
				k13=1;
				if(menu_index!=4)
				{
					menu_index=4;
					set_index=0;
				}else 
				{
					menu_index=1;
					if(last_temp != temp_val)
					{
						last_temp=temp_val;
						n++;
					}
					if(last_dist != dist_val)
					{
						last_dist=dist_val;
						n++;
					}
					write_rom(0,n>>8);Delay20ms();
					write_rom(1,n);Delay20ms();
				}
			}else if(key==16)
			{
				if(menu_index==4)
				{
					if(set_index==0 && temp_val>0)
					{
						temp_val-=2;
						if(temp_val<0)temp_val=0;
					}else if(set_index==1 && dist_val>0)
					{
						dist_val-=5;
						if(dist_val<0)dist_val=0;
					}
				}
			}else if(key==17)
			{
				if(menu_index==4)
				{
					if(set_index==0 && temp_val<99)
					{
						temp_val+=2;
						if(temp_val>99)temp_val=99;
					}else if(set_index==1 && dist_val<99)
					{
						dist_val+=5;
						if(dist_val>99)dist_val=99;
					}
				}
			}
		}
		if(k_flag)
		{
			k_flag=0;
			if(k12==1)
			{
				n=0;
			}else if(k13==1)
			{
				dac_flag=~dac_flag;
			}
		}
		//DAC
		if(distance > last_dist)
			vout=204;
		else vout=102;
		if(dac_flag)
		{
			l3=1;
			EA=0;
			write_dac(vout);
			EA=1;
			Delay20ms();
		}else 
		{
			l3=0;
			EA=0;
			write_dac(204/10);
			EA=1;
			Delay20ms();
		}
		//串口单元
		if(finish)
		{
			k=0;
			finish=0;
			if(buf[0]=='S' && buf[1]=='T' && buf[2]=='\r' && buf[3]=='\n')
			{
				st[0]='$';st[1]=distance/10+'0';
				st[2]=distance%10+'0';
				st[3]=',';
				st[4]=temp/1000+'0';
				st[5]=temp%100/100+'0';
				st[6]='.';
				st[7]=temp%100/10+'0';
				st[8]=temp%10+'0';
				st[9]='\r';st[10]='\n';
				for(j=0;j<11;j++)SendData(st[j]);
			}else if(buf[0]=='P' && buf[1]=='A' && buf[2]=='R' && buf[3]=='A' && buf[4]=='\r' && buf[5]=='\n')
			{
				para[0]='#';
				para[1]=last_dist/10+'0';
				para[2]=last_dist%10+'0';
				para[3]=',';
				para[4]=last_temp/10+'0';
				para[5]=last_temp%10+'0';
				para[6]='\r';
				para[7]='\n';
				for(j=0;j<8;j++)SendData(para[j]);
			}else 
			{
				error[0]='E';error[1]='R';error[2]='R';
				error[3]='O';error[4]='R';
				error[5]='\r';error[6]='\n';
				for(j=0;j<7;j++)SendData(error[j]);
			}
		}
		//数码管单元
		//C
		menu1[0]=0xc6;menu1[1]=0xff;
		menu1[2]=0xff;menu1[3]=0xff;
		menu1[4]=tab[temp/1000];
		menu1[5]=tab[temp%1000/100]&0x7f;
		menu1[6]=tab[temp%100/10];
		menu1[7]=tab[temp%10];
		//L
		menu2[0]=0xc7;menu2[1]=0xff;
		menu2[2]=0xff;menu2[3]=0xff;
		menu2[4]=0xff;menu2[5]=0xff;
		menu2[6]=tab[distance/10];
		menu2[7]=tab[distance%10];
		//N
		menu3[0]=0xc8;menu3[1]=0xff;menu3[2]=0xff;
		if(n>10000)
		{
			menu3[3]=tab[n/10000];
			menu3[4]=tab[n%10000/1000];
			menu3[5]=tab[n%1000/100];
			menu3[6]=tab[n%100/10];
			menu3[7]=tab[n%10];
		}else if(n>1000)
		{
			menu3[3]=0xff;
			menu3[4]=tab[n/1000];
			menu3[5]=tab[n%1000/100];
			menu3[6]=tab[n%100/10];
			menu3[7]=tab[n%10];
		}else if(n>100)
		{
			menu3[3]=0xff;menu3[4]=0xff;
			menu3[5]=tab[n/100];
			menu3[6]=tab[n%100/10];
			menu3[7]=tab[n%10];
		}else if(n>10)
		{
			menu3[3]=0xff;menu3[4]=0xff;
			menu3[5]=0xff;
			menu3[6]=tab[n/10];
			menu3[7]=tab[n%10];
		}else 
		{
			menu3[3]=0xff;menu3[4]=0xff;
			menu3[5]=0xff;menu3[6]=0xff;
			menu3[7]=tab[n];
		}
		//参数界面
		if(set_index==0)
		{
			menu4[0]=0x8c;menu4[1]=0xff;
			menu4[2]=0xff;menu4[3]=tab[1];
			menu4[4]=0xff;menu4[5]=0xff;
			menu4[6]=tab[temp_val/10];
			menu4[7]=tab[temp_val%10];
		}else if(set_index==1)
		{
			menu4[0]=0x8c;menu4[1]=0xff;
			menu4[2]=0xff;menu4[3]=tab[2];
			menu4[4]=0xff;menu4[5]=0xff;
			menu4[6]=tab[dist_val/10];
			menu4[7]=tab[dist_val%10];
		}
		//LED
		if(temp>last_temp*100)l1=1;
		else l1=0;
		if(distance < last_dist)l2=1;
		else l2=0;
		if(l1==1)
		{
			if(l2==1)
			{
				if(l3==1)
				{
					P0=0xf8;
					P2=0x80;P0=0xf8;P2=0;
				}else 
				{
					P0=0xfc;
					P2=0x80;P0=0xfc;P2=0;
				}
			}else
			{
				if(l3==1)
				{
					P0=0xfa;
					P2=0x80;P0=0xfa;P2=0;
				}else 
				{
					P0=0xfe;
					P2=0x80;P0=0xfe;P2=0;
				}
			}
		}else
		{
			if(l2==1)
			{
				if(l3==1)
				{
					P0=0xf9;
					P2=0x80;P0=0xf9;P2=0;
				}else 
				{
					P0=0xfd;
					P2=0x80;P0=0xfd;P2=0;
				}
			}else
			{
				if(l3==1)
				{
					P0=0xfb;
					P2=0x80;P0=0xfb;P2=0;
				}else 
				{
					P0=0xff;
					P2=0x80;P0=0xff;P2=0;
				}
			}
		}
	}
}

uchar smg_cnt,i;
uchar key_cnt;
uchar sonic_cnt;
uint k_cnt;
void Timer0()interrupt 1
{
	smg_cnt++;key_cnt++;sonic_cnt++;
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
		if(menu_index==4){P2=0xE0;P0=menu4[i];P2=0;}
		i++;
		if(i==8)i=0;
	}
	if(sonic_cnt==200)
	{
		sonic_cnt=0;
		sonic_flag=1;
	}
	if(k12==1 || k13==1)
	{
		k_cnt++;
		if(k_cnt==1000)
		{
			k_cnt=0;
			k_flag=1;
		}
	}else
	{
		k_cnt=0;
		k_flag=0;
	}
}

void Uart()interrupt 4
{
	if(RI)
	{
		RI=0;
		buf[k]=SBUF;
		k++;
		if(buf[0]=='S')
		{
			if(k==4){k=0;finish=1;}
		}else if(buf[0]=='P')
		{
			if(k==6){k=0;finish=1;}
		}else 
		{
			finish=1;
		}
	}
	if(TI)
	{
		TI=0;
		busy=0;
	}
}