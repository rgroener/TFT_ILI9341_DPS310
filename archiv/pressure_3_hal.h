#ifndef PRESSURE_3_HAL_H_
#define PRESSURE_3_HAL_H_

/******************************************************************************
* Includes
*******************************************************************************/
#include <stdint.h>

/******************************************************************************
* Preprocessor Constants
*******************************************************************************/
/*
 * ARM MCUs */
//#define STM32
//#define LM
//#define TM

/*
 * AVR MCUs */
//#define ATMEGA
//#define ATXMEGA
/******************************************************************************
* Configuration Constants
*******************************************************************************/
/**
 * Register address size */
#define COMMAND_SIZE            1
/**
 * Maximum buffer size */
#define MAX_BUFF_SIZE           18
/******************************************************************************
* Macros
*******************************************************************************/

/******************************************************************************
* Typedefs
*******************************************************************************/
typedef enum
{
    MODE_I2C,
    MODE_SPI
} pressure_hal_mode_t;

/******************************************************************************
* Variables
*******************************************************************************/

/******************************************************************************
* Function Prototypes
*******************************************************************************/
#ifdef __cplusplus
extern "C"
{
    #endif

    /**
     * @brief <h3>HAL Initialization</h3>
     *
     * @par
     * Initialization of HAL layer used to initialize I2C bus and pins needed
     * for proper usage of click board.
     *
     * @param[in] address_id - hardware address
     */
    int pressure_3_hal_init( uint8_t address_id, pressure_hal_mode_t mode );

    /**
     * @brief <h3>HAL Write</h3>
     *
     * @par
     * Generic write function adapted for CLICKNAME click.
     *
     * @param[in] command - register address
     * @param[in] buffer - data buffer
     * @param[in] count - data size in bytes
     */
    void pressure_3_hal_write( uint8_t command,
                               uint8_t *buffer,
                               uint16_t count );

    /**
     * @brief <h3>Pressure 3 Config</h3>
     *
     * @par
     * Required for correct temperature reading.
     */
    void Pressure_3_Config ();
    /**
     * @brief <h3>HAL Read</h3>
     *
     * @par
     * Generic read function adapted for CLICKNAME click.
     *
     * @param[in] command - register address
     * @param[out] buffer - data buffer
     * @param[in] count - data size in bytes
     */
    void pressure_3_hal_read( uint8_t command,
                              uint8_t *buffer,
                              uint16_t count );

    #ifdef __cplusplus
} // extern "C"
#endif

#endif /* CLICKNAME_HAL_H_ */

/*** End of File **************************************************************/