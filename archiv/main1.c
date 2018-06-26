
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
uint8_t result,  xpos, error, x, coeff_start, a;
uint16_t xx, yy, zell, COLOR, var_x,color;
uint16_t buffer[19];//coeffizienten


  

// String für Zahlenausgabe
char string[30] = "";


int16_t DPS310_readCoeffs(void)
{
	uint8_t temp1, temp2;
	temp1=DPS310_read_8(0x00);
	
	
	
	
	return 0;
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

	
		//	
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

		ili9341_setcursor(0,0);
		ili9341_settextsize(3);
		
		
		for(x=0;x<8;x++)
		{
			ili9341_setcursor(0,a);
			printf("0x00 = %2x", buffer[x]);
			_delay_ms(10);
			a=a+30;
		}


/*
		printf("0x00 = %2x", buffer[1]);
		ili9341_setcursor(0,30);
		printf("0x00 = %2x", buffer[2]);
		ili9341_setcursor(0,60);
		printf("0x00 = %2x", buffer[3]);
		ili9341_setcursor(0,90);
		printf("0x00 = %2x", buffer[4]);
		ili9341_setcursor(0,120);
		printf("0x00 = %2x", buffer[5]);
		ili9341_setcursor(0,150);
		printf("0x00 = %2x", buffer[6]);
		ili9341_setcursor(0,180);
		printf("0x00 = %2x", a);
		*/
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
