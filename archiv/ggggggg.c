
//#define F_CPU 8000000UL
#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>
#include <stdbool.h>
#include <inttypes.h>
#include "ili9341.h"
#include "ili9341gfx.h"
#include <avr/pgmspace.h>
#include "grn_TWI.h"

#define DPS310_W 0xee
#define DPS310_R 0xef
#define PSR_B2	0x00
#define PSR_B1	0x01
#define PSR_B0	0x02
#define TMP_B2 	0x03
#define TMP_B1	0x04
#define TMP_B0	0x05
#define PRS_CFG	0x06
#define TMP_CFG	0x07
#define MEAS_CFG	0x08
#define CFG_REG		0x09
#define INT_STS		0x0A
#define FIFO_STS	0x0B
#define RESET		0x0C
#define PRODUCT_ID	0x0D
#define COEF_SRCE	0x28

#define SENS_OP_STATUS 0x08

#define MODE_STBY	0x00
#define MODE_COMMAND_PRES 0x01
#define MODE_COMMAND_TEMP 0x02
#define MODE_COMMAND_PRES_AND_TEMP 0x03
#define MODE_BACKGROUND_PRES 0x05
#define MODE_BACKGROUND_TEMP 0x06
#define MODE_BACKGROUND_PRES_AND_TEMP 0x07
#define POINTCOLOUR PINK

extern uint16_t vsetx,vsety,vactualx,vactualy,isetx,isety,iactualx,iactualy;
static FILE mydata = FDEV_SETUP_STREAM(ili9341_putchar_printf, NULL, _FDEV_SETUP_WRITE);
uint8_t result,  xpos, error, x;
uint16_t xx, yy, zell, COLOR, var_x,color;

//compensation coefficients
int16_t m_C0;
int16_t m_C1;
int32_t m_C00;
int32_t m_C10;
int16_t m_C01;
int16_t m_C11;
int16_t m_C20;
int16_t m_C21;
int16_t m_C30;
int8_t t_vorko;
uint16_t p_vorko, t_nachko, p_nachko;
uint8_t buffer[3] = {0};
int32_t temp,temptemp;
int32_t temp_raw, temp_comp;
int32_t pres;

float Pressure;
float Temperature;

uint8_t bit;

// String für Zahlenausgabe
char string[30] = "";

uint8_t DPS310_read(uint8_t reg)
{
		uint8_t result=0;
		
		TWIStart();
		if(TWIGetStatus() != 0x08)return 123;
		TWIWrite(DPS310_W);
		if(TWIGetStatus() != 0x18)return 2;
		TWIWrite(reg);
		if(TWIGetStatus() != 0x28)return 3;
		TWIStart();
		if(TWIGetStatus() != 0x10)return 4; //repetet Start sent?
		TWIWrite(DPS310_R);
		if(TWIGetStatus() != 0x40)return 5;
		result=TWIReadNACK();
		TWIStop();
	return result;	
//Daten zurueckgeben
}
uint8_t DPS310_write(uint8_t reg, uint8_t data)
{
		TWIStart();
		if(TWIGetStatus() != 0x08)return 11;
		TWIWrite(DPS310_W);
		if(TWIGetStatus() != 0x18)return 22;
		TWIWrite(reg);
		if(TWIGetStatus() != 0x28)return 33;
		TWIWrite(data);
		if(TWIGetStatus() != 0x28)return 44;
		TWIStop();
	return 0;	
	
	//Daten zurueckgeben
}

int16_t DPS310_readCoeffs(void)
{
	uint16_t buffer[19];//coeffizienten
	uint8_t coeff_start;
	coeff_start=0x10;
	
	//coeffizienten einlesen und in buffer-Array speichern
	//Addressen 0x10 - 0x21
	for(x=0;x<18;x++)
	{
		buffer[x]=DPS310_read(coeff_start);
		_delay_ms(10);
		coeff_start++;
	}
	 
    m_C0=(((int)buffer[0]<<8)|buffer[1])>>4;
    m_C0=m_C0/2;
   
    m_C1=((((int)buffer[1]<<8)|buffer[2])<<4)>>4;
      
    m_C00= ((((long)buffer[3]<<8)|buffer[4])<<8)|buffer[5];
    m_C00=(m_C00<<8)>>12;

    m_C10=((((long)buffer[5]<<8)|buffer[6])<<8)|buffer[7];
    m_C10=(m_C10<<12)>>12;

    m_C01=((int)buffer[8]<<8)|buffer[9];

    m_C11=((int)buffer[10]<<8)|buffer[11];

    m_C20=((int)buffer[12]<<8)|buffer[13];

    m_C21=((int)buffer[14]<<8)|buffer[15];

    m_C30=((int)buffer[16]<<8)|buffer[17];
       
    return 0;
}
void printcoeffs(void)
{
	ili9341_setcursor(0,0);
	printf("1= %d", m_C0);
	ili9341_setcursor(0,20);
	printf("2= %d", m_C1);
	ili9341_setcursor(0,40);
	printf("3= %ld", m_C00);
	ili9341_setcursor(0,60);
	printf("4= %ld", m_C10);
	ili9341_setcursor(0,80);
	printf("5= %d", m_C01);
	ili9341_setcursor(0,100);
	printf("6= %d", m_C11);
	ili9341_setcursor(0,120);
	printf("7= %d", m_C20);
	ili9341_setcursor(0,140);
	printf("8= %d", m_C21);
	ili9341_setcursor(0,160);
	printf("9= %d", m_C30);
}
int main(void)
{
	stdout = & mydata;
	ili9341_init();//initial driver setup to drive ili9341
	ili9341_clear(BLACK);//fill screen with black colour
	_delay_ms(1000);
	ili9341_setRotation(3);//rotate screen
	_delay_ms(2);

	//display_init();//display initial data
	yy=240;
	xx=0;
	zell=0;
	color=123;
	var_x=0x01;
	
	t_vorko=0;
	t_nachko=0;
	p_vorko=0;
	p_nachko=0;
	
	pres=0;
	bit=0b10000000;
	
temp_raw=0;
	ili9341_settextcolour(YELLOW,BLACK);
	ili9341_setcursor(0,0);
	ili9341_settextsize(2);
	
	TWIInit();
	_delay_ms(500);
	DPS310_readCoeffs();
	_delay_ms(50);
	DPS310_write(PRS_CFG, 0b00110011);//eight times low power
	_delay_ms(50);
	DPS310_write(TMP_CFG, 0b00110011);// times
	_delay_ms(50);
	DPS310_write(CFG_REG, 0x00);
	_delay_ms(50);
		
	while(1)
	{
		DPS310_write(MEAS_CFG, MODE_COMMAND_PRES_AND_TEMP);//temperature meassurement
		_delay_ms(500);
		
		buffer[0]= DPS310_read(TMP_B2);//TMP_B2
		_delay_ms(500);
		buffer[1]= DPS310_read(TMP_B1);//TMP_B1
		_delay_ms(500);
		buffer[2]= DPS310_read(TMP_B0);//TMP_B0
		_delay_ms(500);
		
		temp=0;
		//compose raw temperature value from buffer
		temp=((((long)buffer[0]<<8)|buffer[1])<<8)|buffer[2];
		temp=(temp<<8)>>8;
		
		bit = DPS310_read(SENS_OP_STATUS);
		if (bit & (1 << 6))
		{
		
		ili9341_setcursor(0,0);
		printf("TMP_B2 = %d", buffer[0]);
		ili9341_setcursor(0,20);
		printf("TMP_B1 = %d", buffer[1]);
		ili9341_setcursor(0,40);
		printf("                     ");
		ili9341_setcursor(0,40);
		printf("TMP_B0 = %d", buffer[2]);
		ili9341_setcursor(0,60);
		printf("temp= %ld", temp);
		
		}
	
	buffer[0]=0;
	buffer[1]=0;
	buffer[2]=0;
	
	
		
		DPS310_write(MEAS_CFG, MODE_COMMAND_PRES_AND_TEMP);//temperature meassurement
		_delay_ms(500);
		
		buffer[0]= DPS310_read(PSR_B2);//TMP_B2
		_delay_ms(500);
		buffer[1]= DPS310_read(PSR_B1);//TMP_B1
		_delay_ms(500);
		buffer[2]= DPS310_read(PSR_B0);//TMP_B0
		_delay_ms(500);
		
		pres=0;
		//compose raw temperature value from buffer
		pres=((((long)buffer[0]<<8)|buffer[1])<<8)|buffer[2];
		pres=(temp<<8)>>8;
		
		
		
		
		
		
		
		ili9341_setcursor(0,110);
		printf("PSR2 = %d", buffer[0]);
		ili9341_setcursor(0,130);
		printf("PSR1 = %d", buffer[1]);
		ili9341_setcursor(0,150);
		printf("                     ");
		ili9341_setcursor(0,150);
		printf("PSR0 = %d", buffer[2]);
		
		ili9341_setcursor(0,170);
		printf("pres= %ld", pres);
		
	
		
		
		
		/*ili9341_setcursor(0,120);
		printf("temptemp= %ld", temptemp);
		
		ili9341_setcursor(0,160);
		printf("Tempe= %lf", Temperature);
		*/
	
		//while(1);
		
		_delay_ms(50);
	
	}//end of while

}//end of main
