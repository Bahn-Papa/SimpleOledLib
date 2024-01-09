//##########################################################################
//#
//#		PrintText
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
//#	This program will write Text on an OLED display and demonstrates
//#	some options of the SimpleOled library, like
//#		- Cursor positioning
//#		- normal and inverse text font
//#
//#-------------------------------------------------------------------------
//#
//#	Dieses Programm schreibt Text auf ein OLED Display und zeigt dabei
//#	einige Möglichkeiten aus der SimpleOled Library:
//#		- Cursor positionieren
//#		- normale und inverse Textausgabe
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

const char strPrintTextDemo[]	= "Print Text Demo\n";
const char strNormalText[]		= "Normal Text\n";
const char strClearDisplay[]	= "Clear Display\n";

oled_display_handle_t	g_Display;

char g_buffer[ 20 ];


/******************************************************************
 *	i2c_bus_init
 */
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
	uint8_t		retValue;


	vTaskDelay( 5000 / portTICK_PERIOD_MS );

	i2c_bus_init( I2C_NUM_0 );

	printf( "SimpleOled Demo: PrintText\n" );
	printf( "  setup chip type ssd1306\n" );

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
			printf( strPrintTextDemo );

			oled_display_print( &g_Display, strPrintTextDemo );

			vTaskDelay( 2000 / portTICK_PERIOD_MS );

			//----------------------------------------------------------------------
			//	positioning the cursor to the beginning of the third line
			//	print text in normal font mode
			//
			printf( strNormalText );

			oled_display_set_cursor( &g_Display, 2, 0 );
			oled_display_println( &g_Display, strNormalText );

			vTaskDelay( 1000 / portTICK_PERIOD_MS );

			//----------------------------------------------------------------------
			//	now print some text in inverse font mode
			//
			printf( "Inverse Font\n" );

			oled_display_set_inverse_font( &g_Display, true );
			oled_display_print( &g_Display, "Inverse Font" );

			vTaskDelay( 3000 / portTICK_PERIOD_MS );

			//----------------------------------------------------------------------
			//	some additional text to demonstrate how to clear a line
			//
			printf( "Clear this line\n" );

			oled_display_set_cursor( &g_Display, 6, 0 );
			oled_display_print( &g_Display, "Clear this Line" );
			oled_display_set_inverse_font( &g_Display, false );

			vTaskDelay( 2000 / portTICK_PERIOD_MS );

			//----------------------------------------------------------------------
			//	this will clear the line where the cursor is in
			//
			oled_display_clear_actual_line( &g_Display );

			vTaskDelay( 1000 / portTICK_PERIOD_MS );

			//----------------------------------------------------------------------
			//	and now clear the hole display
			//	first some text ...
			//
			printf( strClearDisplay );

			oled_display_print( &g_Display, strClearDisplay );

			vTaskDelay( 2000 / portTICK_PERIOD_MS );

			//----------------------------------------------------------------------
			//	... and now clear the display
			//
			oled_display_clear( &g_Display );

			vTaskDelay( 500 / portTICK_PERIOD_MS );

			//----------------------------------------------------------------------
			//	another example for positioning the cursor and writing some text
			//
			printf( "Print stars\n" );

			oled_display_set_inverse_font( &g_Display, true );
			oled_display_set_cursor( &g_Display, 0, 0 );
			oled_display_print_char( &g_Display, '*' );
			oled_display_set_cursor( &g_Display, 0, 15 );
			oled_display_print_char( &g_Display, '*' );
			oled_display_set_cursor( &g_Display, 7, 0 );
			oled_display_print_char( &g_Display, '*' );
			oled_display_set_cursor( &g_Display, 7, 15 );
			oled_display_print_char( &g_Display, '*' );
			oled_display_set_inverse_font( &g_Display, false );

			vTaskDelay( 500 / portTICK_PERIOD_MS );

			//----------------------------------------------------------------------
			//	In this example you can see how to print text with numbers in it.
			//	First prepare the text (in this case with function sprintf).
			//	Then print the text.
			printf( "Print lines\n" );

			for( uint8_t idx = 0 ; idx < oled_display_max_text_lines() ; idx++ )
			{
				oled_display_set_cursor( &g_Display, idx, 4 );

				sprintf( g_buffer, "Zeile %d", idx );
				oled_display_print( &g_Display, g_buffer );

				vTaskDelay( 250 / portTICK_PERIOD_MS );
			}

			vTaskDelay( 2000 / portTICK_PERIOD_MS );

			oled_display_clear( &g_Display );

			vTaskDelay( 500 / portTICK_PERIOD_MS );

			//----------------------------------------------------------------------
			//	the last example for printing text.
			//
			printf( "Print columns\n" );

			oled_display_set_cursor( &g_Display, 4, 0 );
			oled_display_print( &g_Display, "Column" );
			oled_display_set_cursor( &g_Display, 3, 0 );
			
			for( uint8_t idx = 0 ; idx < oled_display_max_column_lines() ; idx++ )
			{
				if( 9 < idx )
				{
					oled_display_set_cursor( &g_Display, 2, idx );
					oled_display_print( &g_Display, "1" );

					sprintf( g_buffer, "%d", idx - 10 );
					oled_display_set_cursor( &g_Display, 3, idx );
				}
				else
				{
					sprintf( g_buffer, "%d", idx );
				}

				oled_display_print( &g_Display, g_buffer );
				
				vTaskDelay( 250 / portTICK_PERIOD_MS );
			}

			vTaskDelay( 2000 / portTICK_PERIOD_MS );

			oled_display_clear( &g_Display );

			vTaskDelay( 2000 / portTICK_PERIOD_MS );
		}
	}
}
