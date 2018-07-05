
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
#define PRS_B2	0x00
#define PRS_B1	0x01
#define PRS_B0	0x02
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

#define COEF_RDY 1
#define SENSOR_RDY 2
#define TMP_RDY 3
#define PRS_RDY 4
#define PROD_ID 5

#define SENS_OP_STATUS 0x08

#define MODE_STBY	0x00
#define MODE_COMMAND_PRES 0x01
#define MODE_COMMAND_TEMP 0x02
#define MODE_COMMAND_PRES_AND_TEMP 0x03
#define MODE_BACKGROUND_PRES 0x05
#define MODE_BACKGROUND_TEMP 0x06
#define MODE_BACKGROUND_PRES_AND_TEMP 0x07
#define POINTCOLOUR PINK

#define COEFF_READY DPS310_read(SENS_OP_STATUS) & (1 << 7);

extern uint16_t vsetx,vsety,vactualx,vactualy,isetx,isety,iactualx,iactualy;
static FILE mydata = FDEV_SETUP_STREAM(ili9341_putchar_printf, NULL, _FDEV_SETUP_WRITE);
uint8_t result,  xpos, error, x, value, rdy;
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

uint8_t buffer[3] = {0};
uint8_t meas=0;
uint8_t id=0;

float ttt=0;
long ccc=0;



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
		_delay_ms(2);
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
		
		_delay_ms(2);
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
   
    //m_C1=((((int)buffer[1]<<8)|buffer[2])<<4)>>4;
    
    m_C1 = (((uint32_t)buffer[1] & 0x0F) << 8) | (uint32_t)buffer[2];
	if(m_C1 & ((uint32_t)1 << 11))
	{
		m_C1 -= (uint32_t)1 << 12;
	}
      
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
	
	uint8_t xxx=150;
	ili9341_setcursor(xxx,0);
	printf("1= %d", m_C0);
	ili9341_setcursor(xxx,20);
	printf("2= %03d", m_C1);
	ili9341_setcursor(xxx,40);
	printf("3= %ld", m_C00);
	ili9341_setcursor(xxx,60);
	printf("4= %ld", m_C10);
	ili9341_setcursor(xxx,80);
	printf("5= %d", m_C01);
	ili9341_setcursor(xxx,100);
	printf("6= %d", m_C11);
	ili9341_setcursor(xxx,120);
	printf("7= %d", m_C20);
	ili9341_setcursor(xxx,140);
	printf("8= %d", m_C21);
	ili9341_setcursor(xxx,160);
	printf("9= %d", m_C30);
}

void DPS310_sreset(void)
{
	// softreset of DPS310 sensor
	DPS310_write(0x0c, 0x99);
	_delay_ms(50);
}

uint8_t DPS310_check(uint8_t checkbit)
{
	
	uint8_t tmpval=99;// temporary register value
	
	switch(checkbit)
	{
		case COEF_RDY:		if(DPS310_read(MEAS_CFG) & (1 << 7)) tmpval=1; else tmpval=0; //Coefs ready to read
							break;
		case SENSOR_RDY:	if(DPS310_read(MEAS_CFG) & (1 << 6)) tmpval=1; else tmpval=0; //Sensor ready
							break;
		case TMP_RDY:		if(DPS310_read(MEAS_CFG) & (1 << 5)) tmpval=1; else tmpval=0; //Temperatur value ready to read
							break;
		case PRS_RDY:		if(DPS310_read(MEAS_CFG) & (1 << 4)) tmpval=1; else tmpval=0; //Presure value ready to read
							break;
		case PRODUCT_ID:	tmpval = DPS310_read(PRODUCT_ID); // get product ID of sensor
							break;
	}
	return tmpval;
}
void init_ili9341(void)
{
	stdout = & mydata;
	ili9341_init();//initial driver setup to drive ili9341
	ili9341_clear(BLACK);//fill screen with black colour
	_delay_ms(100);
	ili9341_setRotation(3);//rotate screen
	_delay_ms(2);
	ili9341_settextcolour(YELLOW,BLACK);
	ili9341_setcursor(0,0);
	ili9341_settextsize(2);
}

uint32_t DPS310_get_temp(void)
{
	
	long temp_raw;
	float temp_sc;
	float temp_comp;
	
		buffer[0] = DPS310_read(TMP_B2);
		buffer[1] = DPS310_read(TMP_B1);
		buffer[2] = DPS310_read(TMP_B0);
		
		temp_raw=((((long)buffer[0]<<8)|buffer[1])<<8)|buffer[2];
		temp_raw=(temp_raw<<8)>>8;
		
		temp_sc = (float)temp_raw/524288;
		temp_comp=m_C0+m_C1*temp_sc;
		return temp_comp*100; //2505 entspricht 25,5 Grad
}

int main(void)
{
	init_ili9341();
	//display_init();//display initial data
	yy=240;
	xx=0;
	zell=0;
	color=123;
	var_x=0x01;
	
	value=0;
	rdy=0;
	TWIInit();
	
	
	_delay_ms(500);
	
		DPS310_readCoeffs();
	
	
	DPS310_write(PRS_CFG, 0x01);//eight times low power
	
	
	
	DPS310_write(TMP_CFG, 0x80);// 1 measurement


	DPS310_write(CFG_REG, 0x00);

	
	DPS310_write(MEAS_CFG, 0x07);
	

	DPS310_write(0x0E, 0xA5);
	DPS310_write(0x0F, 0x96);
	DPS310_write(0x62, 0x02);
	DPS310_write(0x0E, 0x00);
	DPS310_write(0x0F, 0x00);
	

	while(1)
	{
				
		id = DPS310_read(PRODUCT_ID);
		meas = DPS310_read(MEAS_CFG);
		
		
		
	
		
		ili9341_setcursor(0,0);
		printf("B2 = %03d", buffer[0]);
		ili9341_setcursor(0,20);
		printf("B1 = %03d", buffer[1]);
		
		ili9341_setcursor(0,40);
		printf("B0 = %03d",m_C1);
		
		ili9341_setcursor(0,70);
		printf("ID = %03d", id);
		ili9341_setcursor(0,90);
		printf("comp = %03ld", DPS310_get_temp());
		
		ili9341_setcursor(0,10);
		
		printcoeffs();
	
		
	

	_delay_ms(10);
		
	
	}//end of while

}//end of main
