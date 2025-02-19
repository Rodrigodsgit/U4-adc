#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/bootrom.h"
#include "pico/time.h"
#include "inc/ssd1306.h"
#include "hardware/i2c.h"
#include "inc/font.h"
#include "string.h"
#include "hardware/pwm.h"
#include "hardware/adc.h"

// Definitions for the integrated OLED display
#define I2C_PORT i2c1
#define I2C_SDA 14
#define I2C_SCL 15
#define address 0x3C

// Default parameters for the joystick
#define DEFAULT_VRX 1900
#define DEFAULT_VRY 2000
// Maximum counter value - WRAP
#define WRAP_PERIOD  4096
// Clock divisor for the PWM 
#define PWM_DIVISOR  25.4313151f 

// Peripheral pins
const uint8_t LED_RED = 13, LED_BLUE = 12, LED_GREEN = 11;
const uint8_t BUTTON_A = 5, BUTTON_B = 6, JOYSTICK_BUTTON = 22;
const uint8_t JOYSTICK_X = 26, JOYSTICK_Y = 27;
// Variable for time recording and debounce control in the interrupt
static volatile uint32_t previous_time = 0;

// LED control variables
static volatile bool green_led_active = false, pwm_active = true;
static volatile bool border_type = true;
// ADC reading variables
static volatile uint16_t x_value, y_value;
// Initialize the display structure
ssd1306_t ssd; 

// Function prototypes
void initialize_leds();
void initialize_buttons();
void initialize_joystick();
void read_adc();
void get_pwm_level();
void update_display();
static void gpio_interrupt_handler(uint gpio, uint32_t events);
void initialize_oled_display();
void setup_pwm(uint8_t PIN);
void set_pwm_duty_cycle(uint16_t duty_cycle, uint8_t PIN);
void position_square();

int main()
{
    // Initialization of peripherals
    stdio_init_all();
    initialize_leds();
    initialize_buttons();
    initialize_joystick();
    initialize_oled_display();
    adc_init();

    // Configure interrupts
    gpio_set_irq_enabled_with_callback(BUTTON_A, GPIO_IRQ_EDGE_FALL, true, &gpio_interrupt_handler);
    gpio_set_irq_enabled_with_callback(BUTTON_B, GPIO_IRQ_EDGE_FALL, true, &gpio_interrupt_handler);
    gpio_set_irq_enabled_with_callback(JOYSTICK_BUTTON, GPIO_IRQ_EDGE_FALL, true, &gpio_interrupt_handler);

    while (true) {
        read_adc();
        get_pwm_level();
        update_display();
        printf("Joystick reading!\n");
        printf("X Axis: %d  Y Axis: %d\n", x_value, y_value);
        sleep_ms(1000);
    }
}

/*
|   Function initialize_leds
|   Configures the RGB LED pins as outputs
*/
void initialize_leds(){
    // Configure the red and blue LEDs as PWM
    setup_pwm(LED_RED);
    setup_pwm(LED_BLUE);
    // Configure the green LED as a standard output
    gpio_init(LED_GREEN);
    gpio_set_dir(LED_GREEN, GPIO_OUT);
}

/*
|   Function initialize_buttons
|   Configures the BUTTON_A, BUTTON_B, and joystick button as inputs with pull-up mode
*/
void initialize_buttons(){
    // Button A
    gpio_init(BUTTON_A);
    gpio_set_dir(BUTTON_A, GPIO_IN);
    gpio_pull_up(BUTTON_A);
    // Button B
    gpio_init(BUTTON_B);
    gpio_set_dir(BUTTON_B, GPIO_IN);
    gpio_pull_up(BUTTON_B);
}

/*
|   Function initialize_joystick
|   Initializes the joystick pins as ADC inputs and configures the joystick button
*/
void initialize_joystick(){
    // Initialize joystick pins as ADC channels
    adc_gpio_init(JOYSTICK_X);
    adc_gpio_init(JOYSTICK_Y);
    // Joystick button
    gpio_init(JOYSTICK_BUTTON);
    gpio_set_dir(JOYSTICK_BUTTON, GPIO_IN);
    gpio_pull_up(JOYSTICK_BUTTON);
}

/*
|   Function initialize_oled_display
|   Configures and initializes the SSD1306 OLED display for use.
|   Communication is done via I2C.
*/
void initialize_oled_display(){
    // I2C Initialization at 400KHz.
    i2c_init(I2C_PORT, 400 * 1000);

    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C); // Set the GPIO pin function to I2C
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C); // Set the GPIO pin function to I2C
    gpio_pull_up(I2C_SDA); // Pull up the data line
    gpio_pull_up(I2C_SCL); // Pull up the clock line
    //ssd1306_t ssd; // Initialize the display structure
    ssd1306_initialize(&ssd, DISPLAY_WIDTH, DISPLAY_HEIGHT, false, address, I2C_PORT); // Initialize the display
    ssd1306_configure(&ssd); // Configure the display
    ssd1306_send_data_to_display(&ssd); // Send data to the display

    // Clear the display. The display starts with all pixels off.
    ssd1306_clear_display(&ssd, false);
    ssd1306_send_data_to_display(&ssd);
}

/*
|   Function gpio_interrupt_handler
|   Callback function for GPIO interrupt handling.
|   Implements software debouncing and controls the state of the blue and green LEDs,
|   in addition to activating the bootsel mode.
*/
static void gpio_interrupt_handler(uint gpio, uint32_t events){
    // Get the current time since boot
    uint32_t current_time = to_us_since_boot(get_absolute_time());
    // If the button has been pressed for at least 200ms
    if(current_time - previous_time > 200000){
        previous_time = current_time;
        // Handle the interrupt
        if(gpio == BUTTON_A){
            printf("Toggle PWM LED state\n");
            pwm_active = !pwm_active;
            if(!pwm_active){
                set_pwm_duty_cycle(0, LED_RED);
                set_pwm_duty_cycle(0, LED_BLUE);
            }
        }
        else if(gpio == BUTTON_B){
            printf("Bootsel mode\n");
            reset_usb_boot(0, 0);
        }
        else if(gpio == JOYSTICK_BUTTON){
            printf("Toggle green LED state:");
            green_led_active = !green_led_active;
            printf("LED state: %d\n", green_led_active);
            gpio_put(LED_GREEN, green_led_active);
            // Toggle border type
            border_type = !border_type;
            update_display();
        }
    }
}

/*
|   Function setup_pwm
|   Configures the PWM module for the specified pin.
*/
void setup_pwm(uint8_t PIN)
{
    // With a base clock of 125MHz, a WRAP of 4096 and a divisor of PWM_DIVISOR,
    // the calculated PWM frequency is 50Hz.
    // Other parameter configurations could achieve the same frequency,
    // but task requirements limited the wrap value selection.

    gpio_set_function(PIN, GPIO_FUNC_PWM); // Enable the GPIO pin as PWM

    uint slice = pwm_gpio_to_slice_num(PIN); // Get the PWM channel for the GPIO

    pwm_set_wrap(slice, WRAP_PERIOD); // Set the wrap value
    pwm_set_clkdiv(slice, PWM_DIVISOR); // Set the PWM clock divisor
    pwm_set_enabled(slice, true); // Enable PWM on the corresponding slice
}

/*
|   Function set_pwm_duty_cycle
|   Sets the PWM duty cycle for the specified pin.
*/
void set_pwm_duty_cycle(uint16_t duty_cycle, uint8_t PIN){
    uint slice = pwm_gpio_to_slice_num(PIN);
    pwm_set_gpio_level(PIN, duty_cycle); // Set the PWM duty cycle
}

/*
|   Function read_adc
|   Reads the ADC values for the joystick's X and Y axes.
*/
void read_adc(){
    // Read channel 0 - joystick X axis
    adc_select_input(0);
    sleep_us(2);
    x_value = adc_read();
    // Read channel 1 - joystick Y axis
    adc_select_input(1);
    sleep_us(2);
    y_value = adc_read();
}

/*
|   Function get_pwm_level
|   Calculates the PWM level based on the joystick value.
*/
void get_pwm_level(){
    uint16_t pwm_x, pwm_y;
    if(x_value >= DEFAULT_VRX - 200 && x_value <= DEFAULT_VRX + 200){
        set_pwm_duty_cycle(0, LED_RED);
    }
    else{
        // Calculate the level
        pwm_x = (x_value > DEFAULT_VRX) ? x_value - DEFAULT_VRX : DEFAULT_VRX - x_value;
        // Double the value
        pwm_x = pwm_x * 2;
    }

    if(y_value >= DEFAULT_VRY - 200 && y_value <= DEFAULT_VRY + 200){
        set_pwm_duty_cycle(0, LED_BLUE);
    }
    else{
        // Calculate the level
        pwm_y = (y_value > DEFAULT_VRY) ? y_value - DEFAULT_VRY : DEFAULT_VRY - y_value;
        // Double the value
        pwm_y = pwm_y * 2;
    }
    if(pwm_active){
        set_pwm_duty_cycle(pwm_x, LED_RED);
        set_pwm_duty_cycle(pwm_y, LED_BLUE);
    }
    sleep_ms(500);
}

/*
|   Function update_display
|   Clears the display, draws the border and the square according to joystick values,
|   and sends the updated data to the display.
*/
void update_display(){
    // Clear the display
    ssd1306_clear_display(&ssd, false);
    if(border_type){
        // Draw border with a thin rectangle
        ssd1306_draw_rectangle(&ssd, 0, 0, 127, 60, true, false);
    }
    else{
        // Draw border with two rectangles
        ssd1306_draw_rectangle(&ssd, 3, 3, 120, 60, true, false);
        ssd1306_draw_rectangle(&ssd, 6, 6, 115, 55, true, false);
    }

    float x_pos, y_pos;

    x_pos = (x_value / 4080.0) * 119.0;
    y_pos = (y_value / 4080.0) * 55.0;

    // Correction factor for coordinates
    if(x_pos <= 8)
        x_pos += 8;
    if(y_pos <= 9)
        y_pos += 8;
    printf("Square coordinates\n");
    printf("X: %.2f  Y: %.2f\n", x_pos, y_pos);

    ssd1306_draw_rectangle(&ssd, (int)y_pos, (int)x_pos, 8, 8, true, true);
    
    // Send data to the display
    ssd1306_send_data_to_display(&ssd);
}
