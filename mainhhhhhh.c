
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
int32_t a, temp_raw, temp_comp, Temperature, Pressure;

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

uint8_t aaa, bbb,ccc;

int16_t rawtemp, tempcomp;  

// String für Zahlenausgabe
char string[30] = "";


int16_t DPS310_readCoeffs(void)
{
	uint16_t buffer[19];//coeffizienten
	uint8_t coeff_start;
	coeff_start=0x10;
	
	//coeffizienten einlesen und in buffer-Array speichern
	//Addressen 0x10 - 0x21
	for(x=0;x<18;x++)
	{
		buffer[x]=DPS310_read_8(coeff_start);
		_delay_ms(10);
		coeff_start++;
	}
	ccc=buffer[0];
	 aaa=buffer[1];
	 bbb=buffer[2];
	 
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
/*
long Get_Data (char MSB)
{
	uint8_t buffer[3];
	buffer[0]= DPS310_read_8(TMP_B2);
	buffer[1]= DPS310_read_8(TMP_B1);
	buffer[2]= DPS310_read_8(TMP_B0);
   
    long value = 0;
 
    value=((((long)buffer[0]<<8)|buffer[1])<<8)|buffer[2];
    value=(value<<8)>>8;
    return value;
}

void calculate_temperature_pressure()
{
       float Prs;
       long temptemp = 0;
       Temperature=0;
       Prs=(float)Get_Data( 0x00 );
       Temperature_Config();
       Temperature=(float)Get_Data( 0x03 );
       Temperature/=524288;
       temptemp = Temperature;
       Prs/=1572864;
       Temperature=m_C0+Temperature*m_C1;
       Pressure=m_C00+Prs*(m_C10+Prs*(m_C20+(Prs*C30)))+temptemp*C01+temptemp*Prs*(C11+(Prs*C21));
       Pressure/= 100;
}
* */
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
temp_comp=0;
temp_raw=0;
aaa=0;
bbb=0;
ccc=0;
result=0;
	ili9341_settextcolour(RED,BLACK);
	
	TWIInit();

ili9341_setcursor(40,40);

	
		/*	
		xpos=40;
		error=0;
		a=0;
		coeff_start=0x10;
		for(x=0;x<18;x++)
		{
			buffer[x]=DPS310_read_8(coeff_start);
			_delay_ms(10);
			coeff_start++;
		}

		
		
		
		for(x=0;x<8;x++)
		{
			ili9341_setcursor(0,a);
			printf("0x00 = %2x", buffer[x]);
			_delay_ms(10);
			a=a+30;
		}
*/
		DPS310_readCoeffs();
		ili9341_setcursor(0,0);
		ili9341_settextsize(3);
	
		
		
		
		DPS310_write(PRS_CFG,0x01);
		DPS310_write(TMP_CFG,0x10);
		DPS310_write(MEAS_CFG,0x07);//cont temp meas
		DPS310_write(CFG_REG,0x02);//enable fifo
	while(1)
	{
		DPS310_write(PRS_CFG,0x01);
		DPS310_write(TMP_CFG,0x10);
		DPS310_write(MEAS_CFG,0x07);//cont temp meas
		DPS310_write(CFG_REG,0x02);//enable fifo
		
		aaa= DPS310_read_8(TMP_B2);
		bbb= DPS310_read_8(TMP_B1);
		ccc= DPS310_read_8(TMP_B0);
		rawtemp=DPS310_gettemp();
		
		ili9341_setcursor(0,0);
		//temp_raw=DPS310_gettemp();
		printf("B2 %d", aaa);
		//temp_comp=DPS310_calcTemp(temp_raw, m_c0Half, m_c1);
		ili9341_setcursor(0,30);
		printf("B1 %d", bbb);
		ili9341_setcursor(0,60);
		printf("B0 %d", ccc);
		
		
		_delay_ms(500);
	
	}//end of while

}//end of main
