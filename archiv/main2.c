
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
#include "DPS310.h"


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
int32_t a;

//compensation coefficients
		int32_t m_c0Half;
		int32_t m_c1;
		int32_t m_c00;
		int32_t m_c10;
		int32_t m_c01;
		int32_t m_c11;
		int32_t m_c20;
		int32_t m_c21;
		int32_t m_c30;
  

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
	
	//compose coefficients from buffer content
	m_c0Half =    ((uint32_t)buffer[0] << 4)
				| (((uint32_t)buffer[1] >> 4) & 0x0F);
	//this construction recognizes non-32-bit negative numbers
	//and converts them to 32-bit negative numbers with 2's complement
	if(m_c0Half & ((uint32_t)1 << 11))
	{
		m_c0Half -= (uint32_t)1 << 12;
	}
	//c0 is only used as c0*0.5, so c0_half is calculated immediately
	m_c0Half = m_c0Half / 2U;

	//now do the same thing for all other coefficients
	m_c1 = (((uint32_t)buffer[1] & 0x0F) << 8) | (uint32_t)buffer[2];
	if(m_c1 & ((uint32_t)1 << 11))
	{
		m_c1 -= (uint32_t)1 << 12;
	}

	m_c00 =   ((uint32_t)buffer[3] << 12)
			| ((uint32_t)buffer[4] << 4)
			| (((uint32_t)buffer[5] >> 4) & 0x0F);
	if(m_c00 & ((uint32_t)1 << 19))
	{
		m_c00 -= (uint32_t)1 << 20;
	}

	m_c10 =   (((uint32_t)buffer[5] & 0x0F) << 16)
			| ((uint32_t)buffer[6] << 8)
			| (uint32_t)buffer[7];
	if(m_c10 & ((uint32_t)1<<19))
	{
		m_c10 -= (uint32_t)1 << 20;
	}

	m_c01 =   ((uint32_t)buffer[8] << 8)
			| (uint32_t)buffer[9];
	if(m_c01 & ((uint32_t)1 << 15))
	{
		m_c01 -= (uint32_t)1 << 16;
	}

	m_c11 =   ((uint32_t)buffer[10] << 8)
			| (uint32_t)buffer[11];
	if(m_c11 & ((uint32_t)1 << 15))
	{
		m_c11 -= (uint32_t)1 << 16;
	}

	m_c20 =   ((uint32_t)buffer[12] << 8)
			| (uint32_t)buffer[13];
	if(m_c20 & ((uint32_t)1 << 15))
	{
		m_c20 -= (uint32_t)1 << 16;
	}

	m_c21 =   ((uint32_t)buffer[14] << 8)
			| (uint32_t)buffer[15];
	if(m_c21 & ((uint32_t)1 << 15))
	{
		m_c21 -= (uint32_t)1 << 16;
	}

	m_c30 =   ((uint32_t)buffer[16] << 8)
			| (uint32_t)buffer[17];
	if(m_c30 & ((uint32_t)1 << 15))
	{
		m_c30 -= (uint32_t)1 << 16;
	}

	return 1;
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

result=0;
	ili9341_settextcolour(RED,BLACK);
	
	TWIInit();
	
		//Call begin to initialize Dps310PressureSensor
	  //The parameter 0x76 is the bus address. The default address is 0x77 and does not need to be given.
	  //Dps310PressureSensor.begin(Wire, 0x76);
	  //Use the commented line below instead of the one above to use the default I2C address.
	  //if you are using the Pressure 3 click Board, you need 0x76
	 // Dps310PressureSensor.begin(Wire);
	// dps310_init(0x76);
	
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
		a=555555;
		ili9341_setcursor(0,0);
		printf("0x00 = %ld", m_c0Half);
		ili9341_setcursor(0,30);
		printf("0x00 = %ld", m_c30);
		ili9341_setcursor(0,60);
		printf("0x00 = %ld", m_c1);
		ili9341_setcursor(0,90);
		printf("0x00 = %ld", m_c10);
		ili9341_setcursor(0,120);
		printf("0x00 = %ld", m_c00);
		ili9341_setcursor(0,150);
		printf("0x00 = %ld", m_c01);
		ili9341_setcursor(0,180);
		printf("0x00 = %ld", m_c20);
	while(1)
	{
		
		
		
		_delay_ms(500);
		/*
		
		TWIStart();
		if(TWIGetStatus() != 0x08)error=1; //0x08 = Start condition
		TWIWrite(var_x);
		if(TWIGetStatus() == 0x18)
		{
			xpos +=40;
			
		}
		TWIStop();
	
		
		var_x++;
		_delay_ms(500);
*/

	}//end of while

}//end of main
