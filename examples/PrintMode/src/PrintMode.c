//##########################################################################
//#
//#		PrintMode
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
//#	the different PrintModes:
//#		- Overwrite Next Line
//#		- Overwrite Same Line
//#		- Scroll Line (default mode)
//#
//#-------------------------------------------------------------------------
//#
//#		Description of the PrintModes
//#
//#	Overwrite Next Line
//#		Whenever the output of a text comes to the end of a line and there
//#		is still some text to print then the cursor is set to the beginning
//#		of the next line. The output of the text will continue starting
//#		from this new position. If there is old text in this new line then
//#		it will be overwriten.
//#		If the output of a text comes to the end of the last line of the
//#		display then the output of the text will continue starting at the
//#		top left corner (home position) of the display. Again old text will
//#		be overwriten.
//#
//#	Overwrite Same Line
//#		Whenever the output of a text comes to the end of a line and there
//#		is still some text to print then the cursor is set to the beginning
//#		of the actual line. The output of the text will continue starting
//#		from this position. The old text in this line will be overwriten.
//#		As long as the Print function without a '\n' in the text is used
//#		the output of text will go into the same line.
//#		To change the line use function SetCursor, function PrintLn or
//#		use the character '\n' within the text.
//#		If the function PrintLn or the character '\n' is used in the last
//#		line of the display then the output of the text will continue
//#		starting at the home position of the display. Again old text will
//#		be overwriten.
//#
//#	Scroll Line
//#		Basically it is the same behavior as 'Overwrite Next Line' mode.
//#		But if the output of the text reaches the end of the last line of
//#		the display then all lines on the display will be shifted up by
//#		one line. The last line of the display will be cleared and the
//#		cursor will be set to the beginning of that line.
//#		So if just the functions Print and PrintLn or the character '\n'
//#		are used all new text will be printed into the last
//#		line of the display.
//#		If the output of a text should go into another line then the
//#		function SetCursor must be used.
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

const char g_strPrintModes[]		= "PrintModes Demo";
const char g_strOverwriteNextLine[]	= " Overwrite Next Line ";
const char g_strOverwriteSameLine[]	= " Overwrite Same Line ";
const char g_strNewLineCharFound[]	= "\n New Line Character found ";
const char g_strScrollLine[]		= " Scroll Line (default mode) ";

bool g_bInverseFont	= false;

oled_display_handle_t	g_Display;


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
	uint8_t	 retValue;
	char	*pchLetter;


	vTaskDelay( 7000 / portTICK_PERIOD_MS );

	i2c_bus_init( I2C_NUM_0 );

	printf( "SimpleOled Demo: PrintModes\n" );
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
			//------------------------------------------------------------------
			//	start with PrintMode 'Scroll Line' (the default mode)
			//
			printf( "Print Mode: Scroll Line (default)\n" );

			oled_display_clear( &g_Display );
			oled_display_print( &g_Display, g_strPrintModes );

			vTaskDelay( 2000 / portTICK_PERIOD_MS );

			oled_display_set_print_mode( &g_Display, PM_SCROLL_LINE );

			oled_display_set_cursor( &g_Display, 2, 0 );
			oled_display_println( &g_Display, g_strScrollLine );

			for( uint8_t count = 0 ; count < 7 ; count++ )
			{
				pchLetter		= (char *)g_strScrollLine;
				g_bInverseFont	= !g_bInverseFont;

				oled_display_set_inverse_font( &g_Display, g_bInverseFont );

				while( '\0' != *pchLetter )
				{
					oled_display_print_char( &g_Display, *pchLetter++ );

					vTaskDelay( 100 / portTICK_PERIOD_MS );
				}
			}

			g_bInverseFont = false;
			oled_display_set_inverse_font( &g_Display, false );

			vTaskDelay( 5000 / portTICK_PERIOD_MS );


			//------------------------------------------------------------------
			//	next PrintMode is 'Overwrite Next Line'
			//
			printf( "Print Mode: Overwrite Next Line\n" );

			oled_display_clear( &g_Display );
			oled_display_print( &g_Display, g_strPrintModes );

			vTaskDelay( 2000 / portTICK_PERIOD_MS );

			oled_display_set_print_mode( &g_Display, PM_OVERWRITE_NEXT_LINE );

			oled_display_set_cursor( &g_Display, 2, 0 );
			oled_display_println( &g_Display, g_strOverwriteNextLine );

			for( uint8_t count = 0 ; count < 7 ; count++ )
			{
				pchLetter		= (char *)g_strOverwriteNextLine;
				g_bInverseFont	= !g_bInverseFont;

				oled_display_set_inverse_font( &g_Display, g_bInverseFont );

				while( '\0' != *pchLetter )
				{
					oled_display_print_char( &g_Display, *pchLetter++ );

					vTaskDelay( 100 / portTICK_PERIOD_MS );
				}
			}

			g_bInverseFont = false;
			oled_display_set_inverse_font( &g_Display, false );

			vTaskDelay( 5000 / portTICK_PERIOD_MS );


			//------------------------------------------------------------------
			//	the last PrintMode is 'Overwrite Same Line'
			//
			printf( "Print Mode: Overwrite Same Line\n" );

			oled_display_clear( &g_Display );
			oled_display_print( &g_Display, g_strPrintModes );

			vTaskDelay( 2000 / portTICK_PERIOD_MS );

			oled_display_set_cursor( &g_Display, 2, 0 );
			oled_display_println( &g_Display, g_strOverwriteSameLine );

			oled_display_set_print_mode( &g_Display, PM_OVERWRITE_SAME_LINE );

			//------------------------------------------------------------------
			//	first overwrite only the same line
			//
			for( uint8_t count = 0 ; count < 5 ; count++ )
			{
				pchLetter		= (char *)g_strOverwriteSameLine;
				g_bInverseFont	= !g_bInverseFont;

				oled_display_set_inverse_font( &g_Display, g_bInverseFont );

				while( '\0' != *pchLetter )
				{
					oled_display_print_char( &g_Display, *pchLetter++ );

					vTaskDelay( 100 / portTICK_PERIOD_MS );
				}
			}

			//------------------------------------------------------------------
			//	but with a New Line you can go into the next line
			//
			printf( "  ... but with a New Line character you can go into the next line\n" );

			for( uint8_t count = 0 ; count < 5 ; count++ )
			{
				pchLetter		= (char *)g_strNewLineCharFound;
				g_bInverseFont	= !g_bInverseFont;

				oled_display_set_inverse_font( &g_Display, g_bInverseFont );

				while( '\0' != *pchLetter )
				{
					oled_display_print_char( &g_Display, *pchLetter++ );

					vTaskDelay( 100 / portTICK_PERIOD_MS );
				}
			}

			g_bInverseFont = false;
			oled_display_set_inverse_font( &g_Display, false );

			vTaskDelay( 5000 / portTICK_PERIOD_MS );
		}
	}
}
