#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "hardware/pwm.h"

#define LED_PIN 25
#define MOTOR_PIN_1 18
#define MOTOR_PIN_2 19
#define ADC_PIN 26

#define MAX_INPUT_LEN 64
#define PWM_WRAP 10000
#define CURRENT_THRESHOLD 0.2f // 0.2 A (adjust as needed)
#define CURRENT_TOLERANCE 0.05f // +/- tolerance

void blink(int cnt, int delay, int endDelay = 0) {
    for (int i = 0; i < cnt; i++) {
        gpio_put(LED_PIN, 1);
        sleep_ms(delay);
        gpio_put(LED_PIN, 0);
        sleep_ms(delay);
    }
    if (endDelay > 0) sleep_ms(endDelay);
}

// Convert ADC value to current (0-1A)
float read_current() {
    uint16_t raw = adc_read();
    return (raw / 4095.0f) * 1.0f;
}

void init_pwm(unsigned short pin) {
    gpio_set_function(pin, GPIO_FUNC_PWM);
    unsigned int slice = pwm_gpio_to_slice_num(pin);
    pwm_set_wrap(slice, PWM_WRAP);
    pwm_set_enabled(slice, true);
}

void set_pwm(unsigned short pin, unsigned int duty) {
    pwm_set_gpio_level(pin, duty);
}

void stopMotor() {
    set_pwm(MOTOR_PIN_1, 0);
    set_pwm(MOTOR_PIN_2, 0);
}

void openGripper() {
    set_pwm(MOTOR_PIN_1, 0);        // Ensure other pin is off
    set_pwm(MOTOR_PIN_2, PWM_WRAP); // Full speed open
    sleep_ms(5000);                 // Open for 5 seconds
    stopMotor();                    // Stop motor
}

void closeGripper() {
    unsigned int duty = PWM_WRAP;
    
    set_pwm(MOTOR_PIN_2, 0);        // Ensure other pin is off
    set_pwm(MOTOR_PIN_1, duty);     // Full speed close
    sleep_ms(3000);                 // Close for 3 seconds
    
    duty /= 10;

    while (true) {
        float current = read_current();

        // Adjust PWM based on the current feedback
        if (current > CURRENT_THRESHOLD + CURRENT_TOLERANCE) {
            if (duty >= 5) duty -= 5;
        } else if (current < CURRENT_THRESHOLD - CURRENT_TOLERANCE) {
            if (duty <= PWM_WRAP - 5) duty += 5;
        }

        // Set the new PWM duty
        set_pwm(MOTOR_PIN_1, duty);

        // If current is within the threshold, exit loop
        if (current >= CURRENT_THRESHOLD - CURRENT_TOLERANCE && current <= CURRENT_THRESHOLD + CURRENT_TOLERANCE) {
            break;
        }

        sleep_ms(10);
    }
}


int main() {
    stdio_init_all();
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);

    init_pwm(MOTOR_PIN_1);
    init_pwm(MOTOR_PIN_2);
    stopMotor();

    adc_init();
    adc_gpio_init(ADC_PIN);
    adc_select_input(0);
    
    blink(3, 100, 1000);

    char input[MAX_INPUT_LEN];
    int index = 0;

    while (true) {
        if (stdio_usb_connected()) {
            char c = getchar();
            if (c == '\n' || c == '\r') {
                input[index] = '\0';
                if (strcmp(input, "Open") == 0) {
                    gpio_put(LED_PIN, 1);
                    printf("Ack\n");
                    openGripper();
                    printf("Ok\n");
                    gpio_put(LED_PIN, 0);
                } else if (strcmp(input, "Close") == 0) {
                    gpio_put(LED_PIN, 1);
                    printf("Ack\n");
                    closeGripper();
                    printf("Ok\n");
                    gpio_put(LED_PIN, 0);
                }
                else if (strcmp(input, "Stop") == 0) {
                    blink(3, 100);
                    gpio_put(LED_PIN, 1);
                    stopMotor();
                    gpio_put(LED_PIN, 0);
                }
                index = 0;
            } else if (index < MAX_INPUT_LEN - 1) {
                input[index++] = c;
            } else {
                index = 0;
            }
        } else {
            blink(1, 50);
        }
        sleep_ms(10);
    }
    return 0;
}
