import bluetooth
import aioble
import asyncio
from machine import Pin, PWM
import sys

# --- KONFIGURACJA DIODY LED ---
# "LED" działa dla Raspberry Pi Pico W. Jeśli masz inne boardy, użyj numeru pinu, np. Pin(25, Pin.OUT)
led = Pin("LED", Pin.OUT)
is_connected = False  # Globalna flaga stanu połączenia

# --- KONFIGURACJA SILNIKÓW ---
class Motor:
    def __init__(self, pwm_pin, in1_pin, in2_pin):
        self.pwm = PWM(Pin(pwm_pin))
        self.pwm.freq(1000)
        self.in1 = Pin(in1_pin, Pin.OUT)
        self.in2 = Pin(in2_pin, Pin.OUT)

    def set(self, speed):
        if speed > 0:
            self.in1.value(1)
            self.in2.value(0)
            self.pwm.duty_u16(int(min(speed, 100) * 655.35))
        elif speed < 0:
            self.in1.value(0)
            self.in2.value(1)
            self.pwm.duty_u16(int(min(abs(speed), 100) * 655.35))
        else:
            self.stop()

    def stop(self):
        self.pwm.duty_u16(0)
        self.in1.value(0)
        self.in2.value(0)

left_motor = Motor(10, 12, 11)
right_motor = Motor(15, 13, 14)

# --- BLUETOOTH CONFIG ---
_UART_UUID = bluetooth.UUID("6E400001-B5A3-F393-E0A9-E50E24DCCA9E")
_UART_TX_UUID = bluetooth.UUID("6E400003-B5A3-F393-E0A9-E50E24DCCA9E")
_UART_RX_UUID = bluetooth.UUID("6E400002-B5A3-F393-E0A9-E50E24DCCA9E")

uart_service = aioble.Service(_UART_UUID)
tx_characteristic = aioble.Characteristic(uart_service, _UART_TX_UUID, read=True, notify=True)
rx_characteristic = aioble.BufferedCharacteristic(uart_service, _UART_RX_UUID, write=True, max_len=100)
aioble.register_services(uart_service)

# --- ZADANIE DIODY LED ---
async def led_task():
    global is_connected
    while True:
        if is_connected:
            led.value(1)  # Świeć ciągle jeśli połączono
            await asyncio.sleep(0.5)
        else:
            led.toggle()  # Migaj jeśli czekasz na połączenie
            await asyncio.sleep(0.2)

def parse_long_command(input_cmd):
    words = input_cmd.strip().split()
    word_idx = 0
    while word_idx < len(words):
        cmd_word = words[word_idx].upper()
        if cmd_word in ["LEFT", "RIGHT"] and word_idx + 1 < len(words):
            try:
                speed = int(words[word_idx + 1])
                motor = left_motor if cmd_word == "LEFT" else right_motor
                motor.set(speed)
                print(f"Silnik {cmd_word}: {speed}")
                word_idx += 2
            except ValueError:
                word_idx += 1
        else:
            word_idx += 1

async def bluetooth_task():
    global is_connected
    while True:
        is_connected = False
        print("Czekam na połączenie Bluetooth...")
        try:
            connection = await aioble.advertise(
                250_000,
                name="Pico2W-Robot",
                services=[_UART_UUID],
            )
            
            is_connected = True # Zmieniamy stan na połączony
            print(f"Połączono z: {connection.device}")
            
            try:
                while connection.is_connected():
                    try:
                        await asyncio.wait_for(rx_characteristic.written(), timeout=1.0)
                        raw_data = rx_characteristic.read()
                        if raw_data:
                            command = raw_data.decode("utf-8").strip()
                            parse_long_command(command)
                            tx_characteristic.write(f"ACK: {command}\n".encode())
                    except asyncio.TimeoutError:
                        continue
            except Exception as e:
                print(f"Błąd połączenia: {e}")
        except Exception as e:
            print(f"Błąd advertise: {e}")
        finally:
            is_connected = False
            left_motor.stop()
            right_motor.stop()
            print("Rozłączono. Restartuję...")
            await asyncio.sleep(1)

async def main():
    # Uruchamiamy oba zadania równolegle
    await asyncio.gather(bluetooth_task(), led_task())

try:
    asyncio.run(main())
except KeyboardInterrupt:
    print("Zatrzymanie...")
finally:
    led.value(0)
    left_motor.stop()
    right_motor.stop()