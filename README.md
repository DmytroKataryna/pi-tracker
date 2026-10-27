# pi-tracker

Embedded Linux project on a Raspberry Pi 5 in modern C++ (C++20): userspace sensor drivers,
a multithreaded sensor pipeline and, next, camera video streaming and MAVLink guidance.

## Hardware

| Part | Notes |
|---|---|
| Raspberry Pi 5 | Debian 13 (trixie), kernel 6.18 |
| GY-521 IMU board | sold as MPU-6050, but `WHO_AM_I = 0x70` shows it is an **MPU-6500** (I2C `0x68`) |
| GY-302 (BH1750) | ambient light sensor (I2C `0x23`) |
| Logic analyzer | 24 MHz, 8 channels (FX2-based, works with sigrok via `fx2lafw`) |

## Project layout

```
include/pi_tracker/   public headers of the pitracker library
  fd.h                  RAII owner of a Linux file descriptor (move-only)
  ring_buffer.h         fixed-capacity ring buffer, no heap allocation
  thread_safe_queue.h   blocking queue (mutex + condition_variable) with close()
  i2c_device.h          register-level I2C access via I2C_RDWR (repeated START)
  imu.h                 MPU-6500 driver: init, ranges, DLPF, gyro calibration
src/                  library sources
sandbox/              daily exercises (dayNN_*.cpp), each built as its own executable
data/                 recorded sensor logs and logic analyzer captures
docs/buses.md         notes on UART / I2C / SPI
```

## Build

```bash
cmake -S . -B build
cmake --build build
```

## IMU driver (MPU-6500)

- Initialisation follows the register map: reset, PLL clock, `WHO_AM_I` check,
  ±4 g / ±500 °/s ranges, on-chip low-pass filters (gyro ≈41 Hz, accel ≈45 Hz) against
  motor-vibration aliasing, 100 Hz sample rate.
- One 14-byte burst read per sample (accel, temperature, gyro) gives a consistent snapshot.
- Gyro bias is calibrated at startup by averaging 200 samples at rest.

`sandbox/day10_imu_logger.cpp` logs 10 s at 100 Hz (`sleep_until` scheduling) to CSV:

| Metric | Result |
|---|---|
| Samples / gaps | 1000 / 0 |
| I2C read time | ≈1.6 ms per sample (bus at 100 kHz) |
| Gyro at rest, max \|ω\| | ≈5 °/s before calibration → 0.15 °/s after |
| Integrated yaw drift over 10 s | −0.02° |

## Logic analyzer: I2C on the wire

Captured on the Pi with `sigrok-cli`, viewed in PulseView. Reading the `WHO_AM_I`
register of the IMU:

![I2C read of WHO_AM_I decoded in PulseView](data/logic_analyzer.png)

- `Address read: 68` — the address byte on the wire is `0xD1` (`0x68 << 1 | R`);
- `ACK` from the sensor, then `Data read: 70` — the MPU-6500 chip ID;
- `NACK` from the Pi (master: "no more bytes"), then `STOP`.

What the capture confirmed:

- **Bus time dominates the read.** A 14-byte register read is 153 SCL clocks
  (3 address/register bytes + 14 data bytes, 9 clocks each) ≈ 1.53 ms at 100 kHz, which
  matches the ≈1.6 ms measured in the logger. Raising the bus to 400 kHz should make it ~4× faster.
- **Trigger pitfall.** Triggering on the first falling edge of SDA without pre-trigger samples
  cut off the initial START, so the decoder skipped the register-address write and labelled
  the repeated START as a plain `S`. Capturing with a wider window (or pre-trigger) shows the
  full write → repeated START → read sequence.

Raw captures: `data/day11_whoami.sr`, `data/day11_logger.sr` (open in PulseView).
