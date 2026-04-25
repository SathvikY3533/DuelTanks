# Duel Tanks
### Two-player real-time tank battle — ECE319H Spring 2026

> Check out more of my work at **[sathvikyechuri.com](https://www.sathvikyechuri.com)**

---

## Demo

📺 *Video coming soon*

---

## Screenshots

| Splash Screen | Tank Select | Gameplay |
|---|---|---|
| *coming soon* | *coming soon* | *coming soon* |

---

## Overview

Duel Tanks is a real-time two-player artillery game built completely from scratch on the MSPM0G3507 microcontroller. Two players face off on separate boards connected by a UART link — shells fired off one screen physically cross over to the opponent's screen, with full parabolic physics preserved across the wire.

Everything — the graphics engine, sprite system, physics engine, UART protocol, sound system, and game state machine — was written from scratch in C++. No game engine, no external libraries.

**Placed 3rd in the ECE319H Spring 2026 in-class competition.**

---

## How It Works

Each board runs the same firmware with a `PLAYER_ID` flag determining which side the player starts on. Missiles exit one screen, get encoded into a 5-byte UART packet, and spawn on the opponent's screen with matching velocity — making it feel like a single continuous battlefield split across two displays.

```
Board 1 (left)          Board 2 (right)
┌──────────────┐        ┌──────────────┐
│   🎯   >>>   │──UART──│   >>> 💥     │
│  [Tank]      │        │       [Tank] │
└──────────────┘        └──────────────┘
     PA8 TX ──────────────► PA22 RX
     PA22 RX ◄────────────── PA8 TX
     GND ◄──────────────────► GND
```

---

## Features

- **Parabolic missile physics** — fixed-point arithmetic, gravity simulation, arc trajectory
- **Cross-screen projectiles** — missiles exit your screen and appear on the opponent's via UART
- **City background** — night skyline with building silhouettes and window lighting
- **Hilly terrain** — multi-sine generated terrain profile, tanks ride the hills
- **3 playable tanks** with unique abilities:
  - 🔵 **Brawler** — Double Shot (fires two missiles spread ±5°)
  - 🟢 **Phantom** — Speed Boost (2× movement speed for 1.5 sec)
  - 🔴 **Artillery** — Ricochet (next shot bounces off terrain)
- **Full game flow** — splash → language select → rules → tank select → battle ready handshake → countdown → gameplay → end screen
- **2-minute match timer** — compare lives at time's up for winner
- **English / Spanish** language support
- **Sound effects** — shoot, explosion, incoming warning, victory, defeat, and more

---

## Hardware

| Component | Details |
|---|---|
| MCU | MSPM0G3507 LaunchPad |
| Display | ST7735R LCD (160×128, landscape) |
| Input | 4 tactile buttons + slide potentiometer |
| Audio | 5-bit R2R DAC → MCP34119P amplifier → speaker |
| Sync | Battle Ready GPIO handshake (PA17 / PA31) |
| Communication | UART at 115200 baud, board-to-board |

### Pin Map
| Signal | Pin |
|---|---|
| LCD SPI | PB6, PB7, PB8, PB9, PB15, PA13 |
| Slide pot (ADC) | PB18 |
| 5-bit DAC | PB0 – PB4 |
| UART TX | PA8 (UART1) |
| UART RX | PA22 (UART2) |
| Battle Ready Out | PA17 (P1) / PA31 (P2) |
| Battle Ready In | PA31 (P1) / PA17 (P2) |
| Fire button | PB17 |
| Shield button | PB12 |
| Left button | PB16 |
| Right button | PA27 |

---

## Software Architecture

```
main5.cpp
└── Game_Update()          ← called every 33ms (30Hz)
    ├── Switch_Tick()      ← edge detection for all 4 buttons
    ├── State machine      ← SPLASH → LANGUAGE → RULES → TANK_SELECT
    │                         → WAITING → COUNTDOWN → GAMEPLAY → END
    ├── Physics engine     ← fixed-point parabolic missile simulation
    ├── Sprite renderer    ← transparent pixel-by-pixel draw, dirty-rect erase
    ├── GameUART           ← 5-byte packets with 0xAA 0x55 sync header
    └── Sound engine       ← SysTick ISR at 11025Hz → R2R DAC output
```

### UART Packet Format
```
[ 0xAA ][ 0x55 ][ TYPE ][ DATA1 ][ DATA2 ]
  sync    sync    what    payload  payload
```

---

## Controls

| Input | Action |
|---|---|
| Left / Right buttons | Move tank |
| Slide pot | Aim angle (0° – 89°) |
| Hold Fire | Charge shot power |
| Release Fire | Shoot |
| Shield | Use tank ability |
| Fire + Left + Right (hold) | Force quit match |

---

## Building

Open in Code Composer Studio with the ECE319H Lab 9 starter project. Add all `.cpp` / `.h` files. Before flashing:

1. Set `#define PLAYER_ID 1` in `Game.h` for the left board
2. Set `#define PLAYER_ID 2` in `Game.h` for the right board
3. Connect boards: PA8→PA22, PA22→PA8, GND→GND

---

## License

Built for ECE319H at UT Austin. All game logic, physics, graphics, and UART protocol written from scratch.
