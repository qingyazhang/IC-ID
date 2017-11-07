/*-----------------------------------------------
  名称：st7920字库液晶显示
  论坛：www.doflye.net
  编写：shifang
  日期：2009.5
  修改：无
  内容：通过显示字符、数字、汉字和图片测试液晶基本功能
------------------------------------------------*/
#include <STC12C5A.H>
#include "mfrc522.h"
#include <intrins.h>
#include  <STDIO.H>
#include <string.h>


#define uint unsigned int
#define uchar unsigned char


sbit rs		= P2^0;
sbit rw		= P2^1;
sbit e  	= P2^2;

#define lcddata P0


unsigned char artemp1 = 0xff, artemp2 = 0xff;

bit arflag1 = 0,arflag2 = 0;

//unsigned char aurs1[50],aurs2[50];


unsigned char code data1[16] = {0x12,0x34,0x56,0x78,0xED,0xCB,0xA9,0x87,0x12,0x34,0x56,0x78,0x01,0xFE,0x01,0xFE};
//M1卡的某一块写为如下格式，则该块为钱包，可接收扣款和充值命令
//4字节金额（低字节在前）＋4字节金额取反＋4字节金额＋1字节块地址＋1字节块地址取反＋1字节块地址＋1字节块地址取反 
unsigned char code data2[4]  = {0,0,0,0x01};
unsigned char code DefaultKey[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}; 


unsigned char g_ucTempbuf[20];  


uchar xdata icstr[9] = 0;                             //空数组，要显示的8位  ic
uchar cuctemp[4] = 0;

uchar xdata idstr[27] = 0;		     //id
uchar idtemp[13] = 0;

uchar idbuffnum = 0;

//延时子程序模块
//**********************************************
void mdelay(uint delay)
{	uint i;
 	for(;delay>0;delay--)
   		{for(i=0;i<80;i++) //1ms延时.
       		{;}
   		}
}

//************************************************

void show();   //液晶显示程序
//****************************************
//12864液晶显示部分子程序模块
//****************************************



sbit busy=P0^7;   //lcd busy bit
void wr_d_lcd(uchar content);
void wr_i_lcd(uchar content);
void clrram_lcd (void);
void init_lcd(void);
void busy_lcd(void);
void rev_row_lcd(uchar row);
void rev_co_lcd(uchar row,uchar col,uchar mode);
void clr_lcd(void);
void wr_co_lcd(uchar row,uchar col,uchar lcddata1,uchar lcddtta2);
void wr_row_lcd(uchar row,char *p);



void delaye(int ms)
{
    while(ms--)
	{
      uchar i;
	  for(i=0;i<250;i++)  
	   {
	    _nop_();			   
		_nop_();
		_nop_();
		_nop_();
	   }
	}
}

//**********************************
//液晶初始化
//**********************************
void init_lcd(void)
{
	wr_i_lcd(0x06);  /*光标的移动方向*/
	wr_i_lcd(0x0c);  /*开显示，关游标*/
	wr_i_lcd(0x01);
	delaye(5);
}
//***********************************
//填充液晶DDRAM全为空格
//**********************************
void clrram_lcd (void)
{
	wr_i_lcd(0x30);
	wr_i_lcd(0x01);
}
//***********************************
//对液晶写数据
//content为要写入的数据
//***********************************
void wr_d_lcd(uchar content)
{
	busy_lcd();
	rs=1;
    rw=0;
	lcddata=content;
	e=1;
	;
	e=0;
}
//********************************
//对液晶写指令
//content为要写入的指令代码
//*****************************
void wr_i_lcd(uchar content)
{
	busy_lcd();
	rs=0;
    rw=0;
	lcddata=content;
	e=1;
	;
	e=0;
}
//********************************
//液晶检测忙状态
//在写入之前必须执行
//********************************
void busy_lcd(void)
{
  lcddata=0xff;
  rs=0;
  rw=1;
  e =1;
  while(busy==1);
  e =0;
}
//********************************
//指定要显示字符的坐标
//*******************************
void gotoxy(unsigned char y, unsigned char x)
{
	if(y==1)
		wr_i_lcd(0x80|x);
	if(y==2)
        wr_i_lcd(0x90|x);
	if(y==3)
		wr_i_lcd((0x80|x)+8);
	if(y==4)
        wr_i_lcd((0x90|x)+8);
}
//**********************************
//液晶显示字符串程序
//**********************************
void print(uchar *str)
{
	while(*str!='\0')
	{
		wr_d_lcd(*str);
		str++;
	}
}




void UartInit(void)		//9600bps@11.0592MHz
{
	 TMOD = 0x20;
	 SCON = 0x50;
 	 TH1 = 0xFd;
	 TL1 = TH1;
	 TR1 = 1;
	 EA =1;
	 ES =1;

 	S2CON = 0x50;//串口2工作在方式1??10位异步收发 S2REN=1允许接收
	BRT = 0xFd;//独立波特率发生器初值
	AUXR = 0x10;//BRTR=1 独立波特率发生器开始计数
	IE2 =0x01;//开串口2中断   ES2=1
}


void sentbit(unsigned char dat) 
{
	SBUF = dat;
	while(!TI);
	TI = 0;	

}


void send_str(unsigned char* p)//发送字符串
{
	unsigned char i=0;
	while(*p > 0)
	{
		sentbit(*p++);
	}
}


/****************串行口2发送****************/
	
void UART_2SendOneByte(unsigned char c)
{
	S2BUF = c;
	while(!(S2CON&S2TI));//若S2TI=0，在此等待
	S2CON&=~S2TI;//S2TI=0
}
	
void delay(unsigned int z)
{
	unsigned int x,y;
	for(x=z;x>0;x--)
	for(y=600;y>0;y--);
}



void convert(uchar *ps, uchar *num_ID, int n)	 //显示卡号
{
	unsigned char j=0;
	for(j = 0; j < n; j++)			  
	{
		ps[2*j] = (num_ID[j] & 0xf0) >> 4; 

		if(ps[2*j] >= 10) {
		 ps[2*j] = ps[2*j] + 48 + 7;
		}
		else {
		   ps[2*j] = ps[2*j] + 48;
		}

		ps[2*j+1] = num_ID[j] & 0x0f;

		if(ps[2*j+1] >= 10) {
		  ps[2*j+1] = ps[2*j+1] + 48 + 7;
		}
		else {
		   ps[2*j+1] = ps[2*j+1] + 48;
		}
		
	}	 
}





/************串行口1中断处理函数*************/
void UART_1Interrupt(void) interrupt 4
{
	 if(RI==1)
	 {
		RI=0;
		artemp1=SBUF;
		if(idbuffnum < 13) {

			sentbit(artemp1);
		   	idtemp[idbuffnum] = artemp1;
			idbuffnum++;
		}
		else {
				arflag1=1;
		
		}
	 }
}

/************串行口2中断处理函数*************/


 
void UART_2Interrupt(void) interrupt 8
{
	if(S2CON&S2RI)
	{
		S2CON&=~S2RI;
		arflag2=1;
		artemp2=S2BUF;
	    sentbit(artemp2);
	} 
}



	 
/*------------------------------------------------
                 主程序
------------------------------------------------*/
void main()
{
	unsigned char i=0;
	unsigned char status,temp;
	UartInit();

	init_lcd();

	   //RC522
     PcdReset();
     PcdAntennaOff(); 
     PcdAntennaOn();

	gotoxy(1,0);
	print("IC卡号：");

	sentbit('o');
	sentbit('k');

	LED_GREEN = 1;
	while(1)
	{
	    //串口接收数据
	   if(arflag1 == 1) 
	   {
		 LED_GREEN = 0;

			convert(idstr,idtemp,13);
			init_lcd();
	/*	for(i=0;i<26;i++) {
				sentbit(idtemp[i/2]);
			  sentbit(idstr[i]);
		} */
			gotoxy(1,0);
			print("ID卡号：");
			gotoxy(2,0);
				print(idstr); 
			idbuffnum = 0;
			arflag1 = 0;	
	   }

	   /*
	   if(arflag2 == 1)
	   {
			sentbit(artemp2);
			arflag2 = 0;
			S2CON&=~S2RI;
	   }  */
	   status = PcdRequest(PICC_REQALL, g_ucTempbuf);//寻卡


	  /* if (status != MI_OK)
        {    
			continue;
        } 
			 */
		 status = PcdAnticoll(g_ucTempbuf);//防冲撞
		 if (status != MI_OK){continue;}
		if(rcc == 1) {
			rcc = 1;
		}
		else {
		    for(i=0;i<4;i++)
			{
				temp=g_ucTempbuf[i];
				cuctemp[i] = temp;
				sentbit(cuctemp[i]);
				rcc = 1;
			}

			LED_GREEN = 0;

			convert(icstr,cuctemp,4);
			init_lcd();
			gotoxy(1,0);
			print("IC卡号：");
			gotoxy(2,0);
			print(icstr); 
			while(MI_OK == status){
				status = PcdAnticoll(g_ucTempbuf);//防冲撞
			}
		}
	}

}
