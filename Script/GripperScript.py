import machine
import time

# PWM output pin setup
zero_pin = machine.Pin(18)
pwm_pin = machine.Pin(19)
pwm = machine.PWM(pwm_pin)
pwm.freq(1000)  # 1 kHz frequency
led = machine.Pin("LED", machine.Pin.OUT)  # Onboard LED

# ADC setup on GP26
adc = machine.ADC(26)

zero_pin.value(0)

# Function to map 0-100% to 16-bit duty cycle (0-65535)
def percent_to_duty(pct):
    return int((pct / 100) * 65535)

def blink():
    led.value(1)
    time.sleep(0.1)
    led.value(0)
    time.sleep(0.1)
    led.value(1)
    time.sleep(0.1)
    led.value(0)
    time.sleep(0.1)
    led.value(1)
    time.sleep(0.1)
    led.value(0)
    time.sleep(0.1)

# Open file for writing (will overwrite each time)
with open("data.txt", "w") as file:
    file.write("Stage, PWM_Percent, ADC_Percent\n")
    
    # Stage loop: 6 steps (5% to 30% in 5% intervals)
    for i in range(6):
        blink()

        duty_pct = (i + 1) * 5
        pwm.duty_u16(percent_to_duty(duty_pct))
        print(f"\nStage {i+1} - PWM set to {duty_pct}%")

        time.sleep(90 / (i + 1))
        
        adc_value = adc.read_u16()
        calibrated_value = adc_value / 1.2  # For voltage scaling
        percent_value = (calibrated_value / 65535) * 100  # 0-100% range

        print(f"ADC Reading: {percent_value:.2f}%\n")
        file.write(f"{i+1}, {duty_pct}, {percent_value:.2f}%\n")

# Turn off PWM at end
pwm.duty_u16(0)
print("PWM test complete.")