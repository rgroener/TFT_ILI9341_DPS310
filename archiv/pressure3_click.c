#include <stdint.h>
#include "resources.h"
#include "pressure_3_hal.h"
#include <built_in.h>


/******************************************************************************
* TFT module connections
*******************************************************************************/

char TFT_DataPort at GPIO_PORT_32_39;
sbit TFT_RST at GPIO_PIN42_bit;
sbit TFT_RS at GPIO_PIN40_bit;
sbit TFT_CS at GPIO_PIN41_bit;
sbit TFT_RD at GPIO_PIN46_bit;
sbit TFT_WR at GPIO_PIN47_bit;
sbit TFT_BLED at GPIO_PIN58_bit;


void Auxiliary_Code(void);
void Set_Index(unsigned short index);
void Write_Command(unsigned short cmd);
void Write_Data(unsigned int _data);

void Auxiliary_Code() {
  TFT_WR = 0;
  asm nop;
  TFT_WR = 1;
}

void Set_Index(unsigned short index) {
  TFT_RS = 0;
  asm nop;
  TFT_DataPort = index;
  asm nop; asm nop;
  Auxiliary_Code();
}

void Write_Command(unsigned short cmd) {
  TFT_RS = 1;
  asm nop;
  TFT_DataPort = cmd;
  asm nop; asm nop;
  Auxiliary_Code();
}

void Write_Data(unsigned int _data) {
  TFT_RS = 1;
  asm nop;
  TFT_DataPort = Hi(_data);
  asm nop; asm nop;
  Auxiliary_Code();
  asm nop;
  TFT_DataPort = Lo(_data);
  asm nop; asm nop;
  Auxiliary_Code();
}



char txt[20];
char txt1[20];
// Calculation coefficients as stated in the datasheet
int C0;
int C1;
long C00;
long C10;
int C01;
int C11;
int C20;
int C21;
int C30;


float Pressure;
float Temperature;

static void Display_Init()
{
  TFT_Init_ILI9341_8bit(320, 240);
  TFT_BLED = 1;
  TFT_Fill_Screen(CL_AQUA);
  TFT_Set_Pen(CL_BLACK, 1);
  TFT_Line(20, 220, 300, 220);
  TFT_Line(20,  46, 300,  46);
  TFT_Set_Font(&HandelGothic_BT21x22_Regular, CL_BLACK, FO_HORIZONTAL);
  TFT_Write_Text("Pressure 3  Board  Demo", 42, 14);
  TFT_Set_Font(&Verdana12x13_Regular, CL_BLACK, FO_HORIZONTAL);
  TFT_Write_Text("EasyMx PRO v7 for STM32", 19, 223);
  TFT_Set_Font(&Verdana12x13_Regular, CL_BLACK, FO_HORIZONTAL);
  TFT_Write_Text("www.mikroe.com", 200, 223);

  TFT_Set_Font(TFT_defaultFont, CL_BLACK, FO_HORIZONTAL);
  TFT_Write_Text("mbar",200, 70);
  TFT_Write_Text("C", 200, 100);

}

static void Set_Status (char status)
{
    uint8_t temp_data[2];
    temp_data[0]= 0x08 ;
    temp_data[1]=status;
    Pressure_3_hal_write(0x08, &temp_data[1], 1);
}

long Get_Data (char MSB)
{
    uint8_t temp_data[3];
    long value = 0;
    temp_data[0]=MSB;
    Pressure_3_hal_read(temp_data[0], temp_data, 3);
    value=((((long)temp_data[0]<<8)|temp_data[1])<<8)|temp_data[2];
    value=(value<<8)>>8;
    return value;
}

static void Temperature_Config()
{
    uint8_t temp_data[2];
    temp_data[0]= 0x07 ;
    temp_data[1]=0x80;
    Pressure_3_hal_write(0x07, &temp_data[1], 1);
}

static void Pressure_Config()
{
    uint8_t temp_data[2];
    temp_data[0]= 0x06 ;
    temp_data[1]=0x01;
    Pressure_3_hal_write(0x06, &temp_data[1], 1);
}

void pressure3_calculate_coefficients()
{
       uint8_t temp_read[3];
       temp_read[0]=0x10;

       Pressure_3_hal_read(temp_read[0], temp_read, 2);
       C0=(((int)temp_read[0]<<8)|temp_read[1])>>4;
       C0=C0/2;

       temp_read[0]=0x11;
       Pressure_3_hal_read(temp_read[0], temp_read, 2);
       C1=((((int)temp_read[0]<<8)|temp_read[1])<<4)>>4;


       temp_read[0]=0x13;
       Pressure_3_hal_read(temp_read[0], temp_read, 2);
       C00= ((((long)temp_read[0]<<8)|temp_read[1])<<8)|temp_read[2];
       C00=(C00<<8)>>12;

       temp_read[0]=0x15;
       Pressure_3_hal_read(temp_read[0], temp_read, 2);
       C10=((((long)temp_read[0]<<8)|temp_read[1])<<8)|temp_read[2];
       C10=(C10<<12)>>12;


       temp_read[0]=0x18;
       Pressure_3_hal_read(temp_read[0], temp_read, 2);
       C01=((int)temp_read[0]<<8)|temp_read[1];

       temp_read[0]=0x1A;
       Pressure_3_hal_read(temp_read[0], temp_read, 2);
       C11=((int)temp_read[0]<<8)|temp_read[1];

       temp_read[0]=0x1C;
       Pressure_3_hal_read(temp_read[0], temp_read, 2);
       C20=((int)temp_read[0]<<8)|temp_read[1];

       temp_read[0]=0x1E;
       Pressure_3_hal_read(temp_read[0], temp_read, 2);
       C21=((int)temp_read[0]<<8)|temp_read[1];

       temp_read[0]=0x20;
       Pressure_3_hal_read(temp_read[0], temp_read, 2);
       C30=((int)temp_read[0]<<8)|temp_read[1];
}

void calculate_temperature_pressure()
{
       float Prs;
       long temptemp = 0;
       Temperature=0;
       Pressure_Config();
       Prs=(float)Get_Data( 0x00 );
       Temperature_Config();
       Temperature=(float)Get_Data( 0x03 );
       Temperature/=524288;
       temptemp = Temperature;
       Prs/=1572864;
       Temperature=C0+Temperature*C1;
       Pressure=C00+Prs*(C10+Prs*(C20+(Prs*C30)))+temptemp*C01+temptemp*Prs*(C11+(Prs*C21));
       Pressure/= 100;
}

void main() {

    Display_Init();
    I2CM1_Init(_I2CM_SPEED_MODE_FAST, _I2CM_SWAP_DISABLE);
    delay_ms(300);
    Pressure_3_hal_init(0x76, MODE_I2C);
    Pressure_3_Config ();
    Set_Status(0x07);

    while (1)
    {
       pressure3_calculate_coefficients();
       calculate_temperature_pressure();

       FloatToStr(Pressure, txt);
       FloatToStr(Temperature, txt1);
       TFT_Set_Font(TFT_defaultFont, CL_BLACK, FO_HORIZONTAL);
       TFT_Write_Text("Pressure:", 30, 70);
       TFT_Write_Text(txt, 100, 70);
       TFT_Write_Text("Temperature:", 30, 100);
       TFT_Write_Text(txt1, 120, 100);
       Delay_ms(500);
       TFT_Set_Font(TFT_defaultFont, CL_AQUA, FO_HORIZONTAL);

       TFT_Write_Text("Pressure:", 30, 70);
       TFT_Write_Text(txt, 100, 70);
       TFT_Write_Text("Temperature:", 30, 100);
       TFT_Write_Text(txt1, 120, 100);
    }
}