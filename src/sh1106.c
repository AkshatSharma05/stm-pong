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

void oled_set_pixel( uint8_t x, uint8_t y ) {
    if (x >= 128 || y >= 64) return; //no drawing out of bounds

    uint8_t page = y / 8;
    uint8_t bit = y % 8;

    oled_framebuffer[page * 128 + x] |= (1U << bit);
}

void oled_clear_pixel( uint8_t x, uint8_t y ) {
    uint8_t page = y / 8;
    uint8_t bit = y % 8;

    oled_framebuffer[page * 128 + x] &= ~(1U << bit);
}

void oled_toggle_pixel( uint8_t x, uint8_t y ) {
    uint8_t page = y / 8;
    uint8_t bit = y % 8;

    oled_framebuffer[page * 128 + x] ^= (1U << bit);
}

void oled_update(void) {
    for( uint8_t page = 0; page < 8; page++ )
    {
        oled_set_cursor(page, 0);

        for( uint8_t col = 0; col < 128; col++ )
        {
            oled_data(oled_framebuffer[page * 128 + col]);
        }
    }
}

//DRAW BASIC PRIMITIVES

void oled_draw_hline( uint8_t x, uint8_t y, uint8_t length ) {
    for (uint8_t i = 0; i < length; i++)
    {
        oled_set_pixel(x + i, y);
    }
}

void oled_draw_vline( uint8_t x, uint8_t y, uint8_t length ) {
    for (uint8_t i = 0; i < length; i++)
    {
        oled_set_pixel(x, y + i);
    }
}

void oled_draw_rect( uint8_t x, uint8_t y, uint8_t width, uint8_t height ) { 
    oled_draw_hline( x, y, width );
    oled_draw_hline( x, y + height - 1, width );

    oled_draw_vline( x, y, height );
    oled_draw_vline( x + width - 1, y, height );
}

void oled_draw_rect_filled( uint8_t x, uint8_t y, uint8_t width, uint8_t height ) { 
    for( uint8_t i = 0; i < height; i++ ){
        oled_draw_hline( x, y + i, width );
    }
}

void oled_fill_circle(int16_t xc, int16_t yc, int16_t radius)
{
    for (int16_t y = -radius; y <= radius; y++)
    {
        int16_t x = 0;

        while ((x * x + y * y) <= radius * radius)
        {
            x++;
        }

        oled_draw_hline((xc - x) - 1, yc + y, 2 * x - 1);
    }
}

void oled_draw_line(int16_t x0,
                    int16_t y0,
                    int16_t x1,
                    int16_t y1)
{
    int16_t dx = iabs(x1 - x0);
    int16_t sx = (x0 < x1) ? 1 : -1;

    int16_t dy = iabs(y1 - y0);
    int16_t sy = (y0 < y1) ? 1 : -1;

    int16_t err = dx - dy;

    while (1)
    {
        oled_set_pixel(x0, y0);

        if (x0 == x1 && y0 == y1)
            break;

        int16_t e2 = err * 2;

        if (e2 > -dy)
        {
            err -= dy;
            x0 += sx;
        }

        if (e2 < dx)
        {
            err += dx;
            y0 += sy;
        }
    }
}