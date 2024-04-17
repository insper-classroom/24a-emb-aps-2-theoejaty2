import serial
import time
from pynput.keyboard import Key, Controller

# Replace '/dev/ttyACM0' with the correct Bluetooth serial port path
# Replace '/dev/ttyACM0' with the correct Bluetooth serial port path
ser = serial.Serial('/dev/rfcomm0', 9600, timeout=1)
ser2 = serial.Serial('/dev/rfcomm1', 9600, timeout=1)
#ou /dev/ttyACM0
keyboard = Controller()

try:
    print('Listening for key presses...')
    while True:
        char = ser.readline()
        char2 = ser2.readline()
        if char:
            print(f"Raw input: {char}")  # Adicione esta linha para depuração
            char = char.decode('ascii', errors='ignore').strip()
            if char[0] == 's':
                char = char[1]
            else:
                char = char[0]
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
        if char2:
            print(f"Raw input: {char2}")  # Adicione esta linha para depuração
            char2 = char2.decode('ascii', errors='ignore').strip()
            if char2[0] == 's':
                char2 = char2[1]
            else:
                char2 = char2[0]
            if not char2:
                continue
            print(f"Pressing {char2}")
            if char2 == 'w':
                keyboard.press(Key.up)
            if char2 == 'a':
                keyboard.press(Key.left)
            if char2 == 'd':
                keyboard.press(Key.right)
            if char2 == 'n':
                keyboard.release(Key.left)
                keyboard.release(Key.up)
                keyboard.release(Key.right)    
            if char2 == 'b':
                keyboard.press('l')
                time.sleep(0.1)
                keyboard.release('l')
            if char2 == 'v':
                keyboard.press('k')
                time.sleep(0.1)
                keyboard.release('k')
            else:
                print(f"Received unknown character: {char}")


except KeyboardInterrupt:
    print("Program terminated by user")
except Exception as e:
    print(f"An error occurred: {e}")
finally:
    ser.close()
