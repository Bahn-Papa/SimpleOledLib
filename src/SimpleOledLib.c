//##########################################################################
//#
//#		SimpleOledLib.c
//#
//#-------------------------------------------------------------------------
//#
//#	The functions in this library are used to control an OLED display
//#	with an sh1106 or ssd1306 chipset via the I²C bus.
//#	Supported are only simple text output and some auxiliary functions,
//#	e.g.: clear display, clear line, position cursor, etc.
//#	If no display is connected nothing will be send over the I2C bus.
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
//#	File Version:	1		Date: 14.12.2023
//#
//#	Implementation:
//#		-	First implementation of the functions
//#
//##########################################################################


//==========================================================================
//
//		I N C L U D E S
//
//==========================================================================

#include "SimpleOledLib.h"
#include "font.h"


//==========================================================================
//
//		D E F I N I T I O N S
//
//==========================================================================

//#define	PRINT_DEBUG_INFO


#define	TEXT_LINES						8
#define TEXT_COLUMNS					16

#define PIXELS_CHAR_HEIGHT				8
#define PIXELS_CHAR_WIDTH				8

#define DISPLAY_COLUMN_OFFSET_MAX		3
#define DISPLAY_COLUMN_OFFSET_MIN		0
#define DISPLAY_COLUMN_OFFSET_DEFAULT	2


//--------------------------------------------------------------------------
//	Definitions for I²C protocol
//

//----	common command codes (for both chips)  -------------------------
#define	OPC_COLUMN_ADDRESS_LOW			0x00
#define OPC_COLUMN_ADDRESS_HIGH			0x10
#define OPC_DISPLAY_START_LINE			0x40
#define OPC_SET_CONTRAST				0x81
#define OPC_SEG_ROTATION_RIGHT			0xA0
#define OPC_SEG_ROTATION_LEFT			0xA1
#define	OPC_ENTIRE_DISPLAY_NORMAL		0xA4
#define	OPC_ENTIRE_DISPLAY_ON			0xA5
#define OPC_MODE_NORMAL					0xA6
#define OPC_MODE_INVERSE				0xA7
#define OPC_SET_MULTIPLEX_RATIO			0xA8
#define OPC_DISPLAY_OFF					0xAE
#define OPC_DISPLAY_ON					0xAF
#define	OPC_PAGE_ADDRESS				0xB0
#define OPC_OUTPUT_SCAN_NORMAL			0xC0
#define OPC_OUTPUT_SCAN_INVERSE			0xC8
#define OPC_DISPLAY_LINE_OFFSET			0xD3
#define OPC_CLK_DIV_OSC_FREQ			0xD5
#define OPC_DIS_PRE_CHARGE_PERIOD		0xD9
#define OPC_SET_COM_PINS				0xDA
#define OPC_SET_VCOM_DESELECT_LEVEL		0xDB

#define OPC_NOP							0xE3

//----	sh1106 specific command codes  ---------------------------------
#define OPC_DC_DC_PUMP_VOLTAGE_6_4		0x30
#define OPC_DC_DC_PUMP_VOLTAGE_7_4		0x31
#define OPC_DC_DC_PUMP_VOLTAGE_8_0		0x32
#define OPC_DC_DC_PUMP_VOLTAGE_9_0		0x33
#define OPC_DC_DC_CONTROL_MODE			0xAD

//----	ssd1306 specific command codes  --------------------------------
#define OPC_MEMORY_ADR_MODE				0x20
#define OPC_DEACTIVATE_SCROLL			0x2E
#define OPC_CHARGE_PUMP_SETTING			0x8D

//----	prefix codes  --------------------------------------------------
#define	PREFIX_NEXT_COMMAND				0x80
#define PREFIX_LAST_COMMAND				0x00
#define PREFIX_DATA						0x40

//----	Masks to prepare commands  -------------------------------------
#define MASK_PAGE_ADDRESS				0x0F
#define	MASK_COLUMN_ADDRESS_LOW			0x0F
#define MASK_COLUMN_ADDRESS_HIGH		0xF0

//----	Idx into command buffer  ---------------------------------------
#define	IDX_PAGE_ADDRESS				1
#define IDX_COLUMN_ADDRESS_LOW			3
#define IDX_COLUMN_ADDRESS_HIGH			5

//----	memory addressing modes  ---------------------------------------
#define ADR_MODE_HORIZONTAL				0x00
#define ADR_MODE_VERTICAL				0x01
#define ADR_MODE_PAGE					0x02

//----	DC DC control modes  -------------------------------------------
#define DC_DC_OFF						0x8A
#define DC_DC_ON						0x8B

//----	Clock divide ratio values  -------------------------------------
#define CLOCK_DIV_RATIO_1				0x00
#define CLOCK_DIV_RATIO_2				0x01
#define CLOCK_DIV_RATIO_3				0x02
#define CLOCK_DIV_RATIO_4				0x03
#define CLOCK_DIV_RATIO_5				0x04
#define CLOCK_DIV_RATIO_6				0x05
#define CLOCK_DIV_RATIO_7				0x06
#define CLOCK_DIV_RATIO_8				0x07
#define CLOCK_DIV_RATIO_9				0x08
#define CLOCK_DIV_RATIO_10				0x09
#define CLOCK_DIV_RATIO_11				0x0A
#define CLOCK_DIV_RATIO_12				0x0B
#define CLOCK_DIV_RATIO_13				0x0C
#define CLOCK_DIV_RATIO_14				0x0D
#define CLOCK_DIV_RATIO_15				0x0E
#define CLOCK_DIV_RATIO_16				0x0F

//----	Oscilator frequence variation  ---------------------------------
#define OSC_FREQ_VARIATION_M_25			0x00
#define OSC_FREQ_VARIATION_M_20			0x10
#define OSC_FREQ_VARIATION_M_15			0x20
#define OSC_FREQ_VARIATION_M_10			0x30
#define OSC_FREQ_VARIATION_M_5			0x40
#define OSC_FREQ_VARIATION_P_M_0		0x50
#define OSC_FREQ_VARIATION_P_5			0x60
#define OSC_FREQ_VARIATION_P_10			0x70
#define OSC_FREQ_VARIATION_P_15			0x80
#define OSC_FREQ_VARIATION_P_20			0x90
#define OSC_FREQ_VARIATION_P_25			0xA0
#define OSC_FREQ_VARIATION_P_30			0xB0
#define OSC_FREQ_VARIATION_P_35			0xC0
#define OSC_FREQ_VARIATION_P_40			0xD0
#define OSC_FREQ_VARIATION_P_45			0xE0
#define OSC_FREQ_VARIATION_P_50			0xF0

//----	Pre Charge periods  --------------------------------------------
#define PRE_CHARGE_PERIOD_DCLK_1		0x01
#define PRE_CHARGE_PERIOD_DCLK_2		0x02
#define PRE_CHARGE_PERIOD_DCLK_3		0x03
#define PRE_CHARGE_PERIOD_DCLK_4		0x04
#define PRE_CHARGE_PERIOD_DCLK_5		0x05
#define PRE_CHARGE_PERIOD_DCLK_6		0x06
#define PRE_CHARGE_PERIOD_DCLK_7		0x07
#define PRE_CHARGE_PERIOD_DCLK_8		0x08
#define PRE_CHARGE_PERIOD_DCLK_9		0x09
#define PRE_CHARGE_PERIOD_DCLK_10		0x0A
#define PRE_CHARGE_PERIOD_DCLK_11		0x0B
#define PRE_CHARGE_PERIOD_DCLK_12		0x0C
#define PRE_CHARGE_PERIOD_DCLK_13		0x0D
#define PRE_CHARGE_PERIOD_DCLK_14		0x0E
#define PRE_CHARGE_PERIOD_DCLK_15		0x0F

//----	Dis Charge periods  --------------------------------------------
#define DIS_CHARGE_PERIOD_DCLK_1		0x10
#define DIS_CHARGE_PERIOD_DCLK_2		0x20
#define DIS_CHARGE_PERIOD_DCLK_3		0x30
#define DIS_CHARGE_PERIOD_DCLK_4		0x40
#define DIS_CHARGE_PERIOD_DCLK_5		0x50
#define DIS_CHARGE_PERIOD_DCLK_6		0x60
#define DIS_CHARGE_PERIOD_DCLK_7		0x70
#define DIS_CHARGE_PERIOD_DCLK_8		0x80
#define DIS_CHARGE_PERIOD_DCLK_9		0x90
#define DIS_CHARGE_PERIOD_DCLK_10		0xA0
#define DIS_CHARGE_PERIOD_DCLK_11		0xB0
#define DIS_CHARGE_PERIOD_DCLK_12		0xC0
#define DIS_CHARGE_PERIOD_DCLK_13		0xD0
#define DIS_CHARGE_PERIOD_DCLK_14		0xE0
#define DIS_CHARGE_PERIOD_DCLK_15		0xF0


//==========================================================================
//
//		G L O B A L   V A R I A B L E S
//
//==========================================================================

uint8_t g_displayCommandBuffer[] =
	{
		PREFIX_LAST_COMMAND,
		0x00,
		0x00
	};

uint8_t	g_arusPositionCommandBuffer[] =
	{
		PREFIX_NEXT_COMMAND,
		OPC_PAGE_ADDRESS,
		PREFIX_NEXT_COMMAND,
		OPC_COLUMN_ADDRESS_LOW,
		PREFIX_LAST_COMMAND,
		OPC_COLUMN_ADDRESS_HIGH
	};

//const uint8_t *gp_CommandBuffer = (const uint8_t *)g_arusPositionCommandBuffer;


//==========================================================================
//
//		L O C A L   F U N C T I O N   D E C L A R A T I O N
//
//==========================================================================

void _oled_display_init_sh1106(  oled_display_handle_t *pHandle );
void _oled_display_init_ssd1306( oled_display_handle_t *pHandle );
void _oled_display_next_line( oled_display_handle_t *pHandle, bool shiftLine );
void _oled_display_send_opcode( oled_display_handle_t *pHandle, uint8_t opCode );
void _oled_display_send_parameter( oled_display_handle_t *pHandle, uint8_t opCode, uint8_t parameter );
void _oled_display_shift_display_one_line( oled_display_handle_t *pHandle );


//==========================================================================
//
//		F U N C T I O N S
//
//==========================================================================


//**************************************************************************
//	oled_display_max_text_lines
//--------------------------------------------------------------------------
//	The Function will return the number of text lines that the display
//	can handle
//
uint8_t oled_display_max_text_lines( void )
{
	return( TEXT_LINES );
}


//**************************************************************************
//	oled_display_max_column_lines
//--------------------------------------------------------------------------
//	The Function will return the number of text columns that the display
//	can handle
//
uint8_t oled_display_max_column_lines( void )
{
	return( TEXT_COLUMNS );
}


//**************************************************************************
//	oled_display_init
//--------------------------------------------------------------------------
//	The Function initializes the class, sets the display in default
//	operation mode, switches the display 'on', clears the display and
//	sets the cursor to home position (top left corner).
//
uint8_t oled_display_init( oled_display_handle_t *pHandle, i2c_port_t port, chip_type_t chipType, uint8_t address )
{
	//------------------------------------------------------------------
	//	set initial values for internal variables
	//
	pHandle->port					= port;
	pHandle->chipType				= chipType;
	pHandle->address				= address;
	pHandle->printMode				= PM_SCROLL_LINE;
	pHandle->textLine				= 0;
	pHandle->textColumn				= 0;
	pHandle->lineOffset				= 0;
	pHandle->displayColumnOffset	= 0;
	pHandle->displayConnected		= false;
	pHandle->inverse				= false;


	//------------------------------------------------------------------
	//	Check the given address
	//
	if( 	(DISPLAY_ADDRESS_ONE != address)
		&&	(DISPLAY_ADDRESS_TWO != address)
		&&	(DISPLAY_ADDRESS_DEFAULT != address) )
	{
		//----------------------------------------------------------
		//	no valid address
		//
		return( 1 );
	}

	if( DISPLAY_ADDRESS_DEFAULT == address )
	{
		if( ESP_OK == i2c_master_write_to_device( port, DISPLAY_ADDRESS_ONE, g_arusPositionCommandBuffer, 0, 50 / portTICK_PERIOD_MS ) )
		{
			address	=	DISPLAY_ADDRESS_ONE;
		}
		else if( ESP_OK == i2c_master_write_to_device( port, DISPLAY_ADDRESS_TWO, g_arusPositionCommandBuffer, 0, 50 / portTICK_PERIOD_MS ) )
		{
			address	=	DISPLAY_ADDRESS_TWO;
		}
		else
		{
			//--------------------------------------------------
			//	no display connected
			//
			return( 2 );
		}
	}

	//------------------------------------------------------------------
	//	Check if Display can be connected under the given address
	//
	if( ESP_OK == i2c_master_write_to_device( port, address, g_arusPositionCommandBuffer, 0, 50 / portTICK_PERIOD_MS ) )
	{
		//----------------------------------------------------------
		//	YES the display can be connected with the given address
		//	so initialize the display
		//
		pHandle->address			= address;
		pHandle->displayConnected	= true;

		if( CHIP_TYPE_SSD1306 == chipType )
		{
			_oled_display_init_ssd1306( pHandle );
		}
		else
		{
			pHandle->displayColumnOffset = DISPLAY_COLUMN_OFFSET_DEFAULT;

			_oled_display_init_sh1106( pHandle );
		}

		_oled_display_send_opcode( pHandle, OPC_PAGE_ADDRESS );
		_oled_display_send_opcode( pHandle, OPC_COLUMN_ADDRESS_LOW );
		_oled_display_send_opcode( pHandle, OPC_COLUMN_ADDRESS_HIGH );
		_oled_display_send_opcode( pHandle, OPC_DISPLAY_ON );
		_oled_display_send_opcode( pHandle, OPC_SEG_ROTATION_RIGHT );
		_oled_display_send_opcode( pHandle, OPC_OUTPUT_SCAN_NORMAL );

		oled_display_clear( pHandle );

		return( 0 );
	}
	
	return( 2 );
}


//**************************************************************************
//	oled_display_print_char
//--------------------------------------------------------------------------
//	This function will print the given character on the display starting at
//	the actual cursor position.
//	If the text contains a new line character ('\n') then the output will
//	continue at the beginning of the next line.
//	If the text ouput reaches the end of the line then depending of the
//	PrintMode the cursor will be set to the beginning of the (next) line
//	and the text output will continue there.
//
void oled_display_print_char( oled_display_handle_t *pHandle, uint8_t charIdx )
{
	i2c_cmd_handle_t	cmd;
	uint16_t			uiHelper;
	uint8_t				usLetterColumn;

	if( pHandle->displayConnected )
	{
		if( '\n' == charIdx )
		{
			_oled_display_next_line( pHandle, true );
		}
		else if( (' ' <= charIdx) && (128 > charIdx) )
		{
			//--------------------------------------------------------------
			//	if we reached the end of the line then depending of the
			//	PrintMode continue in the 'next line'
			//
			if( TEXT_COLUMNS <= pHandle->textColumn )
			{
				_oled_display_next_line( pHandle, false );
			}

			//--------------------------------------------------------------
			//	this is a printable character, so calculate the pointer
			//	into the font array to that position where the bitmap of
			//	this character starts
			//
			uiHelper   = charIdx - 32;
			uiHelper <<= 3;	//	mit 8 multiplizieren

#ifdef PRINT_DEBUG_INFO
			printf( "PrintChar( %c ): Idx: %d => ", charIdx, uiHelper );
#endif

			//--------------------------------------------------------------
			//	transmit the bitmap of the character to the display
			//
			cmd = i2c_cmd_link_create();
			i2c_master_start( cmd );
			i2c_master_write_byte( cmd, (pHandle->address << 1) | I2C_MASTER_WRITE, true );
			i2c_master_write_byte( cmd, PREFIX_DATA, true );

			for( uint8_t idx = 0 ; PIXELS_CHAR_WIDTH > idx ; idx++ )
			{
				usLetterColumn = (uint8_t)font8x8_simple[ uiHelper ];
				uiHelper++;

#ifdef PRINT_DEBUG_INFO
				printf( " %02X ", usLetterColumn );
#endif

				if( pHandle->inverse )
				{
					usLetterColumn = ~usLetterColumn;
				}

				i2c_master_write_byte( cmd, usLetterColumn, true );
			}

			i2c_master_stop( cmd );
			i2c_master_cmd_begin( pHandle->port, cmd, 50 / portTICK_PERIOD_MS );
			i2c_cmd_link_delete( cmd );

#ifdef PRINT_DEBUG_INFO
			printf( "\n" );
#endif

			//--------------------------------------------------------------
			//	one character printed, so move cursor
			//
			pHandle->textColumn++;
		}
	}
}


//**************************************************************************
//	oled_display_print
//--------------------------------------------------------------------------
//	This function will print the given text on the display starting at
//	the actual cursor position.
//	If the text contains a new line character ('\n') then the output will
//	continue at the beginning of the next line.
//	If the text ouput reaches the end of the line then depending of the
//	PrintMode the cursor will be set to the beginning of the (next) line
//	and the text output will continue there.
//
void oled_display_print( oled_display_handle_t *pHandle, const char* strText )
{
	uint8_t *pText = (uint8_t *)strText;

	if( pHandle->displayConnected )
	{
		uint8_t	charIdx	= *pText++;

		while( 0x00 != charIdx )
		{
			oled_display_print_char( pHandle, charIdx );

			charIdx = *pText++;
		}
	}
}


//**************************************************************************
//	oled_display_println
//--------------------------------------------------------------------------
//	This function will print the given text on the display starting at
//	the actual cursor position and then sets the cursor to the beginning
//	of the next line.
//	If the text contains a new line character ('\n') then the output will
//	continue at the beginning of the next line.
//	If the text ouput reaches the end of the line then depending of the
//	PrintMode the cursor will be set to the beginning of the (next) line
//	and the text output will continue there.
//
void oled_display_println( oled_display_handle_t *pHandle, const char* strText )
{
	if( pHandle->displayConnected )
	{
		oled_display_print( pHandle, strText );
		_oled_display_next_line( pHandle, true );
	}
}


//**************************************************************************
//	oled_display_clear
//--------------------------------------------------------------------------
//	The function deletes all text shown on the display.
//
void oled_display_clear( oled_display_handle_t *pHandle )
{
	if( pHandle->displayConnected )
	{
		for( uint8_t usTextLine = 0 ; usTextLine < TEXT_LINES ; usTextLine++ )
		{
			oled_display_clear_line( pHandle, usTextLine );
		}

		//------------------------------------------------------------------
		//	Set the display line offset back to the default value '0'.
		//	That means beginn to display the display with the top line.
		//
		pHandle->lineOffset = 0;

		_oled_display_send_parameter( pHandle, OPC_DISPLAY_LINE_OFFSET, 0 );

		//------------------------------------------------------------------
		//	set the cursor to home position
		//
		oled_display_set_cursor( pHandle, 0, 0 );
	}
}


//**************************************************************************
//	oled_display_clear_line
//--------------------------------------------------------------------------
//	The function deletes the text line at the given cursor position and
//	sets the cursor to the bginning of that line.
//
void oled_display_clear_line( oled_display_handle_t *pHandle, uint8_t lineToClear )
{
	i2c_cmd_handle_t	cmd;
	uint8_t				usLoop1End;
	uint8_t				usLoop2End;


	if( pHandle->displayConnected )
	{
		//--------------------------------------------------------------
		//	at the end of the function the cursor will be positioned to
		//	the beginning of the line that will be cleared
		//
		pHandle->textLine	= lineToClear;
		pHandle->textColumn	= 0;

		//--------------------------------------------------------------
		//	take care of the display line shift
		//	and correct the line to clear accordingly
		//
		lineToClear += pHandle->lineOffset;

		if( TEXT_LINES <= lineToClear )
		{
			lineToClear -= TEXT_LINES;
		}

		//--------------------------------------------------------------
		//	preparation for the command that will be send to the
		//	display: set cursor to actual line first column
		//
		lineToClear &= MASK_PAGE_ADDRESS;
		g_arusPositionCommandBuffer[ IDX_PAGE_ADDRESS ] = OPC_PAGE_ADDRESS | lineToClear;
		g_arusPositionCommandBuffer[ IDX_COLUMN_ADDRESS_LOW  ] = OPC_COLUMN_ADDRESS_LOW;
		g_arusPositionCommandBuffer[ IDX_COLUMN_ADDRESS_HIGH ] = OPC_COLUMN_ADDRESS_HIGH;

		//--------------------------------------------------------------
		//	now send the commands to position the cursor to the display
		//
		i2c_master_write_to_device( pHandle->port, pHandle->address, g_arusPositionCommandBuffer, sizeof( g_arusPositionCommandBuffer ), 50 / portTICK_PERIOD_MS );

		//--------------------------------------------------------------
		//	split the number of bytes to be send to clear the display
		//	to less than 31 ?? (I2C buffer size)
		//
		if( CHIP_TYPE_SSD1306 == pHandle->chipType )
		{
			//------------------------------------------------------
			//	ssd1306 has 128 pixel columns (8 x 16 = 128)
			//
			usLoop1End	= 8;
			usLoop2End	= 16;
		}
		else
		{
			//------------------------------------------------------
			//	sh1106 has 132 pixel columns (6 x 22 = 132)
			//
			usLoop1End	= 6;
			usLoop2End	= 22;
		}

		for( uint8_t idx1 = 0 ; idx1 < usLoop1End ; idx1++ )
		{
			cmd = i2c_cmd_link_create();
			i2c_master_start( cmd );
			i2c_master_write_byte( cmd, (pHandle->address << 1) | I2C_MASTER_WRITE, true );
			i2c_master_write_byte( cmd, PREFIX_DATA, true );

			for( uint8_t idx2 = 0 ; idx2 < usLoop2End ; idx2++ )
			{
				i2c_master_write_byte( cmd, 0x00, true );
			}

			i2c_master_stop( cmd );
			i2c_master_cmd_begin( pHandle->port, cmd, 50 / portTICK_PERIOD_MS );
			i2c_cmd_link_delete( cmd );
		}

		//--------------------------------------------------------------
		//	set cursor to first text position of this line
		//
		g_arusPositionCommandBuffer[ IDX_COLUMN_ADDRESS_LOW  ] =	  OPC_COLUMN_ADDRESS_LOW
																	| pHandle->displayColumnOffset;

		i2c_master_write_to_device( pHandle->port, pHandle->address, g_arusPositionCommandBuffer, sizeof( g_arusPositionCommandBuffer ), 50 / portTICK_PERIOD_MS );
	}
}


//**************************************************************************
//	oled_display_set_cursor
//--------------------------------------------------------------------------
//	The function sets the cursor to the given line and column
//	valid values are:
//		line:	0 -  7
//		column:	0 - 15
//
void oled_display_set_cursor( oled_display_handle_t *pHandle, uint8_t textLine, uint8_t textColumn )
{
	uint8_t		usAddressLow;
	uint8_t		usAddressHigh;


	if( pHandle->displayConnected && (TEXT_LINES > textLine) && (TEXT_COLUMNS > textColumn) )
	{
		//------------------------------------------------------------------
		//	store the new cursor position
		//
		pHandle->textLine	= textLine;
		pHandle->textColumn	= textColumn;

		//------------------------------------------------------------------
		//	take care of the display line shift
		//	and correct the text line accordingly
		//
		textLine += pHandle->lineOffset;

		if( TEXT_LINES <= textLine )
		{
			textLine -= TEXT_LINES;
		}

		//------------------------------------------------------------------
		//	preparation for the command that will be send to the display
		//
		textLine &= MASK_PAGE_ADDRESS;
		g_arusPositionCommandBuffer[ IDX_PAGE_ADDRESS ] = OPC_PAGE_ADDRESS | textLine;

		//------------------------------------------------------------------
		//	calculate bit column
		//	the calculated bit column is the start column of a character
		//
		textColumn <<= 3;		//	multiply by 8
		textColumn  += pHandle->displayColumnOffset;

		//------------------------------------------------------------------
		//	preparation for the commands that will be send to the display
		//
		usAddressLow	 = textColumn & MASK_COLUMN_ADDRESS_LOW;
		g_arusPositionCommandBuffer[ IDX_COLUMN_ADDRESS_LOW  ] = OPC_COLUMN_ADDRESS_LOW | usAddressLow;

		usAddressHigh	 = textColumn & MASK_COLUMN_ADDRESS_HIGH;
		usAddressHigh >>= 4;
		g_arusPositionCommandBuffer[ IDX_COLUMN_ADDRESS_HIGH ] = OPC_COLUMN_ADDRESS_HIGH | usAddressHigh;

		//------------------------------------------------------------------
		//	now send the commands to position the cursor to the display
		//
		i2c_master_write_to_device( pHandle->port, pHandle->address, g_arusPositionCommandBuffer, sizeof( g_arusPositionCommandBuffer ), 50 / portTICK_PERIOD_MS );
	}
}


//**************************************************************************
//	oled_display_set_inverse
//--------------------------------------------------------------------------
//	This function inverses the display, means every OLED pixel that is 'on'
//	will be turned 'off' and vice versa.
//
void oled_display_set_inverse( oled_display_handle_t *pHandle, bool inverse )
{
	if( pHandle->displayConnected )
	{
		if( inverse )
		{
			_oled_display_send_opcode( pHandle, OPC_MODE_INVERSE );
		}
		else
		{
			_oled_display_send_opcode( pHandle, OPC_MODE_NORMAL );
		}
	}
}


//**************************************************************************
//	oled_display_flip
//--------------------------------------------------------------------------
//	This function will turn the output on the display by 180 degree
//	and clears the display.
//
void oled_display_flip( oled_display_handle_t *pHandle, bool flip )
{
	if( pHandle->displayConnected )
	{
		if( flip )
		{
			_oled_display_send_opcode( pHandle, OPC_SEG_ROTATION_LEFT );
			_oled_display_send_opcode( pHandle, OPC_OUTPUT_SCAN_INVERSE );
		}
		else
		{
			_oled_display_send_opcode( pHandle, OPC_SEG_ROTATION_RIGHT );
			_oled_display_send_opcode( pHandle, OPC_OUTPUT_SCAN_NORMAL );
		}
		
		oled_display_clear( pHandle );
	}
}


//**************************************************************************
//	SetDisplayColumnOffset
//--------------------------------------------------------------------------
//	With this function it is possible to adjust the display in left right
//	direction in a small range.
//
//	Background:
//	The OLED display with sh1106 chip has 132 columns of OLED pixels.
//	The font I use has 8 pixels per character.
//	So 128 pixels are used for one text line. This leads to a left over
//	of 4 pixels that can be used to adjust the text output on the display.
//
//	The OLED disable with ssd1306 chip has 128 columns of OLED pixels.
//	So the column offset is allways '0'.
//
void oled_display_set_display_column_offset( oled_display_handle_t *pHandle, uint8_t offset )
{
//-----------------------------------------------------------------------
//	suppress compiler warning
//
//	if( (DISPLAY_COLUMN_OFFSET_MIN <= usOffset) && (DISPLAY_COLUMN_OFFSET_MAX >= usOffset) )
//
	if( DISPLAY_COLUMN_OFFSET_MAX >= offset )
	{
		if( CHIP_TYPE_SSD1306 == pHandle->chipType )
		{
			pHandle->displayColumnOffset = 0;
		}
		else
		{
			pHandle->displayColumnOffset = offset;
		}
	}
}


//**************************************************************************
//	_oled_display_init_sh1106 (local)
//--------------------------------------------------------------------------
//	This function will send the initialize sequence to a display with
//	a ssd1306 chip type.
//
void _oled_display_init_sh1106(  oled_display_handle_t *pHandle )
{
	_oled_display_send_opcode( pHandle, OPC_DISPLAY_OFF );
	_oled_display_send_opcode( pHandle, OPC_ENTIRE_DISPLAY_NORMAL );
	_oled_display_send_parameter( pHandle, OPC_CLK_DIV_OSC_FREQ, (OSC_FREQ_VARIATION_P_M_0 | CLOCK_DIV_RATIO_1) );
	_oled_display_send_parameter( pHandle, OPC_SET_MULTIPLEX_RATIO, 0x3F );
	_oled_display_send_parameter( pHandle, OPC_DISPLAY_LINE_OFFSET, 0 );
	_oled_display_send_opcode( pHandle, OPC_DISPLAY_START_LINE );
	_oled_display_send_parameter( pHandle, OPC_CHARGE_PUMP_SETTING, 0x14 );
	_oled_display_send_parameter( pHandle, OPC_DC_DC_CONTROL_MODE, DC_DC_ON );
	_oled_display_send_parameter( pHandle, OPC_DIS_PRE_CHARGE_PERIOD, (DIS_CHARGE_PERIOD_DCLK_2 | PRE_CHARGE_PERIOD_DCLK_2) );
	_oled_display_send_parameter( pHandle, OPC_SET_VCOM_DESELECT_LEVEL, 0x35 );
	_oled_display_send_opcode( pHandle, OPC_DC_DC_PUMP_VOLTAGE_8_0 );
	_oled_display_send_parameter( pHandle, OPC_SET_CONTRAST, 0xFF );
	_oled_display_send_opcode( pHandle, OPC_MODE_NORMAL );
	_oled_display_send_parameter( pHandle, OPC_SET_COM_PINS, 0x12 );
}


//**************************************************************************
//	_oled_display_init_ssd1306 (local)
//--------------------------------------------------------------------------
//	This function will send the initialize sequence to a display with
//	a ssd1306 chip type.
//
void _oled_display_init_ssd1306( oled_display_handle_t *pHandle )
{
	_oled_display_send_opcode( pHandle, OPC_DISPLAY_OFF );
	_oled_display_send_parameter( pHandle, OPC_CLK_DIV_OSC_FREQ, (OSC_FREQ_VARIATION_P_15 | CLOCK_DIV_RATIO_1) );
	_oled_display_send_parameter( pHandle, OPC_SET_MULTIPLEX_RATIO, 0x3F );
	_oled_display_send_parameter( pHandle, OPC_DISPLAY_LINE_OFFSET, 0 );
	_oled_display_send_opcode( pHandle, OPC_DISPLAY_START_LINE );
	_oled_display_send_parameter( pHandle, OPC_CHARGE_PUMP_SETTING, 0x14 );
	_oled_display_send_parameter( pHandle, OPC_MEMORY_ADR_MODE, ADR_MODE_PAGE );
	_oled_display_send_parameter( pHandle, OPC_SET_COM_PINS, 0x12 );
	_oled_display_send_parameter( pHandle, OPC_SET_CONTRAST, 0xCF );
//	_oled_display_send_parameter( pHandle, OPC_DIS_PRE_CHARGE_PERIOD, (DIS_CHARGE_PERIOD_DCLK_2 | PRE_CHARGE_PERIOD_DCLK_2) );
	_oled_display_send_parameter( pHandle, OPC_DIS_PRE_CHARGE_PERIOD, (DIS_CHARGE_PERIOD_DCLK_15 | PRE_CHARGE_PERIOD_DCLK_1) );
	_oled_display_send_parameter( pHandle, OPC_SET_VCOM_DESELECT_LEVEL, 0x40 );
//	_oled_display_send_opcode( pHandle, OPC_DEACTIVATE_SCROLL );	//	I think this is not needed, because we are in Page mode
	_oled_display_send_opcode( pHandle, OPC_ENTIRE_DISPLAY_NORMAL );
	_oled_display_send_opcode( pHandle, OPC_MODE_NORMAL );
}


//**************************************************************************
//	_oled_display_send_opcode (local)
//--------------------------------------------------------------------------
//	This function will send a one byte command to the display.
//	A one byte command exists of
//		-	1 byte prefix
//		-	1 byte command code
//
void _oled_display_send_opcode( oled_display_handle_t *pHandle, uint8_t opCode )
{
	g_displayCommandBuffer[ 1 ] = opCode;

	i2c_master_write_to_device( pHandle->port, pHandle->address, g_displayCommandBuffer, 2, 50 / portTICK_PERIOD_MS );
}


//**************************************************************************
//	_oled_display_send_parameter (local)
//--------------------------------------------------------------------------
//	This function will send a two byte command to the display.
//	A two byte command exists of
//		-	1 byte prefix
//		-	1 byte command code
//		-	1 byte parameter
//
void _oled_display_send_parameter( oled_display_handle_t *pHandle, uint8_t opCode, uint8_t parameter )
{
	g_displayCommandBuffer[ 1 ] = opCode;
	g_displayCommandBuffer[ 2 ] = parameter;

	i2c_master_write_to_device( pHandle->port, pHandle->address, g_displayCommandBuffer, 3, 50 / portTICK_PERIOD_MS );
}


//**************************************************************************
//	_oled_display_next_line (local)
//--------------------------------------------------------------------------
//	The function will set the cursor to the beginning of the 'next print
//	line'. Which will be the 'next print line' depends on the print mode
//	and the function parameter.
//
void _oled_display_next_line( oled_display_handle_t *pHandle, bool shiftLine )
{
	pHandle->textColumn	= 0;

	if( PM_SCROLL_LINE == pHandle->printMode )
	{
		//------------------------------------------------------------------
		//	PrintMode is scroll line
		//	if		the cursor is in the last line of the display,
		//	then	stay there and shift all other lines one up
		//	else	set cursor to the next line
		//
		if( (TEXT_LINES - 1) == pHandle->textLine )
		{
			_oled_display_shift_display_one_line( pHandle );
		}
		else
		{
			pHandle->textLine++;
		}
	}
	else if( shiftLine || (PM_OVERWRITE_NEXT_LINE == pHandle->printMode) )
	{
		//------------------------------------------------------------------
		//	if PrintMode is overwrite next line or bShiftLine is 'true'
		//	set cursor to the next line
		//
		pHandle->textLine++;
	}

	//----------------------------------------------------------------------
	//	check if the cursor is set to the allowed line range
	//
	if( TEXT_LINES <= pHandle->textLine )
	{
		pHandle->textLine = 0;
	}

	//----------------------------------------------------------------------
	//	now set the cursor to the new position and
	//	if required clear the line
	//
	oled_display_set_cursor( pHandle, pHandle->textLine, pHandle->textColumn );

	if( PM_OVERWRITE_NEXT_LINE < pHandle->printMode )
	{
		oled_display_clear_actual_line( pHandle );
	}
}


//**************************************************************************
//	_oled_display_shift_display_one_line (local)
//--------------------------------------------------------------------------
//	Die Funktion
//
void _oled_display_shift_display_one_line( oled_display_handle_t *pHandle )
{
	pHandle->lineOffset++;
	
	if( TEXT_LINES <= pHandle->lineOffset )
	{
		pHandle->lineOffset = 0;
	}

	_oled_display_send_parameter( pHandle, OPC_DISPLAY_LINE_OFFSET, (pHandle->lineOffset << 3) );
}
