//this is the code for making the the grapper grap shit

#include "pico/stdlib.h"

int main() {
    const uint led_pin = 25;

    //initialize LED pin 
    gpio_init(led_pin);
    gpio_set_dir(led_pin,GPIO_OUT);

    //loop forever
    while(true) {

        //blink LED
        gpio_put(led_pin,true);
        sleep_ms(200);
        gpio_put(led_pin,false);
        sleep_ms(200);
    }
}

