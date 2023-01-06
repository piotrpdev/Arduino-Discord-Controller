# Arduino-Discord-Controller

![Showcase video](./.github/showcase.mp4)

Controlling Discord using a combination of Arduino, BetterDiscord, and Python.

## How to use

See [INSTRUCTIONS.md](./INSTRUCTIONS.md) for set-up.

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
