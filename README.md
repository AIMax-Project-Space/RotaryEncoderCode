# DIY Gaming Steering Wheel Firmware

Arduino firmware for a DIY gaming steering wheel. It reads an **AS5600** magnetic steering encoder, two analog paddles (gas and brake), and a digital shift button, then sends those values over serial as FlySky **IBUS** channel packets.

Target board: **Arduino Nano (ATmega328, new bootloader)** as configured in `platformio.ini`. Serial runs at **115200** baud.  
  
## Serial Monitor with hex code
```
C:\Users\Dulana\.platformio\penv\Scripts\platformio.exe device monitor --baud 115200 -f hexlify
```

## Wiring

Power the Nano from USB (or VIN if you use an external supply). Use a **common GND** for every sensor.

### AS5600 magnetic encoder (steering)

Mount a diametric magnet on the steering shaft, facing the chip on the AS5600 board. Typical air gap is about **0.5–3 mm**, magnet centered over the IC.

| AS5600 pin | Arduino Nano |
| --- | --- |
| VCC | 5 V (use 3.3 V if your module is 3.3 V only) |
| GND | GND |
| SDA | **A4** (SDA) |
| SCL | **A5** (SCL) |

Leave DIR / PGO / OUT unconnected unless your module requires them. This firmware talks I2C at address `0x36` and does not use analog or PWM out.

Do not use **A4** or **A5** for the paddles; those pins are I2C.

### Shift button

| Button | Arduino Nano |
| --- | --- |
| One side | **D4** |
| Other side | GND |

The pin uses the internal pull-up. Pressed = LOW.

### Gas and brake paddles

| Paddle | Pot | Arduino Nano |
| --- | --- | --- |
| Gas | B10K | **A0** (wiper) |
| Brake | B100K | **A7** (wiper) |

For each pot: one outer pin to **5 V**, the other outer pin to **GND**, wiper to the analog pin above.

### Pin summary

| Function | Hardware | Nano pin |
| --- | --- | --- |
| Steering | AS5600 SDA | A4 |
| Steering | AS5600 SCL | A5 |
| Shift / button | Switch to GND | D4 |
| Gas paddle | B10K wiper | A0 |
| Brake paddle | B100K wiper | A7 |

## Steering setup

Power on with the wheel **straight**. That angle is IBUS center (1500).

Lock-to-lock is **270°** (`STEERING_LOCK_TO_LOCK_DEG` in `src/main.cpp`). Change that value if your wheel has more or less travel.

## What is sent

Every ~5 ms the firmware writes a 32-byte IBUS packet on USB serial (`Serial.write`). Channel values use the usual RC range of **1000–2000**, with **1500** as center / unused.

| IBUS channel | Source | Notes |
| --- | --- | --- |
| 1 | Steering (AS5600) | Mapped from lock-to-lock degrees; constrained to 1000–2000 |
| 2 | Shift button | 2000 when pressed (pin LOW), 1000 when released |
| 3 | Gas paddle | Analog 0–1023 mapped to 1000–2000 |
| 4 | Brake paddle | Analog 0–1023 mapped to 1000–2000 |
| 5–14 | Unused | Filled with 1500 |

The packet layout is a standard 32-byte IBUS frame: length `0x20`, command `0x40`, 14 little-endian channel words, then a 16-bit checksum (`0xFFFF` minus the first 30 bytes).

## Build and upload

This is a [PlatformIO](https://platformio.org/) project.

```bash
pio run
pio run -t upload
pio device monitor
```

Source lives in `src/main.cpp`. Do not treat the serial monitor as text: IBUS is binary, so a game or RC adapter that speaks IBUS should consume the port.

## Repository layout

- `src/main.cpp` — AS5600 steering, paddles, and IBUS output
- `platformio.ini` — Nano board and 115200 baud
- `include/`, `lib/`, `test/` — unused PlatformIO placeholders
