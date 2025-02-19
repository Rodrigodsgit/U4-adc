#include "ssd1306.h"
#include "font.h"

// Initializes the SSD1306 display with the given parameters
void ssd1306_initialize(ssd1306_t *display, uint8_t width, uint8_t height, bool external_vcc, uint8_t address, i2c_inst_t *i2c) {
    display->width = width;
    display->height = height;
    display->pages = height / 8U; // Calculate the number of pages (each page is 8 pixels tall)
    display->address = address;
    display->i2c_port = i2c;
    display->buffer_size = display->pages * display->width + 1; // Allocate buffer size for the display
    display->buffer = calloc(display->buffer_size, sizeof(uint8_t)); // Allocate memory for the buffer
    display->buffer[0] = 0x40; // Set the first byte to indicate data mode
    display->port_buffer[0] = 0x80; // Set the first byte of the port buffer to indicate command mode
}

// Configures the SSD1306 display with default settings
void ssd1306_configure(ssd1306_t *display) {
    ssd1306_send_command(display, CMD_DISPLAY_ON_OFF | 0x00); // Turn off the display during configuration
    ssd1306_send_command(display, CMD_SET_MEMORY_MODE); // Set memory addressing mode
    ssd1306_send_command(display, 0x01); // Horizontal addressing mode
    ssd1306_send_command(display, CMD_SET_START_LINE | 0x00); // Set display start line to 0
    ssd1306_send_command(display, CMD_SET_SEGMENT_REMAP | 0x01); // Flip the display horizontally
    ssd1306_send_command(display, CMD_SET_MULTIPLEX_RATIO);
    ssd1306_send_command(display, DISPLAY_HEIGHT - 1); // Set multiplex ratio to match display height
    ssd1306_send_command(display, CMD_SET_COM_OUTPUT_SCAN_DIR | 0x08); // Flip the display vertically
    ssd1306_send_command(display, CMD_SET_DISPLAY_OFFSET);
    ssd1306_send_command(display, 0x00); // No offset
    ssd1306_send_command(display, CMD_SET_COM_PINS_CONFIG);
    ssd1306_send_command(display, 0x12); // Alternate COM pin configuration
    ssd1306_send_command(display, CMD_SET_CLOCK_DIVIDE);
    ssd1306_send_command(display, 0x80); // Set default clock divide ratio
    ssd1306_send_command(display, CMD_SET_PRECHARGE_PERIOD);
    ssd1306_send_command(display, 0xF1); // Set pre-charge period
    ssd1306_send_command(display, CMD_SET_VCOM_DESELECT_LEVEL);
    ssd1306_send_command(display, 0x30); // Set VCOM deselect level
    ssd1306_send_command(display, CMD_SET_CONTRAST);
    ssd1306_send_command(display, 0xFF); // Set maximum contrast
    ssd1306_send_command(display, CMD_SET_ENTIRE_ON); // Disable entire display on
    ssd1306_send_command(display, CMD_SET_INVERT); // Disable inverted display
    ssd1306_send_command(display, CMD_SET_CHARGE_PUMP);
    ssd1306_send_command(display, 0x14); // Enable charge pump
    ssd1306_send_command(display, CMD_DISPLAY_ON_OFF | 0x01); // Turn on the display
}

// Sends a single command to the SSD1306 display
void ssd1306_send_command(ssd1306_t *display, uint8_t command) {
    display->port_buffer[1] = command; // Set the command byte
    i2c_write_blocking(
        display->i2c_port,
        display->address,
        display->port_buffer,
        2,
        false
    ); // Send the command via I2C
}

// Sends the entire display buffer to the SSD1306 display
void ssd1306_send_data_to_display(ssd1306_t *display) {
    ssd1306_send_command(display, CMD_SET_COLUMN_ADDR); // Set column address range
    ssd1306_send_command(display, 0); // Start column
    ssd1306_send_command(display, display->width - 1); // End column
    ssd1306_send_command(display, CMD_SET_PAGE_ADDR); // Set page address range
    ssd1306_send_command(display, 0); // Start page
    ssd1306_send_command(display, display->pages - 1); // End page
    i2c_write_blocking(
        display->i2c_port,
        display->address,
        display->buffer,
        display->buffer_size,
        false
    ); // Send the buffer via I2C
}

// Sets or clears a pixel at the specified coordinates
void ssd1306_set_pixel(ssd1306_t *display, uint8_t x, uint8_t y, bool value) {
    uint16_t index = (y >> 3) + (x << 3) + 1; // Calculate the buffer index
    uint8_t pixel = (y & 0b111); // Calculate the bit position within the byte
    if (value)
        display->buffer[index] |= (1 << pixel); // Set the pixel
    else
        display->buffer[index] &= ~(1 << pixel); // Clear the pixel
}

// Fills the entire display with a specific value (on/off)
void ssd1306_clear_display(ssd1306_t *display, bool value) {
    // Iterate through all pixels and set their state
    for (uint8_t y = 0; y < display->height; ++y) {
        for (uint8_t x = 0; x < display->width; ++x) {
            ssd1306_set_pixel(display, x, y, value);
        }
    }
}

// Draws a rectangle on the display
void ssd1306_draw_rectangle(ssd1306_t *display, uint8_t top, uint8_t left, uint8_t width, uint8_t height, bool value, bool fill) {
    // Draw the top and bottom edges
    for (uint8_t x = left; x < left + width; ++x) {
        ssd1306_set_pixel(display, x, top, value);
        ssd1306_set_pixel(display, x, top + height - 1, value);
    }
    // Draw the left and right edges
    for (uint8_t y = top; y < top + height; ++y) {
        ssd1306_set_pixel(display, left, y, value);
        ssd1306_set_pixel(display, left + width - 1, y, value);
    }
    // Fill the rectangle if requested
    if (fill) {
        for (uint8_t x = left + 1; x < left + width - 1; ++x) {
            for (uint8_t y = top + 1; y < top + height - 1; ++y) {
                ssd1306_set_pixel(display, x, y, value);
            }
        }
    }
}

// Draws a line between two points using Bresenham's algorithm
void ssd1306_draw_line(ssd1306_t *display, uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1, bool value) {
    int dx = abs(x1 - x0);
    int dy = abs(y1 - y0);
    int sx = (x0 < x1) ? 1 : -1;
    int sy = (y0 < y1) ? 1 : -1;
    int err = dx - dy;

    while (true) {
        ssd1306_set_pixel(display, x0, y0, value); // Draw the current pixel
        if (x0 == x1 && y0 == y1) break; // Stop when the end point is reached
        int e2 = err * 2;
        if (e2 > -dy) {
            err -= dy;
            x0 += sx;
        }
        if (e2 < dx) {
            err += dx;
            y0 += sy;
        }
    }
}

// Draws a horizontal line
void ssd1306_draw_horizontal_line(ssd1306_t *display, uint8_t x0, uint8_t x1, uint8_t y, bool value) {
    for (uint8_t x = x0; x <= x1; ++x)
        ssd1306_set_pixel(display, x, y, value);
}

// Draws a vertical line
void ssd1306_draw_vertical_line(ssd1306_t *display, uint8_t x, uint8_t y0, uint8_t y1, bool value) {
    for (uint8_t y = y0; y <= y1; ++y)
        ssd1306_set_pixel(display, x, y, value);
}

// Draws a single character on the display
void ssd1306_draw_character(ssd1306_t *display, char c, uint8_t x, uint8_t y) {
    uint16_t index = 0;
    if (c >= 'A' && c <= 'Z') {
        index = (c - 'A' + 11) * 8; // For uppercase letters
    } else if (c >= 'a' && c <= 'z') {
        index = (c - 'a' + 37) * 8; // For lowercase letters
    } else if (c >= '0' && c <= '9') {
        index = (c - '0' + 1) * 8; // For numbers
    }

    for (uint8_t i = 0; i < 8; ++i) {
        uint8_t line = font_data[index + i]; // Get the bitmap line for the character
        for (uint8_t j = 0; j < 8; ++j) {
            ssd1306_set_pixel(display, x + i, y + j, line & (1 << j)); // Draw each pixel
        }
    }
}

// Draws a string on the display
void ssd1306_draw_string(ssd1306_t *display, const char *str, uint8_t x, uint8_t y) {
    while (*str) {
        ssd1306_draw_character(display, *str++, x, y); // Draw each character
        x += 8; // Move to the next character position
        if (x + 8 >= display->width) {
            x = 0; // Wrap to the next line if necessary
            y += 8;
        }
        if (y + 8 >= display->height) {
            break; // Stop if the string exceeds the display height
        }
    }
}