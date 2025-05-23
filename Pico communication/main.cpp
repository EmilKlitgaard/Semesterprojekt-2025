#include <stdio.h>
#include <string.h>
#include <vector>
#include <numeric>
#include "pico/stdlib.h"
#include "hardware/adc.h"

#define LED_PIN 25
#define MOTOR_PIN_1 18
#define MOTOR_PIN_2 19
#define ADC_PIN 26
#define MAX_INPUT_LEN 64
#define THRESHOLD 44

void blink(int cnt, int delay, int endDelay = 0) {
    for (int i = 0; i < cnt; i++) {
        gpio_put(LED_PIN, 1);
        sleep_ms(delay);
        gpio_put(LED_PIN, 0);
        sleep_ms(delay);
    }
    if (endDelay > 0) sleep_ms(endDelay);
}

void stopMotor() {
    gpio_put(MOTOR_PIN_1, 0);
    gpio_put(MOTOR_PIN_2, 0);
}

void openGripper() {
    gpio_put(MOTOR_PIN_1, 0);       // Ensure other pin is off
    gpio_put(MOTOR_PIN_2, 1);       // Full speed open
    sleep_ms(1250);                 // Open for 1.25 seconds
    stopMotor();                    // Stop motor
}

void closeGripper() {
    gpio_put(MOTOR_PIN_1, 1);       // Full speed close
    gpio_put(MOTOR_PIN_2, 0);       // Ensure other pin is off

    int itterations = 0;
    std::vector<float> averageList(10, 0.0f);       // Initialize with 10 zeros
    float average = 0.0f;

    while (true) {
        float current = adc_read();                 // raw 12-bit value

        // Update the rolling buffer
        averageList.erase(averageList.begin());     // Remove oldest
        averageList.push_back(current);             // Add newest
        
        // Compute average
        float sum = std::accumulate(averageList.begin(), averageList.end(), 0.0f);
        average = sum / averageList.size();

        float percent = (average / 4095.0f) * 100;  // Converts to 0–100%
        printf("Percent: %.1f%%\n", percent);       // Print with 1 decimal place

        // If current is below threshold, stop the motor and break
        if (percent > THRESHOLD && itterations > 50) {
            stopMotor();
            break;
        }
        itterations++;
        sleep_ms(10);
    }
}

int main() {
    stdio_init_all();
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
    gpio_init(MOTOR_PIN_1);
    gpio_set_dir(MOTOR_PIN_1, GPIO_OUT);
    gpio_init(MOTOR_PIN_2);
    gpio_set_dir(MOTOR_PIN_2, GPIO_OUT);

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
