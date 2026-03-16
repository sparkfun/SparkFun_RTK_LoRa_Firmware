# SparkFun RTK LoRa Firmware

**The file you need is [RTCM_TRX_RTK-Facet-FP_LoRa_250kHz-hopping_3.0.0.hex](./RTCM_TRX_FSS_RTK/RTCM_TRX_RTK-Facet-FP_LoRa_250kHz-hopping_3.0.0.hex)**

### Starting point:

_**LoRa-lora_hop.zip**_

A zip of a private repo, dated February 12th 2026

Contains a compiled binary named [RTCM_TRX_LORA_HOP_V005.bin](./RTCM_TRX_FSS_RTK/RTCM_TRX_LORA_HOP_V005.bin) but the VERSION in app_common.h is actually "0.0.6"

### Modifications:

In ```RTCM_TRX_FSS_RTK\STM32CubeIDE\.project```
* Replaced ```<location>D:/lora_fss/RTCM_TRX_FIX1/SubGHz_Phy/App/drv_flash.c</location>```
* with ```<locationURI>PARENT-1-PROJECT_LOC/SubGHz_Phy/App/drv_flash.c</locationURI>```
* Replaced ```<location>D:/lora_fss/RTCM_TRX_FIX1/SubGHz_Phy/App/rtcm_crc.c</location>```
* with ```<locationURI>PARENT-1-PROJECT_LOC/SubGHz_Phy/App/rtcm_crc.c</locationURI>```

Added ```.gitignore```

Corrected an error in ```drv_radio.c```: replaced ```rx_ptr += i;``` with ```uint8_t *rx_ptr = &msg.buf[i];```

Disable the LOG traffic by:
* Change ```#define APP_LOG_ENABLED``` to 0
* Commenting the ```#define MW_LOG_ENABLED```
* Add ```#define APP_PRINTF(...)``` in ```sys_app.h```

In ```usart_if.c```: add better support in ```vcom_Init``` etc. for ```DBG_PORT == 0 / 1```
* Remember to ```#include "sys_conf.h"``` where needed (```usart_if.c```)
* Comment ```DBG_PORT``` to disable vcom / trace driver and avoid UART conflict

Replace ```PORT_NUM``` with ```radio_param->dprt```
* On Torch: dprt defaults to ```COM_PORT_IDX``` (1 = UART2) for combined command and data on a single port
* On Facet FP: ```AT+DPRT=0``` selects UART1 as the data port

Changed the version to ```VERSION "3.0.0"```

Using STM32CubeIDE Version: 2.1.0

In the project properties \ C/C++ Build \ Settings \ MCU/MPU Post build outputs:
* Select "Convert to binary file" and "Convert to Intel Hex file"
* The Intel Hex file is useful as it contains the 0x8000000 start address for the CubeProgrammer

Building RTCM_TRX_FSS_RTK\STM32CubeIDE\.project produces:
* ```LoRa-lora_hop\LoRa-lora_hop\RTCM_TRX_FSS_RTK\STM32CubeIDE\Debug\RTCM_TRX.bin``` / ```.elf``` / ```.hex``` / ```.list``` / ```.map```

These have been copied to:
* [RTCM_TRX_RTK-Facet-FP_LoRa_250kHz-hopping_3.0.0.hex](./RTCM_TRX_FSS_RTK/RTCM_TRX_RTK-Facet-FP_LoRa_250kHz-hopping_3.0.0.hex) etc.

Set RTK unit to LoRa direct connect (s h 17) to connect STM32WL55 UART to USB through the ESP32
* On Torch: the STM32WL55 is programmed via UART1
* On Facet FP: the STM32WL55 is programmed via UART2

Use STM32CubeProgrammer v2.22.0 to program the .hex onto the STM32WL55

Set the Base RTK unit to Base (Survey-In) and enable LoRa (TX) at 910MHz

```
Menu: Radios
1) ESP-NOW Radio: Disabled
10) LoRa Radio: Enabled - Firmware v3.0.0
11) LoRa Coordination Frequency: 910.000
12) LoRa Transmit Power: 0dBm
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

On the Rover RTK unit, enable LoRa Radio at 910MHz

On Facet FP: the firmware sends ```AT+DPRT=0``` to select UART1 as the data port

Setting the LoRa transmit power to 0dBm helps reduce the peak current consumption:
* On a battery-less Facet FP, this can help prevent power brown-outs during TX
* Default transmit power is 10dBm
