//this is the code for making the the grapper grap shit


#include "pico/stdlib.h"
#include "hardware/gpio.h"

//#include "hardware/irq.h"

#define LED_PIN  25
#define LED_PIN_red  22
#define LED_PIN_green  26
#define LED_PIN_white  27

#define BUTTON_PIN 16

#define NUM_LEDS 4  // Number of LEDs
#define NUM_BUTTONS 1 // Number of Buttons

const uint LED_PINS[NUM_LEDS] = {25, 22, 26, 27};  // LED GPIO pins
const uint BUTTON_PINS[NUM_BUTTONS] = {16}; // Button GPIO pins

volatile bool led_state = false;  // Track LED state
volatile bool led_states = true;  //Track to turn the led's on or off.

volatile int state = 0; //number to keep track of which if loops should rund in button interrupts (button_isr)
volatile uint32_t last_press_time = 0; //timestamp for debounce:


void button_isr(uint gpio, uint32_t events) {

    uint32_t current_time = to_ms_since_boot(get_absolute_time());
    
    if(current_time - last_press_time < 200) {
        return;
    }

    if (state>2){
        state=0;
        led_states = !led_states;
    }

    last_press_time = current_time;

    if (state==0){
        gpio_put(LED_PIN_red, led_states);
    }
    if (state == 1){
        gpio_put(LED_PIN_green, led_states);
    }
    if (state == 2) {
        gpio_put(LED_PIN_white, led_states);
    }
    state++;
}



int main() {
    volatile bool led_state_onboard = false;
    stdio_init_all();

    for(int i = 0; i<NUM_LEDS;i++){
        gpio_init(LED_PINS[i]);
        gpio_set_dir(LED_PINS[i],GPIO_OUT);
        gpio_put(LED_PINS[i],false);
    }

    for(int i = 0; i < NUM_BUTTONS; i++){
        gpio_init(BUTTON_PINS[i]);
        gpio_set_dir(BUTTON_PINS[i],GPIO_IN);
        gpio_pull_up(BUTTON_PINS[i]);
        gpio_set_irq_enabled_with_callback(BUTTON_PINS[i],GPIO_IRQ_EDGE_FALL, true, &button_isr);
    }

    //loop forever
    while (true) {
        
        gpio_put(LED_PIN, led_state_onboard);
        led_state_onboard = !led_state_onboard;
        
        sleep_ms(1000);
    }

    return 0;
    
}

