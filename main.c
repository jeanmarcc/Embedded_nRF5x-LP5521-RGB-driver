/** @example lp5521-rgb-driver
 *
 * @brief Simple example to run a LP5521 RGB Driver from Texas Instruments
 *
 *   
 */
/*
	Customization:

- sdk_config.h : for TWI, in sdk_config.h, TWI_DEFAULT_CONFIG_CLR_BUS_INIT = 0
- sdk_config.h : for TWI, TWI0_USE_EASY_DMA = 0
 
*/
#include <stdbool.h>
#include <stdint.h>
#include <string.h>


#include "nrf_drv_clock.h"
#include "nrf_power.h"
#include "nrf_drv_twi.h"  
#include "nrf_delay.h" 
#include "nrf_gpio.h" 
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"
#include "ctype.h"  

#define RGB_DRIV_I2C_ADDR 0x32
#define I2C_DELAY     10  // mandatory delay in ms for this interface I2C else the target blocks .... CANNOT be set lower than 10ms

#define LED_EN        		16  	// Digital pin 5
#define ARDUINO_SCL_PIN   10  	// SCL signal pin  
#define ARDUINO_SDA_PIN   9    	// SDA signal pin  

void RGB_driver_display_mode(uint8_t COLOR);
void turn_off_RGB_color(void);

/* Indicates if operation on TWI has ended. */
static volatile bool m_xfer_done = false;


/* TWI instance. */
#define TWI_INSTANCE_ID     0
static const nrf_drv_twi_t m_twi = NRF_DRV_TWI_INSTANCE(TWI_INSTANCE_ID);

/**
 * @brief TWI events handler.
 */
void twi_handler(nrf_drv_twi_evt_t const * p_event, void * p_context)
{
    switch (p_event->type)
    {
        case NRF_DRV_TWI_EVT_DONE:
            if (p_event->xfer_desc.type == NRF_DRV_TWI_XFER_RX)
            {
							//data_handler(m_sample);
            }
            m_xfer_done = true;
            break;
        default:
            break;
    }
}

/**
 * @brief TWI initialization. =>  I2C
 */
void twi_init (void)
{
    ret_code_t err_code;

    const nrf_drv_twi_config_t twi_LP55231_config = {
       .scl                = ARDUINO_SCL_PIN,
       .sda                = ARDUINO_SDA_PIN,
       .frequency          = NRF_TWI_FREQ_100K,
       .interrupt_priority = APP_IRQ_PRIORITY_HIGH,
       .clear_bus_init     = false
    };

    err_code = nrf_drv_twi_init(&m_twi, &twi_LP55231_config, twi_handler, NULL);
    APP_ERROR_CHECK(err_code);

    nrf_drv_twi_enable(&m_twi);
}

/**@brief Function initialize the RGB driver
 */
void RGB_driver_init(void)
{

		// Enable LED driver
		nrf_gpio_cfg_output(LED_EN);
		nrf_gpio_pin_write(LED_EN,1); //enable driver
		nrf_delay_ms(1); //1 ms delay 

	 	ret_code_t err_code;
    uint8_t reg[2];
	
		reg[0] = 0x00;
		reg[1] = 0x40;
    err_code = nrf_drv_twi_tx(&m_twi, RGB_DRIV_I2C_ADDR, reg, sizeof(reg), false);
		nrf_delay_ms(I2C_DELAY);	
    while (m_xfer_done == false);

	  reg[0] = 0x08;
		reg[1] = 0x11; //charge pump 1.5x mode and use internal clock:
    err_code = nrf_drv_twi_tx(&m_twi, RGB_DRIV_I2C_ADDR, reg, sizeof(reg), false);
		nrf_delay_ms(I2C_DELAY);	
    while (m_xfer_done == false);	
		
	  reg[0] = 0x05;
		reg[1] = 0xff; //RED current set to MAX value
    err_code = nrf_drv_twi_tx(&m_twi, RGB_DRIV_I2C_ADDR, reg, sizeof(reg), false);
		nrf_delay_ms(I2C_DELAY);	
    while (m_xfer_done == false);

	  reg[0] = 0x06;
		reg[1] = 0xff; //GREEN current set to MAX value
    err_code = nrf_drv_twi_tx(&m_twi, RGB_DRIV_I2C_ADDR, reg, sizeof(reg), false);
		nrf_delay_ms(I2C_DELAY);	
		while (m_xfer_done == false);

	  reg[0] = 0x07;
		reg[1] = 0xff; //Blue current set to MAX value
    err_code = nrf_drv_twi_tx(&m_twi, RGB_DRIV_I2C_ADDR, reg, sizeof(reg), false);
		nrf_delay_ms(I2C_DELAY);	
    while (m_xfer_done == false);		
	
	  //enable the channels
 		reg[0] = 0x01;
		reg[1] = 0x3F;
		err_code = nrf_drv_twi_tx(&m_twi, RGB_DRIV_I2C_ADDR, reg, sizeof(reg), false);
		nrf_delay_ms(I2C_DELAY);	
		while (m_xfer_done == false);	

	  //shut down colors (PWM = 0)
 		reg[0] = 0x02;
		reg[1] = 0x00;
		err_code = nrf_drv_twi_tx(&m_twi, RGB_DRIV_I2C_ADDR, reg, sizeof(reg), false);
		nrf_delay_ms(I2C_DELAY);	
		while (m_xfer_done == false);	
		
 		reg[0] = 0x03;
		reg[1] = 0x00;
		err_code = nrf_drv_twi_tx(&m_twi, RGB_DRIV_I2C_ADDR, reg, sizeof(reg), false);
		nrf_delay_ms(I2C_DELAY);		
		while (m_xfer_done == false);			
		
 		reg[0] = 0x04;
		reg[1] = 0x00;
		err_code = nrf_drv_twi_tx(&m_twi, RGB_DRIV_I2C_ADDR, reg, sizeof(reg), false);
		nrf_delay_ms(I2C_DELAY);		
		while (m_xfer_done == false);	
	
		nrf_gpio_pin_write(LED_EN,0); //disable RGB driver to save power

}

/**@brief Function to light up the RGB LED
*
* @details Lights up 1 color of the RGB 
*
* @param[in]   COLOR   			The Color to light up
* '0' : no color (turn off)
* '1' : red
* '2' : green
* '3' : blue
* '4' : yellow
* '5' : orange
* '6' : pink
* '7' : purple
* '8' : white bright
* '9' : white soft 
 */
void RGB_driver_display_mode(uint8_t COLOR)
{

    uint8_t reg[2], pwm_r, pwm_g, pwm_b;

		if 			( COLOR == 0) { pwm_r = 0		; pwm_g =0 		; pwm_b = 0 	; }	
		else if ( COLOR == 1) { pwm_r = 255	; pwm_g =0 		; pwm_b = 0 	; }
		else if (	COLOR	== 2)	{ pwm_r = 0	 	; pwm_g =255 	; pwm_b = 0 	; }
		else if (	COLOR	== 3)	{ pwm_r = 0	 	; pwm_g =0 		; pwm_b = 255 ; }
		else if (	COLOR	== 4)	{ pwm_r = 100 ; pwm_g =50 	; pwm_b = 0 	; }
		else if (	COLOR	== 5)	{ pwm_r = 220 ; pwm_g =20		; pwm_b = 0		; }
		else if (	COLOR	== 6)	{ pwm_r = 220	; pwm_g =0 		; pwm_b = 50 	; }
		else if (	COLOR	== 7)	{ pwm_r = 150	; pwm_g =0 		; pwm_b = 200 ; }
		else if (	COLOR	== 8)	{ pwm_r = 220	; pwm_g =180 	; pwm_b = 180 ; }		
		else if (	COLOR	== 9)	{ pwm_r = 50	; pwm_g =30 	; pwm_b = 30 	; }		
			
		if (pwm_r) 
		{
			reg[0] = 0x02; //Red channel	
			reg[1] = pwm_r; //PWM
			nrf_drv_twi_tx(&m_twi, RGB_DRIV_I2C_ADDR, reg, sizeof(reg), false);
			nrf_delay_ms(I2C_DELAY);	
			while (m_xfer_done == false);
		}
		if (pwm_g) 
		{
			reg[0] = 0x03; //Green channel	
			reg[1] = pwm_g; //PWM
			nrf_drv_twi_tx(&m_twi, RGB_DRIV_I2C_ADDR, reg, sizeof(reg), false);
			nrf_delay_ms(I2C_DELAY);	
			while (m_xfer_done == false);
		}
		if (pwm_b) 
		{
			reg[0] = 0x04; //Blue channel	
			reg[1] = pwm_b; //PWM
			nrf_drv_twi_tx(&m_twi, RGB_DRIV_I2C_ADDR, reg, sizeof(reg), false);
			nrf_delay_ms(I2C_DELAY);	
			while (m_xfer_done == false);
		}
		
}

/**@brief Function for application main entry.
 */
int main(void)
{
    twi_init(); //Initialization of the I2C bus driver
		RGB_driver_init();

		RGB_driver_display_mode(3); //example to display Blue color 
	
    // Enter main loop.
    for (;;)
    {
    }
}

