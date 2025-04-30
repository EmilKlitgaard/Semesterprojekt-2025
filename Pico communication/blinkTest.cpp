#include "pico/stdlib.h"

#define LED_PIN 25

int main() {
    stdio_init_all();

    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);

    while (1) {
        gpio_put(LED_PIN, 1);  // Turn on
        sleep_ms(500);
        gpio_put(LED_PIN, 0);  // Turn off
        sleep_ms(500);
    }

    return 0;
}
