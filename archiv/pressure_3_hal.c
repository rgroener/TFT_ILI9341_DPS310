#include "pressure_3_hal.h"
#include <stddef.h>
#define STM32


#ifdef USE_HAL_DRIVER
#define STM32
#if defined( STM32F1 )
#include "stm32f1xx_hal.h"
#elif defined( STM32F2 )
#include "stm32f2xx_hal.h"
#elif defined( STM32F3 )
#include "stm32f3xx_hal.h"
#elif defined( STM32F4 )
#include "stm32f4xx_hal.h"
#elif defined( STM32F7 )
#include "stm32f7xx_hal.h"
#endif
#include "cmsis_os.h"
#endif


/******************************************************************************
* Module Preprocessor Constants
*******************************************************************************/
#define READ_BIT                1
#define WRITE_BIT               0

/******************************************************************************
* Module Preprocessor Macros
*******************************************************************************/

/******************************************************************************
* Module Typedefs
*******************************************************************************/

/******************************************************************************
* Module Variable Definitions
*******************************************************************************/
static uint8_t _i2c_hw_address;
static pressure_hal_mode_t _mode;

#if defined( __MIKROC_PRO_FOR_PIC32__ ) || \
defined( __MIKROC_PRO_FOR_DSPIC__ )
static uint8_t NACK_BIT   = 0x01;
static uint8_t ACK_BIT    = 0x00;
#elif defined( __MIKROC_PRO_FOR_PIC__ ) || \
defined( __MIKROC_PRO_FOR_AVR__ )
static uint8_t NACK_BIT   = 0x00;
static uint8_t ACK_BIT    = 0x01;
#endif

#if defined( __MIKROC_PRO_FOR_ARM__ )
#if defined( STM32 )
static unsigned int ( *start_i2c_p )            ( void );
static unsigned int ( *write_i2c_p )            ( unsigned char slave_address,
        unsigned char *buffer,
        unsigned long count,
        unsigned long end_mode );
static void         ( *read_i2c_p )             ( unsigned char slave_address,
        unsigned char *buffer,
        unsigned long count,
        unsigned long end_mode );
static unsigned int ( *spi_read_p )( unsigned int buffer );
static void ( *spi_write_p )( unsigned int data_out );

#elif defined( TI )
static void         ( *enable_i2c_p )           ( void );
static void         ( *disable_i2c_p )          ( void );
static void         ( *set_slave_address_i2c_p )( unsigned char slave_address,
        unsigned char dir );
static void         ( *write_i2c_p )            ( unsigned char data_out,
        unsigned char mode );
static void         ( *read_i2c_p )             ( unsigned char *data,
        unsigned char mode );
static unsigned int ( *spi_read_p )( unsigned int buffer );
static void ( *spi_write_p )( unsigned int data_out );
#endif

#elif  defined( __MIKROC_PRO_FOR_AVR__ )
static unsigned char( *busy_i2c_p )             ( void );
static unsigned char( *status_i2c_p )           ( void );
static unsigned char( *start_i2c_p )            ( void );
static void         ( *stop_i2c_p )             ( void );
static void         ( *close_i2c_p )            ( void );
static void         ( *write_i2c_p )            ( unsigned char data_out );
static unsigned char( *read_i2c_p )             ( unsigned char ack );
static unsigned char( *spi_read_p )( unsigned char data_in );
static void( *spi_write_p )( unsigned char data_out );

#elif  defined( __MIKROC_PRO_FOR_PIC__ )
static unsigned char( *is_idle_i2c_p )          ( void );
static unsigned char( *start_i2c_p )            ( void );
static void         ( *stop_i2c_p )             ( void );
static void         ( *restart_i2c_p )          ( void );
static unsigned char( *write_i2c_p )            ( unsigned char data_out );
static unsigned char( *read_i2c_p )             ( unsigned char ack );
static unsigned char( *spi_read_p )( unsigned char _data );
static void ( *spi_write_p )( unsigned char data_out );

#elif defined( __MIKROC_PRO_FOR_PIC32__ )
static unsigned int ( *is_idle_i2c_p )          ( void );
static unsigned int ( *start_i2c_p )            ( void );
static void         ( *stop_i2c_p )             ( void );
static unsigned int ( *restart_i2c_p )          ( void );
static unsigned int ( *write_i2c_p )            ( unsigned char data_out );
static unsigned char( *read_i2c_p )             ( unsigned int ack );
static unsigned long( *spi_read_p )( unsigned long buffer );
static void ( *spi_write_p )( unsigned long data_out );

#elif defined( __MIKROC_PRO_FOR_DSPIC__ )
static unsigned int ( *is_idle_i2c_p )          ( void );
static unsigned int ( *start_i2c_p )            ( void );
static void         ( *stop_i2c_p )             ( void );
static void         ( *restart_i2c_p )          ( void );
static unsigned int ( *write_i2c_p )            ( unsigned char data_out );
static unsigned char( *read_i2c_p )             ( unsigned int ack );
static unsigned int( *spi_read_p )( unsigned int buffer );
static void ( *spi_write_p )( unsigned int data_out );

#elif defined( __MIKROC_PRO_FOR_8051__ )
static unsigned char( *status_i2c_p )           ( void );
static unsigned char( *start_i2c_p )            ( void );
static void         ( *stop_i2c_p )             ( void );
static void         ( *close_i2c_p )            ( void );
static void         ( *write_i2c_p )            ( unsigned char data_out );
static unsigned char( *read_i2c_p )             ( unsigned char ack );

#elif defined( __MIKROC_PRO_FOR_FT90x__ )
static void         ( *soft_reset_i2c_p )       ( void );
static void         ( *set_slave_address_i2c_p )( unsigned char slave_address );
static unsigned char( *write_i2c_p )            ( unsigned char data_out );
static unsigned char( *read_i2c_p )             ( unsigned char *data_in );
static unsigned char( *write_bytes_i2c_p )      ( unsigned char *buffer,
        unsigned int count );
static unsigned char( *read_bytes_i2c_p )       ( unsigned char *buffer,
        unsigned int count );
static unsigned char( *write_10bit_i2c_p )      ( unsigned char data_out,
        unsigned int address_10bit );
static unsigned char( *read_10bit_i2c_p )       ( unsigned char *data_in,
        unsigned int address_10bit );
static unsigned char( *spi_read_p )( unsigned char dummy );
static void( *spi_write_p )( unsigned char dataOut );
#endif

#if defined SPI
extern sfr sbit PRESSURE_3_CS_PIN;
#endif

/******************************************************************************
* Function Prototypes
*******************************************************************************/
void cs_low( );
void cs_high(  );

/******************************************************************************
* Function Definitions
*******************************************************************************/
void cs_low()
{
#if defined SPI
    PRESSURE_3_CS_PIN = 0;
#endif
}

void cs_high()
{
#if defined SPI
    PRESSURE_3_CS_PIN = 1;
#endif
}


int pressure_3_hal_init( uint8_t address_id, pressure_hal_mode_t mode )
{
    _mode = mode;

    if( _mode == MODE_I2C )
    {
        #if defined( __MIKROC_PRO_FOR_ARM__ )
        #if defined( STM32 )

        start_i2c_p                 = I2C_Start_Ptr;
        write_i2c_p                 = I2C_Write_Ptr;
        read_i2c_p                  = I2C_Read_Ptr;

        #elif defined( LM ) || defined( TM )

        enable_i2c_p                = I2C_Enable_Ptr;
        disable_i2c_p               = I2C_Disable_Ptr;
        set_slave_address_i2c_p     = I2C_Master_Slave_Addr_Set_Ptr;
        write_i2c_p                 = I2C_Write_Ptr;
        read_i2c_p                  = I2C_Read_Ptr;

        #endif

        #elif defined( __MIKROC_PRO_FOR_AVR__ )
        #if defined( ATMEGA )

        busy_i2c_p                  = TWI_Busy;
        status_i2c_p                = TWI_Status;
        close_i2c_p                 = TWI_Close;
        start_i2c_p                 = TWI_Start;
        stop_i2c_p                  = TWI_Stop;
        write_i2c_p                 = TWI_Write;
        read_i2c_p                  = TWI_Read;

        #elif defined( ATXMEGA )

        busy_i2c_p                  = TWIC_Busy;
        status_i2c_p                = TWIC_Status;
        close_i2c_p                 = TWIC_Close;
        start_i2c_p                 = TWIC_Start;
        stop_i2c_p                  = TWIC_Stop;
        write_i2c_p                 = TWIC_Write;
        read_i2c_p                  = TWIC_Read;

        #endif

        #elif defined( __MIKROC_PRO_FOR_PIC__ )

        is_idle_i2c_p               = I2C1_Is_Idle;
        start_i2c_p                 = I2C1_Start;
        stop_i2c_p                  = I2C1_Stop;
        restart_i2c_p               = I2C1_Repeated_Start;
        write_i2c_p                 = I2C1_Wr;
        read_i2c_p                  = I2C1_Rd;

        #elif defined( __MIKROC_PRO_FOR_PIC32__ )

        is_idle_i2c_p               = I2C_Is_Idle_Ptr;
        start_i2c_p                 = I2C_Start_Ptr;
        stop_i2c_p                  = I2C_Stop_Ptr;
        restart_i2c_p               = I2C_Restart_Ptr;
        write_i2c_p                 = I2C_Write_Ptr;
        read_i2c_p                  = I2C_Read_Ptr;

        #elif defined( __MIKROC_PRO_FOR_DSPIC__ )

        is_idle_i2c_p               = I2C2_Is_Idle;
        start_i2c_p                 = I2C2_Start;
        stop_i2c_p                  = I2C2_Stop;
        restart_i2c_p               = I2C2_Restart;
        write_i2c_p                 = I2C2_Write;
        read_i2c_p                  = I2C2_Read;

        #elif defined( __MIKROC_PRO_FOR_8051__ )

        status_i2c_p                = TWI_Status;
        close_i2c_p                 = TWI_Close;
        start_i2c_p                 = TWI_Start;
        stop_i2c_p                  = TWI_Stop;
        write_i2c_p                 = TWI_Write;
        read_i2c_p                  = TWI_Read;

        #elif defined( __MIKROC_PRO_FOR_FT90x__ )

        soft_reset_i2c_p            = I2CM_Soft_Reset_Ptr;
        set_slave_address_i2c_p     = I2CM_Set_Slave_Address_Ptr;
        write_i2c_p                 = I2CM_Write_Ptr;
        read_i2c_p                  = I2CM_Read_Ptr;
        write_bytes_i2c_p           = I2CM_Write_Bytes_Ptr;
        read_bytes_i2c_p            = I2CM_Read_Bytes_Ptr;
        write_10bit_i2c_p           = I2CM_Write_10Bit_Ptr;
        read_10bit_i2c_p            = I2CM_Read_10Bit_Ptr;
        #endif

        #if defined( __MIKROC_PRO_FOR_ARM__ )   ||  \
        defined( __MIKROC_PRO_FOR_FT90x__ ) //|| \
        //defined( STM32 )

        _i2c_hw_address             = address_id;
        #else

        _i2c_hw_address             = ( address_id << 1 );
        #endif
        
        
        
        
        

    }
    else if( _mode == MODE_SPI )
    {
        #if defined( __MIKROC_PRO_FOR_ARM__ )   || \
        defined( __MIKROC_PRO_FOR_AVR__ )   || \
        defined( __MIKROC_PRO_FOR_PIC__ )   || \
        defined( __MIKROC_PRO_FOR_PIC32__ ) || \
        defined( __MIKROC_PRO_FOR_DSPIC__ ) || \
        defined( __MIKROC_PRO_FOR_8051__ )

        spi_read_p = SPI_Rd_Ptr;
        spi_write_p = SPI_Wr_Ptr;
        #elif defined( __MIKROC_PRO_FOR_FT90x__ )

        spi_read_p = SPIM_Rd_Ptr;
        spi_write_p = SPIM_Wr_Ptr;

        if ( spi_read_p == NULL || spi_write_p == NULL )
            ;
        return -1;
        #endif
        cs_high();
    }

    return 0;
}

void pressure_3_hal_write( uint8_t command,
                           uint8_t *buffer,
                           uint16_t count )
{
    uint16_t i                     = 1;
    uint8_t  temp[ MAX_BUFF_SIZE ] = { 0 };
    uint8_t  *temp_ptr = &temp[0];

    temp[0] = command;

    while ( count-- )
        temp[ i++ ] = *( buffer++ );

    switch(_mode)
    {
        case MODE_I2C:
            #if defined(__MIKROC_PRO_FOR_ARM__)
            #if defined( STM32 )

            start_i2c_p();
            write_i2c_p( _i2c_hw_address, temp_ptr, i, END_MODE_STOP );

            #elif defined( LM ) || defined( TM )

            set_slave_address_i2c_p( _i2c_hw_address, _I2C_DIR_MASTER_TRANSMIT );

            if( i == 2 )
            {
                write_i2c_p( *( temp_ptr++ ), _I2C_MASTER_MODE_BURST_SEND_START );
                write_i2c_p( *temp_ptr, _I2C_MASTER_MODE_BURST_SEND_STOP );

            }
            else
            {

                write_i2c_p( *( temp_ptr++ ), _I2C_MASTER_MODE_BURST_SEND_START );

                while( i-- > 1 )
                    write_i2c_p( *( temp_ptr++ ), _I2C_MASTER_MODE_BURST_SEND_CONT );

                write_i2c_p( *temp_ptr, _I2C_MASTER_MODE_BURST_SEND_FINISH );
            }
            #endif

            #elif defined(__MIKROC_PRO_FOR_FT90x__)
            set_slave_address_i2c_p( _i2c_hw_address );
            write_bytes_i2c_p( temp_ptr, i );

            #elif defined(__MIKROC_PRO_FOR_AVR__)   || \
            defined(__MIKROC_PRO_FOR_8051__)  || \
            defined(__MIKROC_PRO_FOR_DSPIC__) || \
            defined(__MIKROC_PRO_FOR_PIC32__) || \
            defined(__MIKROC_PRO_FOR_PIC__)

            start_i2c_p();
            write_i2c_p( _i2c_hw_address | WRITE_BIT );

            while( i-- )
                write_i2c_p( *( temp_ptr++ ) );

            stop_i2c_p();

                        #elif defined( STM32 )
                    HAL_I2C_Master_Transmit( _PRESSURE_3_I2C, (uint16_t)_i2c_hw_address, temp_ptr, i, 0xFFFF );
            #endif

            break;
        case MODE_SPI:
            cs_low();
                #if defined( __MIKROC_PRO_FOR_ARM__ )   || \
            defined( __MIKROC_PRO_FOR_AVR__ )   || \
            defined( __MIKROC_PRO_FOR_PIC__ )   || \
            defined( __MIKROC_PRO_FOR_PIC32__ ) || \
            defined( __MIKROC_PRO_FOR_DSPIC__ ) || \
            defined( __MIKROC_PRO_FOR_8051__ )  || \
            defined( __MIKROC_PRO_FOR_FT90x__ )
            while( i-- )
                    spi_write_p( *temp_ptr++ );
            #elif defined( STM32 )
                    HAL_SPI_Transmit( _PRESSURE_3_SPI, temp_ptr, i, 0xFFF );
            #endif
            cs_high();
            break;
    }

}


void Pressure_3_Config ()
{
    uint8_t temp_data[2];
    temp_data[0]= 0x0E ;
    temp_data[1]=0xA5;
    Pressure_3_hal_write(0x0E, &temp_data[1], 1);
    temp_data[0]= 0x0F ;
    temp_data[1]=0x96;
    Pressure_3_hal_write(0x0F, &temp_data[1], 1);
    temp_data[0]= 0x62 ;
    temp_data[1]=0x02;
    Pressure_3_hal_write(0x62, &temp_data[1], 1);
    temp_data[0]= 0x0E ;
    temp_data[1]=0x00;
    Pressure_3_hal_write(0x0E, &temp_data[1], 1);
    temp_data[0]= 0x0F ;
    temp_data[1]=0x00;
    Pressure_3_hal_write(0x0F, &temp_data[1], 1);
}

void pressure_3_hal_read( uint8_t command,
                          uint8_t *buffer,
                          uint16_t count )
{
    uint8_t* buff_ptr = buffer;

    switch( _mode )
    {
        case MODE_I2C:
            #if defined(__MIKROC_PRO_FOR_ARM__)
            #if defined( STM32 )

            start_i2c_p();
            write_i2c_p( _i2c_hw_address, &command, 1, END_MODE_RESTART );
            read_i2c_p( _i2c_hw_address, buff_ptr, count, END_MODE_STOP );

            #elif defined( LM ) || defined( TM )

            set_slave_address_i2c_p( _i2c_hw_address, _I2C_DIR_MASTER_TRANSMIT );
            write_i2c_p( command, _I2C_MASTER_MODE_SINGLE_SEND );
            set_slave_address_i2c_p( _i2c_hw_address, _I2C_DIR_MASTER_RECEIVE );

            if( count == 1 )
                read_i2c_p( buff_ptr, _I2C_MASTER_MODE_BURST_SINGLE_RECEIVE );
            else
            {
                read_i2c_p( buff_ptr++ , _I2C_MASTER_MODE_BURST_RECEIVE_START ) )
                count--;

                while( count-- > 1 )
                        read_i2c_p( buff_ptr++ , _I2C_MASTER_MODE_BURST_SEND_CONT );

                read_i2c_p( buff_ptr, _I2C_MASTER_MODE_BURST_SEND_FINISH );
            }
        #endif

        #elif defined( __MIKROC_PRO_FOR_FT90x__ )
            set_slave_address_i2c_p( _i2c_hw_address );
            write_bytes_i2c_p( &command, 1 );
            read_bytes_i2c_p( buff_ptr, count );

            #elif defined( __MIKROC_PRO_FOR_AVR__ )    || \
            defined( __MIKROC_PRO_FOR_PIC32__ )  || \
            defined( __MIKROC_PRO_FOR_8051__ )   || \
            defined( __MIKROC_PRO_FOR_PIC__ )    || \
            defined( __MIKROC_PRO_FOR_DSPIC__ )

            start_i2c_p();
            write_i2c_p( _i2c_hw_address | WRITE_BIT );

            write_i2c_p( command );

            stop_i2c_p();
            start_i2c_p();
            write_i2c_p( _i2c_hw_address | READ_BIT );

            while( count-- > 1 )
            {
                *( buff_ptr++ ) = read_i2c_p( ACK_BIT );
            }

            *buff_ptr = read_i2c_p( NACK_BIT );

            stop_i2c_p();

                    #elif defined( STM32 )
            HAL_I2C_Master_Transmit( _PRESSURE_3_I2C, (uint16_t)_i2c_hw_address, &command, 1, 0xFFFF );
            HAL_I2C_Master_Receive( _PRESSURE_3_I2C, (uint16_t)_i2c_hw_address, buff_ptr, count, 0xFFFF );
            #endif

            break;
        case MODE_SPI:
            cs_low();
                #if defined( __MIKROC_PRO_FOR_ARM__ )   || \
            defined( __MIKROC_PRO_FOR_AVR__ )   || \
            defined( __MIKROC_PRO_FOR_PIC__ )   || \
            defined( __MIKROC_PRO_FOR_PIC32__ ) || \
            defined( __MIKROC_PRO_FOR_DSPIC__ ) || \
            defined( __MIKROC_PRO_FOR_8051__ )  || \
            defined( __MIKROC_PRO_FOR_FT90x__ )
            while( count-- )
                    *buff_ptr++ = spi_read_p(0x00);
            #elif defined( STM32 )
            HAL_SPI_Receive( _PRESSURE_3_SPI, buff_ptr, count, 100 );
            #endif
            cs_high();
            break;
    }

}
/*************** END OF FUNCTIONS *********************************************/