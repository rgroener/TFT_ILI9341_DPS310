#include <util/delay.h>
#include <math.h>
#include <avr/interrupt.h>
#include "grn_TWI.h"
#include <inttypes.h>




#define DPS310_W 0xEC	//Adresse DPS310
#define DPS310_R 0xED

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
#define Product_ID	0x0D
#define COEF_SRCE	0x28

#define MODE_STBY	0x00
#define MODE_COMMAND_PRES 0x01
#define MODE_COMMAND_TEMP 0x02
#define MODE_COMMAND_PRES_AND_TEMP 0x03
#define MODE_BACKGROUND_PRES 0x05
#define MODE_BACKGROUND_TEMP 0x06
#define MODE_BACKGROUND_PRES_AND_TEMP 0x07

//Kompensationskoefizienten
uint16_t C0=0; 
uint16_t C1=0; 


int16_t make_signed_16(uint8_t high_byte, uint8_t low_byte)
{
	uint16_t tmpvar=0;// roh zahl unsigned aus 2er Komplement
	int16_t ret=0;//return variable signed
	tmpvar=(((uint16_t)high_byte)<<8 | low_byte); //schiftet 8
	
	if(tmpvar>=32768)//Vorzeichenbit gesetzt?
	{
		tmpvar -=32768;//Wertigkeit von Vorzeichenbit abzählen
		ret = 0 -((int16_t)tmpvar);
	}else ret = (int16_t)tmpvar;
	return ret;
}
uint8_t DPS310_read_8(uint8_t reg)
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
uint16_t DPS310_init(void)
{
	
	DPS310_write(0x07, 0x00);//1 Messung / sec 8 mal oversampling
	_delay_ms(100);
	DPS310_write(0x08, 0x07);
	_delay_ms(100);
	DPS310_write(0x08, 0x07);
	_delay_ms(10);
		
	/*
	 * Auslesen und Umrechnung Kalibrationskoeffizienten in Dezimal Werte
	 * */
	 
	 uint8_t ut1=0;
	 uint8_t ut2=0;
	 uint8_t ut3=0;
	 
	 ut1 = DPS310_read_8(0x10);	//Bit 4-11 von C0
	 _delay_ms(100);
	 ut2 = DPS310_read_8(0x11);//Bit 4-7 im Register sind Bit 0-3 in C0 / Bit 0-3 in Register sind Bit 8-11 in C1
	 _delay_ms(100);
	 ut3 = DPS310_read_8(0x12);//Bit 0-7 in C1
	 _delay_ms(100);
	 
	 uint16_t t_16_1=0;
	 
	 t_16_1 = (ut2>>4);		 //4 mal nach rechts da nur die obersten 4 bits aus diesem Register zu C0 gehören
	 C0 = (ut1<<4) | t_16_1; //Zusammensetzten der 12 Bit Zahl 
	//C0=(((int)ut1<<8)|ut2)>>4;
      // C0=C0/2;
	 t_16_1 = 0; 			//Hilfsvariable zurücksetzen
	 t_16_1 = ut2 & 0x0F; 	//nur die tiefsten 4 Bits übernehmen
	 C1 = (t_16_1<< 8) | ut3;	//12 Bits zusammensetzen
	 
	return C0;
		
	//16Bite Werte
	/*
	tmp1=DPS310_read_8(0x18);
	_delay_ms(verz);
	tmp2=DPS310_read_8(0x19);
	_delay_ms(verz);
	C01 = make_signed_16(tmp1, tmp2);
	tmp1=DPS310_read_8(0x1A);
	_delay_ms(verz);
	tmp2=DPS310_read_8(0x1B);
	_delay_ms(verz);
	C11 = make_signed_16(tmp1, tmp2);
	tmp1=DPS310_read_8(0x1C);
	_delay_ms(verz);
	tmp2=DPS310_read_8(0x1D);
	_delay_ms(verz);
	C20 = make_signed_16(tmp1, tmp2);
	tmp1=DPS310_read_8(0x1E);
	_delay_ms(verz);
	tmp2=DPS310_read_8(0x1F);
	_delay_ms(verz);
	C21 = make_signed_16(tmp1, tmp2);
	tmp1=DPS310_read_8(0x20);
	_delay_ms(verz);
	tmp2=DPS310_read_8(0x21);
	_delay_ms(verz);
	C30 = make_signed_16(tmp1, tmp2);
	* */
}
int32_t DPS310_get_raw_temp(void)
{
	uint8_t tmp0=0;
	uint8_t tmp1=0;
	uint8_t tmp2=0;
	uint16_t tmp16=0;//	
	uint32_t tmp32=0;
	int32_t ret=0;
	uint8_t verz=100; //Verzoegerund für Auslesen aus Register


	
		
	tmp2=DPS310_read_8(TMP_B2);	//MSB rohdaten aus Sensor auslesen
	_delay_ms(verz);
	tmp1=DPS310_read_8(TMP_B1);	//mittleres Bit auslesen
	_delay_ms(verz);
	tmp0=DPS310_read_8(TMP_B0);	//LSB auslesen
	_delay_ms(verz);
	
	
	tmp16 = ((uint16_t)tmp2<<8) | tmp1;	//schiebe MSB 8 Schritte links
	tmp32 = tmp16;				//16 Bit Variable in 32 Bit Variable kopieren
	tmp32 = (tmp32<<8) | tmp0;	//LSB hinzufügen
	
/*	if(tmp32 >= 8388608)		//Vorzeichenbit gesetzt
	{
		tmp32 -= 8388608;		//Wertigkeit von Vorzeichenbit abzählen
		ret = 0 -((int32_t)tmp32);
	}else ret = (int32_t)tmp32;
	ret = (int32_t)tmp32;
 */
	ret = tmp32;
  return ret;
}

int32_t calculate_temperature_pressure(void)
{
	float Temperature=0;
       long temptemp = 0;
       Temperature=0;
      
     _delay_ms(100);
	DPS310_write(PRS_CFG, 0x01);
	_delay_ms(100);
	DPS310_write(TMP_CFG, 0x80);//1 Messung / sec 8 mal oversampling
	_delay_ms(100);
	DPS310_write(0x08, 0x07);
	_delay_ms(100);
       
       
       
       
       Temperature=(float)DPS310_get_raw_temp();
       Temperature/=524288;
       temptemp = Temperature;
     return temptemp;
}



uint32_t DPS310_get_raw_pres(void)
{
	uint8_t tmp0=0;
	uint8_t tmp1=0;
	uint8_t tmp2=0;
	uint16_t tmp16=0;
	uint32_t tmp32=0;
	int32_t ret=0;
	uint8_t verz=100; //Verzoegerund für Auslesen aus Register
	
	DPS310_write(0x08, 0x02);//eine temp messung auslösen
	_delay_ms(100);
		
	tmp2=DPS310_read_8(PSR_B2);	//MSB rohdaten aus Sensor auslesen
	_delay_ms(verz);
	tmp1=DPS310_read_8(PSR_B1);	//mittleres Bit auslesen
	_delay_ms(verz);
	tmp0=DPS310_read_8(PSR_B0);	//LSB auslesen
	_delay_ms(verz);
	
	//ret=((((long)tmp2<<8)|tmp1)<<8)|tmp0;
    //ret=(ret<<8)>>8;
    
	//value=((((long)temp_data[0]<<8)|temp_data[1])<<8)|temp_data[2];
	

	tmp16 = (tmp2<<8) | tmp1;	//schiebe MSB 8 Schritte links
	tmp32 = tmp16;				//16 Bit Variable in 32 Bit Variable kopieren
	tmp32 = (tmp32<<8) | tmp0;	//LSB hinzufügen
	
	/*
	if(tmp32 >= 8388608)		//Vorzeichenbit gesetzt
	{
		tmp32 -= 8388608;		//Wertigkeit von Vorzeichenbit abzählen
		ret = 0 -((int32_t)tmp32);
	}else ret = (int32_t)tmp32;
  //  ret=(ret<<8)>>8;
	
	*/
	return ret;
	
}



int32_t DPS310_get_komp_temp(void)
{
	uint8_t tmp0=0;
	uint8_t tmp1=0;
	uint8_t tmp2=0;
	uint16_t tmp16=0;
	uint32_t tmp32=0;
	int32_t ret=0;
	int32_t temp_raw_sc=0;
	int32_t temp_raw=0;
	uint8_t verz=10; //Verzoegerund für Auslesen aus Register
		
	tmp2=DPS310_read_8(TMP_B2);	//MSB rohdaten aus Sensor auslesen
	_delay_ms(verz);
	tmp1=DPS310_read_8(TMP_B1);	//mittleres Bit auslesen
	_delay_ms(verz);
	tmp0=DPS310_read_8(TMP_B0);	//LSB auslesen
	_delay_ms(verz);
	
	tmp16 = (tmp2<<8) | tmp1;	//schiebe MSB 8 Schritte links
	tmp32 = tmp16;				//16 Bit Variable in 32 Bit Variable kopieren
	tmp32 = (tmp32<<8) | tmp0;	//LSB hinzufügen
	
	if(tmp32 >= 8388608)		//Vorzeichenbit gesetzt
	{
		tmp32 -= 8388608;		//Wertigkeit von Vorzeichenbit abzählen
		temp_raw = 0 -((int32_t)tmp32);
	}else ret = (int32_t)tmp32;
	
	temp_raw_sc=temp_raw / 7864320;
	ret = (C0*0.5)+(C1*temp_raw_sc);
    
	return ret;
}
