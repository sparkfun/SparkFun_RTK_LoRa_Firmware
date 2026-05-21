# SparkFun RTK LoRa Firmware

This repo contains the source code and compiled firmware binaries for the STM32WL55 LoRa chipset used in the [SparkPNT Facet FP](https://www.sparkfun.com/sparkpnt-facet-fp) product range

You will find the latest firmware binary attached to the latest [release](https://github.com/sparkfun/SparkFun_RTK_LoRa_Firmware/releases). The **RTCM_TRX.zip** Asset contains the firmware in **.elf** , **.hex** and **.bin** binary format.

This same firmware can also be run on the original [SparkFun RTK Torch](https://www.sparkfun.com/sparkfun-rtk-torch.html). Provided both Facet FP and Torch are running the same LoRa firmware, they can act as a LoRa-connected Base-Rover pair. But, please be aware that the firmware in this repo is _not_ compatible with the original [Torch LoRa firmware v2.0.2](https://github.com/sparkfun/SparkFun_RTK_Everywhere_Firmware_Binaries/tree/main/STM32_LoRa). If you want to use Torch and Facet FP together, the Torch LoRa firmware will need to be upgraded.

## Updating the Facet FP LoRa Firmware

The procedure for updating the Facet FP LoRa Firmware is still work-in-progress. But - for now - it is almost identical to Torch (described [below](#updating-the-torch-lora-firmware)): put the firmware into ```STM32 direct connect``` mode using menu options ```s``` , ```h``` , ```17```; then use STM32CubeProgrammer to upload the binary.

The **.elf** or **.hex** firmware binary is easier to use than the raw **.bin** binary since they contain the program memory locations within the file. With the **.bin** binary, you need to manually set the flash **Start address** to **0x08000000**

## Updating the Torch LoRa Firmware

The procedure for updating the Torch LoRa Firmware is described [here](https://docs.sparkfun.com/SparkFun_RTK_Everywhere_Firmware/firmware_update_stm32/)

## Thanks!

The LoRa firmware was developed for us by Jay Bu and colleagues at Limosa. Thank you Jay.

SparkFun have tweaked it a little to:

* Allow both UART1 and UART2 to be used as the data port
    * On Torch: UART2 acts as a combined control and data port
    * On Facet FP: the firmware sends ```AT+DPRT=0``` to select UART1 as the data port (which is connected directly to the GNSS)
* Disable the LOG output, since both UARTs are needed on Facet FP for control and data
* ```STMFLASH_Write``` correctly erases the sector before writing the radio attributes
* Added the ```AT+SAVE``` command to avoid unneeded flash writes

## Compiling the Firmware

### How SparkFun Does It

We use [GitHub Actions (Workflows)](https://github.com/sparkfun/SparkFun_RTK_LoRa_Firmware/tree/main/.github/workflows) to compile the firmware binary. We use Docker and a [Dockerfile](./Dockerfile) to build the firmware using ST's STM32CubeCLT command-line toolset for Linux on a virtual ubuntu machine. Because STM32CubeCLT is licenced software, we pull in our own encrypted copy from [this repo](https://github.com/sparkfun/SparkFun_STM32CubeCLT).

### Compiling Locally

If you want to compile the LoRa firmware locally, you absolutely can but you will need to download and install either ST's [STM32CubeIDE](https://www.st.com/en/development-tools/stm32cubeide.html) development environment or the [STM32CubeCLT](https://www.st.com/en/development-tools/stm32cubeclt.html).

The IDE is easier to use, and there are installers for Windows, macOS and Linux. Download or copy the LoRa source code from this repo, use the ```STM32 Project Create / Import``` option (under ```File```) to import the ```RTCM_TRX``` project (in STM32CubeIDE), then select ```Build Project```. It really is as easy as that.

The compiled ```RTCM_TRX.bin \ .elf \ .hex``` firmware files can be found in ```RTCM_TRX_FSS_RTK\STM32CubeIDE\Debug```

If you are looking for the firmware ```VERSION```, you will find it in [RTCM_TRX_FSS_RTK/SubGHz_Phy/App/include/app_common.h](./RTCM_TRX_FSS_RTK/SubGHz_Phy/App/include/app_common.h)

We have built the code successfully using: STM32CubeIDE version 2.1.0 ; and STM32CubeCLT version 1.21.0

If you want to see the saved radio attributes in flash memory, use STM32CubeProgrammer to read (at least) 64 bytes from address **0x0803e800**

## Licence

The parts of this firmware which SparkFun wrote are covered by the MIT licence. Please see [LICENSE.md](./LICENSE.md) for details.

Other parts of this firmware have different licences, depending on the licence selected by the original author. Some parts are covered by the MIT licence, some by the BSD-3-Clause licence, some by the Apache-2.0 licence. Since we use STM32CubeCLT to compile the code, it is also covered by the ST [Software Package License Agreement](https://www.st.com/resource/en/license_agreement/dm00218346.pdf).

If you are reusing or redistributing this firmware, please ensure you comply with all relevant licences.

* Your friends at SparkFun
