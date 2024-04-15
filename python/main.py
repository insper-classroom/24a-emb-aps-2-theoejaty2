import serial
import time
from pynput.keyboard import Key, Controller

ser = serial.Serial('/dev/ttyACM0', 115200, timeout=1)
keyboard = Controller()

try:
    print('Listening for key presses...')
    while True:
        char = ser.readline()
        if char:
            char = char.decode('ascii').strip()
            print(f'Pressing {char}')
            if char in ['v', 'b', 'd', 'a', 'w']:
                keyboard.press(char)
                time.sleep(0.1)
                keyboard.release(char)
            else:
                print(f"Received unknown character: {char}")

except KeyboardInterrupt:
    print("Program terminated by user")
except Exception as e:
    print(f"An error occurred: {e}")
finally:
    ser.close()
