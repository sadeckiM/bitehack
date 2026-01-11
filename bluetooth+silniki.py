import bluetooth
import aioble
import asyncio
from machine import Pin, PWM


import sys
print(f"MicroPython: {sys.version}")
print(f"aioble version check...")

try:
    import aioble
    print("aioble zaimportowane poprawnie")
except ImportError as e:
    print(f"Błąd importu aioble: {e}")
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

# Inicjalizacja sprzętu
left_motor = Motor(10, 12, 11) #a
right_motor = Motor(15, 13, 14) #b

# --- KONFIGURACJA BLUETOOTH (BLE) ---
_UART_UUID = bluetooth.UUID("6E400001-B5A3-F393-E0A9-E50E24DCCA9E")
_UART_TX_UUID = bluetooth.UUID("6E400003-B5A3-F393-E0A9-E50E24DCCA9E")
_UART_RX_UUID = bluetooth.UUID("6E400002-B5A3-F393-E0A9-E50E24DCCA9E")

# Rejestracja serwisu UART (używamy BufferedCharacteristic dla stabilności)
uart_service = aioble.Service(_UART_UUID)
tx_characteristic = aioble.Characteristic(uart_service, _UART_TX_UUID, read=True, notify=True)
rx_characteristic = aioble.BufferedCharacteristic(uart_service, _UART_RX_UUID, write=True, max_len=100)
aioble.register_services(uart_service)

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
                print(f"Silnik {cmd_word}: ustawiono {speed}")
                word_idx += 2
            except ValueError:
                word_idx += 1
        else:
            word_idx += 1

async def bluetooth_task():
    while True:
        print("Czekam na połączenie Bluetooth...")
        connection = None
        try:
            # Reklamowanie urządzenia
            connection = await aioble.advertise(
                250_000,
                name="Pico2W-Robot",
                services=[_UART_UUID],
            )
            
            print(f"Połączono z: {connection.device}")
            
            # Główna pętla obsługi połączenia
            try:
                while connection.is_connected():
                    try:
                        # Timeout, żeby nie blokować na zawsze
                        await asyncio.wait_for(rx_characteristic.written(), timeout=1.0)
                        
                        # Odczytujemy dane
                        raw_data = rx_characteristic.read()
                        
                        if raw_data:
                            command = raw_data.decode("utf-8").strip()
                            print(f"Odebrano: {command}")
                            parse_long_command(command)
                            
                            # Opcjonalne potwierdzenie
                            try:
                                tx_characteristic.write(f"ACK: {command}\n".encode())
                                await tx_characteristic.notified()
                            except:
                                pass  # Ignoruj błędy wysyłania
                                
                    except asyncio.TimeoutError:
                        # Normalny timeout - sprawdzamy czy dalej połączeni
                        continue
                    except Exception as e:
                        print(f"Błąd odczytu: {e}")
                        break
                        
            except Exception as e:
                print(f"Błąd w pętli połączenia: {e}")
                
        except asyncio.CancelledError:
            print("Anulowano zadanie")
            break
        except Exception as e:
            print(f"Błąd advertise: {e}")
            import sys
            sys.print_exception(e)  # Pełny stack trace
        finally:
            # Zatrzymaj silniki przy rozłączeniu
            left_motor.stop()
            right_motor.stop()
            
        print("Rozłączono. Restartuję za 1s...")
        await asyncio.sleep(1)
        
async def main():
    await asyncio.gather(bluetooth_task())

# Start programu
try:
    asyncio.run(main())
except KeyboardInterrupt:
    print("Zatrzymanie...")
finally:
    left_motor.stop()
    right_motor.stop()
    stby.value(0)