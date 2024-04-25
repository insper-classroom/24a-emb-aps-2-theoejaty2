import serial
import time
from pynput.keyboard import Key, Controller
import threading

# Substitua '/dev/ttyACM0' pelo caminho correto do portão serial Bluetooth
ser1 = serial.Serial('/dev/rfcomm0', 9600, timeout=1)
ser2 = serial.Serial('/dev/rfcomm1', 9600, timeout=1)
keyboard = Controller()

def handle_serial(ser, device_num):
    try:
        while True:
            char = ser.readline()
            if char:
                char = char.decode('ascii', errors='ignore').strip()
                # Encontra o primeiro caractere válido dentre ['w', 'a', 'd', 'b', 'v']
                found_char = next((c for c in char if c in ['w', 'a', 'd', 'b', 'v', 'n']), None)
                if not found_char:
                    continue
                char = found_char  # Agora 'char' é o primeiro caractere válido encontrado
                print(f"Device {device_num}: Pressing {char}")

                if device_num == 1:
                    # Aqui você define as ações para ser1
                    if char in ['d', 'a', 'w']:
                        keyboard.press(char)
                    if char == 'n':
                        keyboard.release('d')
                        keyboard.release('a')
                        keyboard.release('w')
                    if char == 'b':
                        keyboard.press('b')
                        time.sleep(0.1)
                        keyboard.release('b')
                    if char == 'v':
                        keyboard.press('v')
                        time.sleep(0.1)
                        keyboard.release('v')
                elif device_num == 2:
                    # Aqui você define as ações para ser2
                    if char == 'w':
                        keyboard.press(Key.up)
                    if char == 'a':
                        keyboard.press(Key.left)
                    if char == 'd':
                        keyboard.press(Key.right)
                    if char == 'n':
                        keyboard.release(Key.left)
                        keyboard.release(Key.up)
                        keyboard.release(Key.right)
                    if char == 'b':
                        keyboard.press('l')
                        time.sleep(0.1)
                        keyboard.release('l')
                    if char == 'v':
                        keyboard.press('k')
                        time.sleep(0.1)
                        keyboard.release('k')
    except Exception as e:
        print(f"An error occurred in device {device_num}: {e}")

def main():
    # Cria threads para cada conexão serial
    thread1 = threading.Thread(target=handle_serial, args=(ser1, 1))
    thread2 = threading.Thread(target=handle_serial, args=(ser2, 2))

    # Inicia as threads
    thread1.start()
    thread2.start()

    try:
        # Aguarda as threads terminarem (o que não ocorrerá neste caso até que o programa seja interrompido)
        thread1.join()
        thread2.join()
    except KeyboardInterrupt:
        print("Program terminated by user")

if __name__ == "__main__":
    main()
