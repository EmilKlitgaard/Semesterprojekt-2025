//this is the code for making the the grapper grap shit


#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/pwm.h"
#include <stdio.h>
#include "hardware/adc.h"
#include "hardware/irq.h"

#define LED_PIN_green_1 5
#define LED_PIN_green_2 13
#define LED_PIN_white 2
#define LED_PIN_red 14

//#define BUTTON_PIN 16 //defining buttons.

#define NUM_LEDS 4  // Number of LEDs
//#define NUM_BUTTONS 1 // Number of Buttons

const uint LED_PINS[NUM_LEDS] = {25, 12, 11, 10};  // LED GPIO pins
//const uint BUTTON_PINS[NUM_BUTTONS] = {16}; // Button GPIO pins


volatile int state = 0; //number to keep track of which if loops should rund in button interrupts (button_isr)
volatile uint32_t last_press_time = 0; //timestamp for debounce:




void button_isr(uint gpio, uint32_t events) {

    uint32_t current_time = to_ms_since_boot(get_absolute_time()); //tracking of time to prevent multiple input on a  single push.
    
    if(current_time - last_press_time < 200) { 
        return;
    }

}


int main() {

    stdio_init_all();
    adc_init();
    sleep_ms(2000);  // Allow time for USB initialization


    int timer = 2000;

    uint PWM = 0; //initialize pwm. 
    const int PWM_drive = 1000; //change accordingly
    const int PWM_hold = 300; //change accordingly
    const int ADC_valForHold = 1.7; //value used for tracking when grapper has hold on pawn. Change accordingly. betweeen 0-3,3 v. 

    //grapperstate = 1 is pickup, grapperstate = 2 is hold, grapperstate 3 is dropoff.
    int grapper_state = 0; //initialize grapper_state variable. 
    
    //SETUP for adc on pico to read sensor value:

    //function to initialize PINS. 
    for(int i = 0; i<NUM_LEDS;i++){ 
        gpio_init(LED_PINS[i]);
        gpio_set_dir(LED_PINS[i],GPIO_OUT);
        gpio_put(LED_PINS[i],false);
    }
/*
    for(int i = 0; i < NUM_BUTTONS; i++){
        gpio_init(BUTTON_PINS[i]);
        gpio_set_dir(BUTTON_PINS[i],GPIO_IN);
        gpio_pull_up(BUTTON_PINS[i]);
        gpio_set_irq_enabled_with_callback(BUTTON_PINS[i],GPIO_IRQ_EDGE_FALL, true, &button_isr);
    }
*/

    
    //pwm for simulating grapper contracting
    gpio_set_function(LED_PIN_green_1, GPIO_FUNC_PWM);
    uint slice_green_1 = pwm_gpio_to_slice_num(LED_PIN_green_1);
    gpio_set_function(LED_PIN_white, GPIO_FUNC_PWM);
    uint slice_white = pwm_gpio_to_slice_num(LED_PIN_white);

    //pwm for simulating grapper releasing
    gpio_set_function(LED_PIN_green_2, GPIO_FUNC_PWM);
    uint slice_green_2 = pwm_gpio_to_slice_num(LED_PIN_green_2);
    gpio_set_function(LED_PIN_red, GPIO_FUNC_PWM);
    uint slice_red = pwm_gpio_to_slice_num(LED_PIN_red);


    // Set period of x number of cycles
    pwm_set_wrap(slice_green_1, 1000);
    pwm_set_wrap(slice_white, 1000);
    pwm_set_wrap(slice_green_2, 1000);
    pwm_set_wrap(slice_red, 1000);
    // Set channel A output high for x number of cycles before dropping
    //pwm_set_chan_level(slice_num, PWM_CHAN_A, 1);
    // Set initial B output high for x number of cycles before dropping 
    //pwm_set_chan_level(slice_num, PWM_CHAN_B, 500);
    // Set the PWM running
    pwm_set_enabled(slice_green_1, true);
    pwm_set_enabled(slice_white, true);
    pwm_set_enabled(slice_green_2, true);
    pwm_set_enabled(slice_red, true);
    /// \end::setup_pwm[]

    //ADC
    // Make sure GPIO is high-impedance, no pullups etc
    adc_gpio_init(26);
    // Select ADC input 0 (GPIO26)
    adc_select_input(0);

    //loop forever
    while (true) {
        //ADC
        //12-bit conversion, assume max value == ADC_VREF == 3.3 V
        const float conversion_factor = 3.3f / (1 << 12);
        float ADC_val = adc_read()*conversion_factor;
        uint32_t current_time = to_ms_since_boot(get_absolute_time());

       if(grapper_state ==1){ //grapper state 1 is pickup. 
            printf(" \n)");
            printf("---------------Pickup------------\n");

            current_time = to_ms_since_boot(get_absolute_time());

            ADC_val= adc_read()*conversion_factor;
            if(PWM != PWM_drive){
                PWM = PWM_drive;
                //set pwm on pinouts. Its set only once, as PWM is set to PWM_drive. This way we dont repeatitly set the PWM.
                pwm_set_chan_level(slice_green_1, PWM_CHAN_B, PWM);
                pwm_set_chan_level(slice_white, PWM_CHAN_A, PWM);
                pwm_set_chan_level(slice_green_2, PWM_CHAN_B, 0);
                pwm_set_chan_level(slice_red, PWM_CHAN_A, 0);
                printf("--------------Grapping--------------\n");
            }
            
            while(ADC_val<ADC_valForHold){
                ADC_val = adc_read()*conversion_factor;
                printf("While loop in grapperstate 1\n");
                
                if(ADC_val>3.3 || ADC_val< 0){
                    printf("ADC_val invalid reading, setting ADC_val = 0");
                    ADC_val =0;
                }
                printf("ADC_val = %.2f\n",ADC_val); //%.2f limits to two decimals
                sleep_ms(1000);
            } 

            printf("ADC_val reached, switching to grapper_state 2 \n");
            grapper_state =2;
            
       }

       if(grapper_state==2){ //grapper state 2 is hold
            printf(" \n)");
            printf("-------------Holding------------\n");

            if(PWM != PWM_hold){
                PWM = PWM_hold;
                //set pwm on pinouts. Its set only once, as PWM is set to PWM_drive. This way we dont repeatitly set the PWM.
                pwm_set_chan_level(slice_green_1, PWM_CHAN_B, PWM);
                pwm_set_chan_level(slice_white, PWM_CHAN_A, PWM);
                pwm_set_chan_level(slice_green_2, PWM_CHAN_B, 0);
                pwm_set_chan_level(slice_red, PWM_CHAN_A, 0);
                printf("GrapperPWM set to hold\n");

            }
            printf("----------------Waiting for input from computer (d for dropoff)-------------- \n");

            // Blocking read: Wait until input is received
            int USBinput = getchar();
            
            printf("USB input received: %c (%d)\n", USBinput, USBinput);

            
            if (USBinput == 'd') {
                grapper_state = 3;  // Start dropoff
                printf("Input 'd' received, grapper_state set to 3\n");
            }
       }

       if(grapper_state==3){
            printf(" \n)");
            printf("---------------Dropoff-------------\n");
            current_time = to_ms_since_boot(get_absolute_time());
            if(PWM!= PWM_drive){
                PWM = PWM_drive;
                pwm_set_chan_level(slice_green_1, PWM_CHAN_B, 0);
                pwm_set_chan_level(slice_white, PWM_CHAN_A, 0);
                pwm_set_chan_level(slice_green_2, PWM_CHAN_B, PWM);
                pwm_set_chan_level(slice_red, PWM_CHAN_A, PWM);
                printf("In State 3 / realeasing: just set the PWM's to go backwards\n");
            }
            sleep_ms(timer);
            grapper_state = 0;
       } 

        if (grapper_state != 1 && grapper_state != 2 && grapper_state != 3){ //if none of the above states is active:
            printf(" \n)");
            printf("-----------Grapperstate default-----------\n)");
            PWM = 0;
            pwm_set_chan_level(slice_green_1, PWM_CHAN_B, PWM);
            pwm_set_chan_level(slice_white, PWM_CHAN_A, PWM);
            pwm_set_chan_level(slice_green_2, PWM_CHAN_B, PWM);
            pwm_set_chan_level(slice_red, PWM_CHAN_A, PWM);

            printf("In state default: set PWM to zero and scans for inputs\n");
            
            
        
            sleep_ms(1000);
            printf("ADC reading is %.2f\n",ADC_val); //%.2f limits to two decimals
            printf("Waiting for input: p for pickup");
            
            sleep_ms(5000);

            int USBinput = getchar();
            printf("USB input received: %c (%d)\n", USBinput, USBinput);
            if (USBinput == 'p') {
                grapper_state = 1;  // Start pickup
                printf("Input 'p' received, grapper_state set to 1\n");
            }
            
        }
        sleep_ms(100);
        
    }
return 0;
}

