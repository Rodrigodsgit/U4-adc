#ifndef SSD1306_H
#define SSD1306_H

#include <stdint.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"

// Display dimensions
#define DISPLAY_WIDTH 128
#define DISPLAY_HEIGHT 64

// SSD1306 commands
typedef enum {
    CMD_SET_CONTRAST = 0x81,
    CMD_SET_ENTIRE_ON = 0xA4,
    CMD_SET_INVERT = 0xA6,
    CMD_DISPLAY_ON_OFF = 0xAE,
    CMD_SET_MEMORY_MODE = 0x20,
    CMD_SET_COLUMN_ADDR = 0x21,
    CMD_SET_PAGE_ADDR = 0x22,
    CMD_SET_START_LINE = 0x40,
    CMD_SET_SEGMENT_REMAP = 0xA0,
    CMD_SET_MULTIPLEX_RATIO = 0xA8,
    CMD_SET_COM_OUTPUT_SCAN_DIR = 0xC0,
    CMD_SET_DISPLAY_OFFSET = 0xD3,
    CMD_SET_COM_PINS_CONFIG = 0xDA,
    CMD_SET_CLOCK_DIVIDE = 0xD5,
    CMD_SET_PRECHARGE_PERIOD = 0xD9,
    CMD_SET_VCOM_DESELECT_LEVEL = 0xDB,
    CMD_SET_CHARGE_PUMP = 0x8D
} ssd1306_command_t;

// SSD1306 display structure
typedef struct {
    uint8_t width;          // Display width
    uint8_t height;         // Display height
    uint8_t pages;          // Number of pages (height / 8)
    uint8_t address;        // I2C address
    i2c_inst_t *i2c_port;   // I2C instance
    bool external_vcc;      // External VCC flag
    uint8_t *buffer;        // Display buffer
    size_t buffer_size;     // Buffer size
    uint8_t port_buffer[2]; // Temporary buffer for I2C communication
} ssd1306_t;

// Function prototypes
void ssd1306_initialize(ssd1306_t *display, uint8_t width, uint8_t height, bool external_vcc, uint8_t address, i2c_inst_t *i2c);
void ssd1306_configure(ssd1306_t *display);
void ssd1306_send_command(ssd1306_t *display, uint8_t command);
void ssd1306_send_data_to_display(ssd1306_t *display);
void ssd1306_set_pixel(ssd1306_t *display, uint8_t x, uint8_t y, bool value);
void ssd1306_clear_display(ssd1306_t *display, bool value);
void ssd1306_draw_rectangle(ssd1306_t *display, uint8_t top, uint8_t left, uint8_t width, uint8_t height, bool value, bool fill);
void ssd1306_draw_line(ssd1306_t *display, uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1, bool value);
void ssd1306_draw_horizontal_line(ssd1306_t *display, uint8_t x0, uint8_t x1, uint8_t y, bool value);
void ssd1306_draw_vertical_line(ssd1306_t *display, uint8_t x, uint8_t y0, uint8_t y1, bool value);
void ssd1306_draw_character(ssd1306_t *display, char c, uint8_t x, uint8_t y);
void ssd1306_draw_string(ssd1306_t *display, const char *str, uint8_t x, uint8_t y);

#endif // SSD1306_H