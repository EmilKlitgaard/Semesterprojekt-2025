//this is the code for making the the grapper grap shit


#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/pwm.h"
#include <stdio.h>
#include "hardware/adc.h"




#include "hardware/irq.h"

#define LED_PIN  25
#define LED_PIN_red  12
#define LED_PIN_green  11
#define LED_PIN_white  10

#define BUTTON_PIN 16

#define NUM_LEDS 4  // Number of LEDs
#define NUM_BUTTONS 1 // Number of Buttons

const uint LED_PINS[NUM_LEDS] = {25, 12, 11, 10};  // LED GPIO pins
const uint BUTTON_PINS[NUM_BUTTONS] = {16}; // Button GPIO pins

volatile bool led_state = false;  // Track LED state
volatile bool led_states = true;  //Track to turn the led's on or off.

volatile int state = 0; //number to keep track of which if loops should rund in button interrupts (button_isr)
volatile uint32_t last_press_time = 0; //timestamp for debounce:

//pwm
#define LED_PIN_green_pwm 5




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
    volatile bool going_up = true;
    stdio_init_all();
    adc_init();

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


    
    //pwm 
    gpio_set_function(LED_PIN_green_pwm, GPIO_FUNC_PWM);
    uint slice_num = pwm_gpio_to_slice_num(LED_PIN_green_pwm);
    // Set period of x number of cycles
    pwm_set_wrap(slice_num, 1000);
    // Set channel A output high for x number of cycles before dropping
    //pwm_set_chan_level(slice_num, PWM_CHAN_A, 1);
    // Set initial B output high for x number of cycles before dropping 
    //pwm_set_chan_level(slice_num, PWM_CHAN_B, 500);
    // Set the PWM running
    pwm_set_enabled(slice_num, true);
    /// \end::setup_pwm[]

    uint pwm_up_and_down = 0;
  

    //ADC
    // Make sure GPIO is high-impedance, no pullups etc
    adc_gpio_init(26);
    // Select ADC input 0 (GPIO26)
    adc_select_input(0);

    //loop forever
    while (true) {
        
        gpio_put(LED_PIN, led_state_onboard);
        led_state_onboard = !led_state_onboard;

        if (going_up == true){
            printf("Going up!! PWM is set to: %d\n", pwm_up_and_down);
            pwm_up_and_down += 100;
            if (pwm_up_and_down >= 1000){
                pwm_up_and_down = 1000;
                going_up = false;
                printf("Changing from up to down\n");
            }
        }
        if (going_up == false){
            pwm_up_and_down -= 100;
            printf("Going down!! PWM is set to: %d\n", pwm_up_and_down);
            if (pwm_up_and_down <= 0){
                pwm_up_and_down = 0;
                going_up = true;
                printf("Changing from down to up\n");
            }
        }

        pwm_set_chan_level(slice_num, PWM_CHAN_B, pwm_up_and_down);

        //ADC
        //12-bit conversion, assume max value == ADC_VREF == 3.3 V
        const float conversion_factor = 3.3f / (1 << 12);
        uint16_t result = adc_read();
        printf("Raw value: 0x%03x, voltage: %f V\n", result, result * conversion_factor);
        
        
        sleep_ms(200);
    }

    return 0;
    
}

