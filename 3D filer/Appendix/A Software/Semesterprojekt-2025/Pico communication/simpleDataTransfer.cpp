#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"

#define LED_PIN 25
#define MAX_INPUT_LEN 64

// Blink LED
void blink(int cnt, int delay, int endDelay = 0) {
    for (int i = 0; i < cnt; i++) {
        gpio_put(LED_PIN, 1);
        sleep_ms(delay);
        gpio_put(LED_PIN, 0);
        sleep_ms(delay);
    }
    if (endDelay > 0) {
        sleep_ms(endDelay);
    }
}

int main() {
    stdio_init_all();
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);

    blink(3, 100, 1000);

    char input[MAX_INPUT_LEN];
    int index = 0;

    while (true) {
        // Read from USB serial (stdio) if available
        if (stdio_usb_connected()) {
            char c = getchar();

            if (c == '\n' || c == '\r') {
                //blink(10, 150);
                input[index] = '\0';

                if (strcmp(input, "Open") == 0) {
                    gpio_put(LED_PIN, 1);
                    printf("Ack\n");
                    sleep_ms(5000);
                    printf("Ok\n");
                    gpio_put(LED_PIN, 0);
                }

                index = 0; // Reset for next command
            } else if (index < MAX_INPUT_LEN) {
                input[index++] = c;
            } else {
                index = 0; // Reset if max is reached
            }
        } else {
            blink(1, 50);
        }
        sleep_ms(10);
    }

    return 0;
}
