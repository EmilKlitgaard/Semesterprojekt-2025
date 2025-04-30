import machine
import time
import sys
import select

# PWM output pin setup
zero_pin = machine.Pin(18)
pwm_pin = machine.Pin(19)
pwm = machine.PWM(pwm_pin)
pwm.freq(1000)  # 1 kHz frequency
led = machine.Pin("LED", machine.Pin.OUT)  # Onboard LED

# ADC setup on GP26
adc = machine.ADC(26)

zero_pin.value(0)
pwm_percent = 5  # Start at 5%
running = True

# Helper functions
def percent_to_duty(pct):
    return int((pct / 100) * 65535)

def blink():
    for _ in range(3):
        led.value(1)
        time.sleep(0.1)
        led.value(0)
        time.sleep(0.1)

def adc_to_current(adc_val):
    voltage = (adc_val / 65535) * 3.3  # Assuming 3.3V reference
    shunt_resistor = 0.1  # Example: 0.1 Ohm
    current = voltage / shunt_resistor
    return current

# Non-blocking input (platform dependent)
def input_available():
    return select.select([sys.stdin], [], [], 0)[0]

# Open file
with open("data.csv", "w") as file:
    file.write("Time_s,PWM_Percent,Current_A\n")
    start_time = time.time()

    blink()
    print("Controls: 'u' = up, 'd' = down, 'q' = quit\n")

    while running:
        # Update PWM
        pwm.duty_u16(percent_to_duty(pwm_percent))

        # Read ADC and compute current
        adc_value = adc.read_u16()
        current = adc_to_current(adc_value)

        # Time since start
        elapsed = time.time() - start_time

        # Write to file
        file.write(f"{elapsed:.1f},{pwm_percent},{current:.3f}\n")
        file.flush()

        print(f"Time: {elapsed:.1f}s | PWM: {pwm_percent}% | Current: {current:.3f} A")

        # Handle input
        if input_available():
            char = sys.stdin.read(1)
            if char == 'u' and pwm_percent < 100:
                pwm_percent += 5
            elif char == 'd' and pwm_percent > 0:
                pwm_percent -= 5
            elif char == 'q':
                running = False
                print("Exiting...")

        time.sleep(1)

# Cleanup
pwm.duty_u16(0)
print("PWM turned off. Logging complete.")
