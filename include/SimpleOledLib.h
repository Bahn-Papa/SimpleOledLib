
#pragma once

//##########################################################################
//#
//#		SimpleOledLib.h
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

#include <inttypes.h>
#include <driver/i2c.h>


//==========================================================================
//
//		D E F I N I T I O N S
//
//==========================================================================

#define	DISPLAY_ADDRESS_ONE			60
#define DISPLAY_ADDRESS_TWO			61
#define DISPLAY_ADDRESS_DEFAULT		255


//==========================================================================
//
//		T Y P E   D E F I N I T I O N S
//
//==========================================================================

typedef enum chip_type
{
	CHIP_TYPE_SH1106	= 0,
	CHIP_TYPE_SSD1306

} chip_type_t;


//----------------------------------------------------------------------
//	The different print modes
//
//	PM_OVERWRITE_SAME_LINE:
//		if the text output comes to the end of a line then continue
//		with the output in the same line and overwrite an existing text.
//
//	PM_OVERWRITE_NEXT_LINE:
//		if the text output comes to the end of a line then continue
//		with the output in the next line and perhaps overwrite an
//		existing text.
//		if it was the last line of the display then jumpt to the first
//		line and continue the output there.
//
//	PM_SCROLL_LINE:
//		if the text output comes to the end of a line then continue
//		with the output in the next line.
//		If it was the last line of the display then scroll all lines
//		up one line, discarding the first line, clear the last line
//		and continue the output in the cleared last line.
//
typedef enum print_mode
{
	PM_OVERWRITE_SAME_LINE	= 1,
	PM_OVERWRITE_NEXT_LINE,
	PM_SCROLL_LINE

} print_mode_t;


//----------------------------------------------------------------------
//	the display structure
//
typedef struct oled_display_handle
{
	i2c_port_t		port;
	chip_type_t		chipType;
	uint8_t			address;
	print_mode_t	printMode;
	uint8_t			displayColumnOffset;
	uint8_t			textLine;
	uint8_t			textColumn;
	uint8_t			lineOffset;
	bool			displayConnected;
	bool			inverse;

} oled_display_handle_t;


//==========================================================================
//
//		E X T E R N   F U N C T I O N S
//
//==========================================================================

uint8_t oled_display_max_text_lines( void );
uint8_t oled_display_max_column_lines( void );

uint8_t oled_display_init( oled_display_handle_t *pHandle, i2c_port_t port, chip_type_t chipType, uint8_t address );

void oled_display_print_char( oled_display_handle_t *pHandle, uint8_t charIdx );
void oled_display_print( oled_display_handle_t *pHandle, const char* strText );
void oled_display_println( oled_display_handle_t *pHandle, const char* strText );

void oled_display_clear( oled_display_handle_t *pHandle );
void oled_display_clear_line( oled_display_handle_t *pHandle, uint8_t lineToClear );
inline void oled_display_clear_actual_line( oled_display_handle_t *pHandle )
{
	oled_display_clear_line( pHandle, pHandle->textLine );
};

void oled_display_set_cursor( oled_display_handle_t *pHandle, uint8_t textLine, uint8_t textColumn );

inline void oled_display_home( oled_display_handle_t *pHandle )
{
	oled_display_set_cursor( pHandle, 0, 0 );
};

void oled_display_set_inverse( oled_display_handle_t *pHandle, bool inverse );
void oled_display_flip( oled_display_handle_t *pHandle, bool flip );

inline void oled_display_set_inverse_font( oled_display_handle_t *pHandle, bool bInverse )
{
	pHandle->inverse = bInverse;
};

inline void oled_display_set_print_mode( oled_display_handle_t *pHandle, print_mode_t printMode )
{
	pHandle->printMode = printMode;
};

void oled_display_set_display_column_offset( oled_display_handle_t *pHandle, uint8_t offset );
