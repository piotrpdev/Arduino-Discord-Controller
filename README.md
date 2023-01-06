# Arduino-Discord-Controller

https://user-images.githubusercontent.com/99439005/211055978-7c768534-915b-4ee5-8f82-d27e4d3f078e.mp4

Controlling Discord using a combination of Arduino, BetterDiscord, and Python.

## Features

- [x] Mute
- [x] Deafen
- [x] Disconnect from voice channel
- [x] Arduino reacts to Discord changes
- [ ] See users in voice channel

## How to use

See [INSTRUCTIONS.md](./INSTRUCTIONS.md) for set-up.

> TECH TIP: If doing dev, create a symlink for the betterdiscord plugin.

## TODO

### Python Script

- [x] Use `"AUDIO_TOGGLE"` for mute and deafen
- [x] Send D0 when exiting script
- [x] Handle RTC_CONNECTION_STATE

### BetterDiscord Plugin

- [ ] Reconnect WebSocket on py script close

### Arduino Sketch

- [ ] Cleaner way of resetting text settings
- [ ] Dot loading animation
- [ ] Display how to use initially
- [ ] Add welcome message or something
- [ ] Mess around with timeout and baud for speed
- [ ] Handle text overflow
