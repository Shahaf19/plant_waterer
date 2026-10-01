# Decision log

One line per decision: what was decided, what else was considered, and why.

| # | Date | Decision | Alternatives | Why |
|---|------|----------|--------------|-----|
| 1 | 2026-10-01 | Arduino Uno as the controller | ESP32 / ESP8266 (Wi-Fi) | Already owned; the 6 h budget favours known hardware over new features |
| 2 | 2026-10-01 | VS Code + PlatformIO | Arduino IDE | One tool for build, upload, serial monitor and native unit tests on the PC |
| 3 | 2026-10-01 | Offline scope, status over serial | Web dashboard / MQTT | The Uno has no Wi-Fi; keeps the scope inside 6 h |
| 4 | 2026-10-01 | No USB cable yet: order M0 (build only) → M2 → M3 → M1 → M4 → M5 → M6 | Stop and wait for the cable | Design and pure logic don't need the board; calibration only changes two constants later |
| 5 | 2026-10-01 | Host compiler for native tests: existing MinGW g++ 9.2 (32-bit) | WSL Ubuntu; fresh MSYS2 UCRT64 g++ 14 | Already installed and on PATH, so no setup time; C++17 is enough for the pure logic |
| 6 | 2026-10-01 | Wokwi simulator: decide at Milestone 4 | Set up now; never | Nothing before M4 needs a board or simulator; the cable may arrive by then |
| 7 | 2026-10-01 | Watering = fixed dose, then soak, then re-measure; max doses in a row → fault | Run until sensor reads wet; dose with early stop | Sensor lags behind the water, so "run until wet" overwaters; the dose cap catches an empty tank or a sensor stuck at "dry" |
| 8 | 2026-10-01 | Fault latches until a person presses reset | Auto-recover when readings look sane; auto-retry after a long wait | An empty tank looks "sane", so auto-recovery would run the pump dry; reset needs no extra code (boots pump-off in Monitoring) |
