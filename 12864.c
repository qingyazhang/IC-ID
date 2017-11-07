/*-----------------------------------------------
  ���ƣ�st7920�ֿ�Һ����ʾ
  ��̳��www.doflye.net
  ��д��shifang
  ���ڣ�2009.5
  �޸ģ���
  ���ݣ�ͨ����ʾ�ַ������֡����ֺ�ͼƬ����Һ����������
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
//M1����ĳһ��дΪ���¸�ʽ����ÿ�ΪǮ�����ɽ��տۿ�ͳ�ֵ����
//4�ֽڽ����ֽ���ǰ����4�ֽڽ��ȡ����4�ֽڽ�1�ֽڿ��ַ��1�ֽڿ��ַȡ����1�ֽڿ��ַ��1�ֽڿ��ַȡ�� 
unsigned char code data2[4]  = {0,0,0,0x01};
unsigned char code DefaultKey[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}; 


unsigned char g_ucTempbuf[20];  


uchar xdata icstr[9] = 0;                             //�����飬Ҫ��ʾ��8λ  ic
uchar cuctemp[4] = 0;

uchar xdata idstr[27] = 0;		     //id
uchar idtemp[13] = 0;

uchar idbuffnum = 0;

//��ʱ�ӳ���ģ��
//**********************************************
void mdelay(uint delay)
{	uint i;
 	for(;delay>0;delay--)
   		{for(i=0;i<80;i++) //1ms��ʱ.
       		{;}
   		}
}

//************************************************

void show();   //Һ����ʾ����
//****************************************
//12864Һ����ʾ�����ӳ���ģ��
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
//Һ����ʼ��
//**********************************
void init_lcd(void)
{
	wr_i_lcd(0x06);  /*�����ƶ�����*/
	wr_i_lcd(0x0c);  /*����ʾ�����α�*/
	wr_i_lcd(0x01);
	delaye(5);
}
//***********************************
//���Һ��DDRAMȫΪ�ո�
//**********************************
void clrram_lcd (void)
{
	wr_i_lcd(0x30);
	wr_i_lcd(0x01);
}
//***********************************
//��Һ��д����
//contentΪҪд�������
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
//��Һ��дָ��
//contentΪҪд���ָ�����
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
//Һ�����æ״̬
//��д��֮ǰ����ִ��
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
//ָ��Ҫ��ʾ�ַ�������
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
//Һ����ʾ�ַ�������
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

 	S2CON = 0x50;//����2�����ڷ�ʽ1??10λ�첽�շ� S2REN=1�������
	BRT = 0xFd;//���������ʷ�������ֵ
	AUXR = 0x10;//BRTR=1 ���������ʷ�������ʼ����
	IE2 =0x01;//������2�ж�   ES2=1
}


void sentbit(unsigned char dat) 
{
	SBUF = dat;
	while(!TI);
	TI = 0;	

}


void send_str(unsigned char* p)//�����ַ���
{
	unsigned char i=0;
	while(*p > 0)
	{
		sentbit(*p++);
	}
}


/****************���п�2����****************/
	
void UART_2SendOneByte(unsigned char c)
{
	S2BUF = c;
	while(!(S2CON&S2TI));//��S2TI=0���ڴ˵ȴ�
	S2CON&=~S2TI;//S2TI=0
}
	
void delay(unsigned int z)
{
	unsigned int x,y;
	for(x=z;x>0;x--)
	for(y=600;y>0;y--);
}



void convert(uchar *ps, uchar *num_ID, int n)	 //��ʾ����
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





/************���п�1�жϴ�����*************/
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

/************���п�2�жϴ�����*************/


 
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
                 ������
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
	print("IC���ţ�");

	sentbit('o');
	sentbit('k');

	LED_GREEN = 1;
	while(1)
	{
	    //���ڽ�������
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
			print("ID���ţ�");
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
	   status = PcdRequest(PICC_REQALL, g_ucTempbuf);//Ѱ��


	  /* if (status != MI_OK)
        {    
			continue;
        } 
			 */
		 status = PcdAnticoll(g_ucTempbuf);//����ײ
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
			print("IC���ţ�");
			gotoxy(2,0);
			print(icstr); 
			while(MI_OK == status){
				status = PcdAnticoll(g_ucTempbuf);//����ײ
			}
		}
	}

}
