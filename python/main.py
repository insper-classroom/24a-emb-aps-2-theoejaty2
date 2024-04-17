import serial
import time
from pynput.keyboard import Key, Controller

# Replace '/dev/ttyACM0' with the correct Bluetooth serial port path
ser = serial.Serial('/dev/rfcomm0', 9600, timeout=1)
keyboard = Controller()

try:
    print('Listening for key presses...')
    while True:
        char = ser.readline()
        if char:
            print(f"Raw input: {char}")  # Adicione esta linha para depuração
            char = char.decode('ascii', errors='ignore').strip()[0]
            if not char:
                continue
            print(f"Pressing {char}")
            if char in ['d', 'a', 'w']:
                keyboard.press(char)
            if char == 'n':
                keyboard.release('d')
                keyboard.release('a')
                keyboard.release('w')
            if char in ['b', 'v']:
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
