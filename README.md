# mtg-duino
MTG Life counter and game assist circuit powered by Arduino ESP32 Nano

## Hardware:
1. Arduino ESP32 Nano
2. I2C Multiplexer for multiple OLEDs with hardcoded address
3. Various buttons
4. Passive buzzer
5. 124x32 sd_1306 OLED screens

## Current Features
1. Track and display life of 2 players
2. Track and display player names
3. Randomized roll-off with animation for first player
4. Async Webserver to reset player/game state
5. Reset player state button
6. Increment/decrement health buttons for each player
7. Jingle on reset
8. Display death screen animation and jingle
9. Can select game mode (ex. commander vs standard)
10. First turn randomizer
11. Game start/reset graphic and music

## Upcoming features
1. Functionality for up to 4 players
2. Track commander damage taken separately
3. Improved graphics, animations, and displays
