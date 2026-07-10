#include "sh1106.h"

/*
    PAGE TO PIXEL COORD
    (x, y) -> page = y / 8;
    bit in page = y % 8

    index in array -> each page has 128 columns/bytes -> 128*page + x (pages are 0 to 7)
*/

uint8_t oled_framebuffer[ 128 * 8 ] = {0}; // 1 page = 1 byte -> * 128 columns

void oled_command( uint8_t cmd ) {
    oled_cs_low( );
    oled_dc_command( );

    pal_spi_send( cmd );

    oled_cs_high( );
}

void oled_data( uint8_t cmd ) {
    oled_cs_low( );
    oled_dc_data( );

    pal_spi_send( cmd );

    oled_cs_high( );
}

void oled_reset( ) {
    oled_reset_low();
    sysDelay(5);

    oled_reset_high();
    sysDelay(5);
}

void oled_init( ) { 
    // TURN OFF DISPLAY -> So it is stable while configuring
    oled_command( 0xAE );

    // CLOCK DIVIDE RATIO -> 2 byte command
    oled_command( 0xD5 );
    oled_command( 0x50 );

    // MULTIPLEX RATIO -> how many lines to actively scan
    oled_command( 0xA8 );
    // default is 64 already -> no need to change
    oled_command( 0x3F );

    // DISPLAY OFFSET -> keeping default
    oled_command( 0xD3 );
    oled_command( 0x00 );

    //Set Display Start Line -> keeping default
    oled_command( 0x40 );
    
    //Set Segment Re-map
    oled_command( 0xA1 ); //A0 if appearing mirrored horizontally

    //Common Output Scan Direction
    oled_command( 0xC8 ); // vertical mirroring -> C0

    // COM Hardware Config -> no change
    oled_command(0xDA);
    oled_command(0x12);

    //Contrast
    oled_command( 0x81 );
    oled_command( 0xCF );

    //DISPLAY ON -> as in RAM 
    oled_command(0xA4);

    //Normal/Inverted Display
    // oled_command(0xA6);

    oled_command( 0xAF );

}

void oled_set_cursor( uint8_t page, uint8_t column) {
    oled_command(0xB0 | page);

    oled_command(0x02 + (column & 0xF));      // lower column
    oled_command(0x10 + (column >> 4));        // higher column
}

void oled_fill(uint8_t pattern)
{
    for(uint8_t page = 0; page < 8; page++)
    {
        oled_set_cursor(page, 0);

        for(uint8_t col = 0; col < 128; col++)
        {
            oled_data(pattern);
        }
    }
}

void oled_clear( ) {
    for ( uint16_t i = 0; i < sizeof( oled_framebuffer ); i++ ){
        oled_framebuffer[i] = 0;
    }

    // memset(oled_framebuffer, 0, sizeof(oled_framebuffer)); -> memset not available right now as no c std libs are being compiled
}

void oled_set_pixel(uint8_t x, uint8_t y)
{
    uint8_t page = y / 8;
    uint8_t bit = y % 8;

    oled_framebuffer[page * 128 + x] |= (1U << bit);
}

void oled_clear_pixel(uint8_t x, uint8_t y)
{
    uint8_t page = y / 8;
    uint8_t bit = y % 8;

    oled_framebuffer[page * 128 + x] &= ~(1U << bit);
}

void oled_toggle_pixel(uint8_t x, uint8_t y)
{
    uint8_t page = y / 8;
    uint8_t bit = y % 8;

    oled_framebuffer[page * 128 + x] ^= (1U << bit);
}

void oled_update(void)
{
    for(uint8_t page = 0; page < 8; page++)
    {
        oled_set_cursor(page, 0);

        for(uint8_t col = 0; col < 128; col++)
        {
            oled_data(oled_framebuffer[page * 128 + col]);
        }
    }
}