//##########################################################################
//#
//#		FlashFlipDisplay
//#
//#-------------------------------------------------------------------------
//#
//#		MIT License
//#
//#		Copyright (c) 2023	Michael Pfeil
//#							Am Kuckhof 8
//#							D - 52146 Würselen
//#							GERMANY
//#
//#-------------------------------------------------------------------------
//#
//#	This example shows how to flash (invert) the display
//#	(e.g. to display an alarm)
//# and it shows how to "flip" the display (turn text by 180 degree).
//#
//#-------------------------------------------------------------------------
//#
//#	Dieses Beispiel zeigt, wie das Display blinken kann
//#	(z.B.: für die Ausgabe eines Alarms)
//#	und wie die Ausgabe auf dem Display um 180 Grad gedreht wird (flip).
//#
//##########################################################################


//==========================================================================
//
//		I N C L U D E S
//
//==========================================================================

#include <stdint.h>
#include <SimpleOledLib.h>


//==========================================================================
//
//		D E F I N I T I O N S
//
//==========================================================================

#define I2C_BUS_0_SDA_PIN		13
#define I2C_BUS_0_SCL_PIN		15
#define I2C_BUS_FREQUENZ_HZ		400000


//==========================================================================
//
//		G L O B A L   V A R I A B L E S
//
//==========================================================================

const char g_strFlashFlipDemo[]	= "Flash-Flip-Demo";
const char g_strBlank[]			= "               ";
const char g_strAttention[]		= " ! ATTENTION ! ";
const char g_strFlashDisplay[]	= " Flash display ";

oled_display_handle_t	g_Display;

char g_buffer[ 20 ];


//**************************************************************************
//	i2c_bus_init
//--------------------------------------------------------------------------
//	description
//
static esp_err_t i2c_bus_init( i2c_port_t port )
{
	i2c_config_t	stMasterConfig =
	{
		.mode				=	I2C_MODE_MASTER,
		.sda_io_num			=	I2C_BUS_0_SDA_PIN,
		.sda_pullup_en		=	GPIO_PULLUP_ENABLE,
		.scl_io_num			=	I2C_BUS_0_SCL_PIN,
		.scl_pullup_en		=	GPIO_PULLUP_ENABLE,
		.master.clk_speed	=	I2C_BUS_FREQUENZ_HZ,
		.clk_flags			=	0,
	};

	i2c_param_config( port, &stMasterConfig );

	return( i2c_driver_install( port, stMasterConfig.mode, 0, 0, 0 ) );
}


//**************************************************************************
//	app_main
//--------------------------------------------------------------------------
//	description
//
void app_main()
{
	uint8_t	retValue;
	bool	bInvertDisplay	= false;


	vTaskDelay( 7000 / portTICK_PERIOD_MS );

	i2c_bus_init( I2C_NUM_0 );

	printf( "SimpleOled Demo: " );
	printf( g_strFlashFlipDemo );
	printf( "\n  setup chip type ssd1306\n" );

	retValue = oled_display_init( &g_Display, I2C_NUM_0, CHIP_TYPE_SSD1306, DISPLAY_ADDRESS_DEFAULT );

	if( 1 == retValue )
	{
		printf( "    ERROR: invalid address !!\n" );
	}
	else if( 2 == retValue )
	{
		printf( "    ERROR: display not found !!\n" );
	}
	else
	{
		while( 1 )
		{
			//----------------------------------------------------------------------
			//	print Info
			//
			oled_display_set_cursor( &g_Display, 2, 0 );
			oled_display_print( &g_Display, g_strFlashFlipDemo );

			vTaskDelay( 2000 / portTICK_PERIOD_MS );

			//----------------------------------------------------------------------
			//	print message and ...
			//
			oled_display_clear( &g_Display );
			oled_display_set_inverse_font( &g_Display, true );
			oled_display_set_cursor( &g_Display, 1, 0 );
			oled_display_println( &g_Display, g_strBlank );
			oled_display_println( &g_Display, g_strAttention );
			oled_display_println( &g_Display, g_strBlank );
			oled_display_println( &g_Display, g_strFlashDisplay );
			oled_display_println( &g_Display, g_strBlank );
			oled_display_set_inverse_font( &g_Display, false );

			vTaskDelay( 2000 / portTICK_PERIOD_MS );

			//----------------------------------------------------------------------
			//	... flash display
			//
			for( uint8_t idx = 0 ; idx < 10 ; idx++ )
			{
				bInvertDisplay = !bInvertDisplay;

				oled_display_set_inverse( &g_Display, bInvertDisplay );
				
				vTaskDelay( 250 / portTICK_PERIOD_MS );
			}

			vTaskDelay( 3000 / portTICK_PERIOD_MS );

			//----------------------------------------------------------------------
			//	print info text and ...
			//
			oled_display_clear( &g_Display );

			oled_display_set_cursor( &g_Display, 3, 0 );
			oled_display_print( &g_Display, "Now flip display" );

			vTaskDelay( 2000 / portTICK_PERIOD_MS );

			//----------------------------------------------------------------------
			//	... flip the display
			//
			oled_display_flip( &g_Display, true );

			oled_display_println( &g_Display, "The text now" );
			oled_display_println( &g_Display, "is turned by" );
			oled_display_println( &g_Display, "180 degree." );

			vTaskDelay( 4000 / portTICK_PERIOD_MS );

			oled_display_print( &g_Display, "\nand flip back" );

			vTaskDelay( 2000 / portTICK_PERIOD_MS );

			oled_display_flip( &g_Display, false );

			oled_display_println( &g_Display, "The text is" );
			oled_display_println( &g_Display, "back to normal" );

			vTaskDelay( 5000 / portTICK_PERIOD_MS );

			oled_display_clear( &g_Display );
		}
	}
}
