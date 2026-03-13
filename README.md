# LoRa

### Starting point:

LoRa-lora_hop.zip
A zip of a private repo, dated February 12th 2026
Contains a compiled binary named "RTCM_TRX_LORA_HOP_V005.bin" but the VERSION in app_common.h is actually "0.0.6"

### Building:

Changing as little as possible:

In RTCM_TRX_FSS_RTK\STM32CubeIDE\.project
Replaced ```<location>D:/lora_fss/RTCM_TRX_FIX1/SubGHz_Phy/App/drv_flash.c</location>```
with ```<locationURI>PARENT-1-PROJECT_LOC/SubGHz_Phy/App/drv_flash.c</locationURI>```
Replaced ```<location>D:/lora_fss/RTCM_TRX_FIX1/SubGHz_Phy/App/rtcm_crc.c</location>```
with ```<locationURI>PARENT-1-PROJECT_LOC/SubGHz_Phy/App/rtcm_crc.c</locationURI>```

Added .gitignore

Corrected an error in drv_radio.c: replaced ```rx_ptr += i;``` with ```uint8_t *rx_ptr = &msg.buf[i];```

Using STM32CubeIDE Version: 2.1.0

In the project properties \ C/C++ Build \ Settings \ MCU/MPU Post build outputs
select "Convert to binary file" and "Convert to Intel Hex file"
The Intel Hex file is useful as it contains the 0x8000000 start address for the CubeProgrammer

Building RTCM_TRX_FSS_RTK\STM32CubeIDE\.project produces
LoRa-lora_hop\LoRa-lora_hop\RTCM_TRX_FSS_RTK\STM32CubeIDE\Debug\RTCM_TRX.bin / .elf / .hex / .list / .map

Copied these to:
LoRa-lora_hop\LoRa-lora_hop\RTCM_TRX_FSS_RTK\RTCM_TRX_RTK-Facet-FP_LoRa_250kHz-hopping_0.0.6_TORCH.bin / .elf / .hex / .list / .map

Used STM32CubeProgrammer v2.22.0 to program the .hex onto Torch

Set Torch to Base (Survey-In) and enabled LoRa (TX) at 910MHz

```
Menu: Radios
1) ESP-NOW Radio: Disabled
LoRa radio configured for receiving
10) LoRa Radio: Enabled - Firmware v0.0.6
11) LoRa Coordination Frequency: 910.000
```

```
Base Mode - SIV: 39
LoRa transmitted 3120 RTCM bytes
Base Mode - SIV: 39
LoRa transmitted 3194 RTCM bytes
Base Mode - SIV: 39
Base Mode - SIV: 39
LoRa transmitted 3120 RTCM bytes
Base Mode - SIV: 39
LoRa transmitted 3120 RTCM bytes
Base Mode - SIV: 39
Base Mode - SIV: 39
LoRa transmitted 3194 RTCM bytes
Base Mode - SIV: 39
LoRa transmitted 3120 RTCM bytes
Base Mode - SIV: 39
Base Mode - SIV: 39
LoRa transmitted 3120 RTCM bytes
```

For Facet FP, we want the RTCM to be on UART2

Disable the LOG traffic by:
Change #define APP_LOG_ENABLED to 0
Commenting the #define MW_LOG_ENABLED
Add #define APP_PRINTF(...) in sys_app.h

In usart_if.c: add better support in vcom_Init etc. for DBG_PORT == 0 / 1
Remember to #include "sys_conf.h" where needed (usart_if.c)
Comment DBG_PORT to disable vcom / trace driver and avoid UART conflict

VERSION "3.0.0"

