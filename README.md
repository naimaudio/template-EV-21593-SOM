# Audio I/O Unification Framework

## Overview
This project provides a **framework for unifying all audio I/O streams**—including **ADC (analog inputs), DAC (analog outputs), and SPDIF (digital input/output)**—into a **single global buffer**. The goal is to simplify the development of DSP (Digital Signal Processing) algorithms by presenting a consistent and synchronized data structure for all channels.

## Features
- **Unified Global Buffer**: Combines multiple input sources (e.g., TDM16 ADC, SPDIF Rx) into one coherent buffer for processing.
- **Ping-Pong Buffering**: Ensures deterministic, low-latency audio streaming without data corruption.
- **Multi-Interface Support**:
  - SPORT interfaces for TDM16 and I²S.
  - SPDIF Rx/Tx integration.
- **Flexible Routing**: Easily map any input channel to any output channel in the processing stage.
- **Scalable Design**: Supports adding more I/O interfaces with minimal changes.

## ⚠️ DAC Output Level (attenuated by default)
The analog **DAC outputs ship attenuated to −24 dB** for safe listening/test levels.
This does **not** affect the SPDIF Tx output. The attenuation is a single master gain
applied to all 12 DAC channels, defined in [`src/audio/io.h`](templateTDM16_Core1/src/audio/io.h):

```c
#define DAC_OUTPUT_GAIN  (0.0630957f)   // -24 dB
```

To change the level, edit that macro (voltage gain = 10^(dB/20)):

| Level             | Value         |
|-------------------|---------------|
| 0 dB (full scale) | `1.0f`        |
| −6 dB             | `0.5011872f`  |
| −12 dB            | `0.2511886f`  |
| −24 dB (current)  | `0.0630957f`  |
| −42 dB            | `0.0079433f`  |

It is applied in `fillDACOutputFromGlobal()` ([`src/audio/io.c`](templateTDM16_Core1/src/audio/io.c)).

## Architecture
```
[ ADC (SPORT4B) ]   [ SPDIF Rx (SPORT0B) ]
        |                   |
        v                   v
   +-----------------------------+
   |     Global Rx Buffer       |
   +-----------------------------+
                |
          [ DSP Processing ]
                |
   +-----------------------------+
   |     Global Tx Buffer       |
   +-----------------------------+
        |                   |
[ DAC (SPORT4A) ]       [ SPDIF Tx (SPORT0A) ]
```

- **Global Rx Buffer**: Aggregates all input streams.
- **Global Tx Buffer**: Holds processed data for all outputs.
- **Main Loop**:
  1. Gather inputs into Global Rx.
  2. Run `processBlock()` on Global Rx → Global Tx.
  3. Distribute Global Tx to all outputs.

## How It Works
- **fillGlobalInput()**: Collects fresh data from JACK (ADC) and SPDIF into the global input buffer.
- **processBlock()**: Applies DSP algorithms on the unified buffer.
- **fillOutputsFromGlobal()**: Pushes processed data to DAC and SPDIF outputs.
- **Ping-Pong Mechanism**: Each stream uses double-buffering for safe concurrent DMA and CPU access.

## Future Improvements
- **ASRC Integration**: Add asynchronous sample rate conversion for SPDIF Rx to handle clock domain differences.
- **Dynamic Routing**: Implement runtime channel mapping.
- **Clock Management**: Option to use PCG as master or slave to external DAC clock.
- **Error Handling**: Add robust detection for SPDIF lock/unlock and buffer underruns.

---
**Target Platform**: ADSP-SC5xx / ADSP-2159x with EV-SOMCRR board.
**Primary Use Case**: Multi-channel audio processing with mixed analog and digital I/O.
