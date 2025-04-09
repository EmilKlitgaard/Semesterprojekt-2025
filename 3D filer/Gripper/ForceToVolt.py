from machine import Pin, PWM
import utime

# Set the desired duty cycle (in percent)
pwm_duty_cycle = 20
pwm_max_value = 65535
# Calculate the PWM duty value corresponding to pwm_duty_cycle percent.
pwm_duty_val = int(pwm_max_value * pwm_duty_cycle / 100)

# Initialize PWM on Pin 2 with a frequency of 1000 Hz.
pwm_pin = PWM(Pin(2))
pwm_pin.freq(1000)

# Set the duty cycle using the calculated value.
pwm_pin.duty_u16(pwm_duty_val)

# Let the PWM signal run for 5 seconds.
utime.sleep(3600)

# Clean up by deinitializing the PWM output.
pwm_pin.deinit()