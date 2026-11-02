# DIY Gaming Steering Wheel Firmware

Arduino firmware for a DIY gaming steering wheel. It reads the steering wheel and two analog paddles (gas and brake), plus a digital shift button, then sends those values over serial as FlySky **IBUS** channel packets.

## Hardware

| Input | Hardware | Arduino Nano pin |
| --- | --- | --- |
| Steering | Rotary encoder (quadrature A/B) | D2 (A), D3 (B) |
| Shift / button | Digital switch, active low | D4 |
| Gas paddle | B10K potentiometer | A0 |
| Brake paddle | B100K potentiometer | A7 |

Target board: **Arduino Nano (ATmega328, new bootloader)** as configured in `platformio.ini`. Serial runs at **115200** baud.

## What is sent

Every ~5 ms the firmware writes a 32-byte IBUS packet on USB serial (`Serial.write`). Channel values use the usual RC range of **1000–2000**, with **1500** as center / unused.

| IBUS channel | Source | Notes |
| --- | --- | --- |
| 1 | Steering | Encoder interrupt on pin A; constrained to 1000–2000 |
| 2 | Shift button | 2000 when pressed (pin LOW), 1000 when released |
| 3 | Gas paddle | Analog 0–1023 mapped to 1000–2000 |
| 4 | Brake paddle | Analog 0–1023 mapped to 1000–2000 |
| 5–14 | Unused | Filled with 1500 |

The packet layout is a standard 32-byte IBUS frame: length `0x20`, command `0x40`, 14 little-endian channel words, then a 16-bit checksum (`0xFFFF` minus the first 30 bytes).

Steering starts at 1500. Each encoder edge on pin A moves the value by 5 counts, based on whether A and B match.

## Build and upload

This is a [PlatformIO](https://platformio.org/) project.

```bash
pio run
pio run -t upload
pio device monitor
```

Source lives in `src/main.cpp`. Do not treat the serial monitor as text: IBUS is binary, so a game or RC adapter that speaks IBUS should consume the port.

## Repository layout

- `src/main.cpp` — encoder, paddles, and IBUS output
- `platformio.ini` — Nano board and 115200 baud
- `include/`, `lib/`, `test/` — unused PlatformIO placeholders
