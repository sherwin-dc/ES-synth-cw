# Useful links

[STM32 Cube github](https://github.com/STMicroelectronics/STM32CubeL4)

[Using generated STCubeMX projects with VSCode](https://community.platformio.org/t/using-stm32cubemx-and-platformio/2611)

[HAL Documentation](https://www.st.com/resource/en/user_manual/dm00173145-description-of-stm32l4l4-hal-and-lowlayer-drivers-stmicroelectronics.pdf)

# Build

Install VSCode, PLatformIO extension

Download [STMCubeMX](https://www.st.com/en/development-tools/stm32cubemx.html)? (Maybe not needed)

Install [stm32pio](https://github.com/ussserrr/stm32pio): `pip install stm32pio`, then make sure PlatformIO CLI is [enabled](https://docs.platformio.org/en/latest/core/installation.html#piocore-install-shell-commands)

To regenerate HAL files from CubeMX file, run this command in this directory
```
stm32pio generate
```
Or to create a new PIO project:
```
stm32pio new -b nucleo_l432kc
```
If for whatever reason `stm32pio` isn't found, try to run it directly from the installation directory. For example, `C:\Users\0tian\AppData\Roaming\Python\Python310\Scripts\stm32pio.exe`

# Troubleshooting

[LIBUSB_ERROR_NOT_FOUND when uploading](https://edstem.org/us/courses/19499/discussion/1228459)

# Additional Reading

[Understanding and using HAL code](https://www.youtube.com/watch?v=txnViYePocg)

[SAI interface](https://www.st.com/content/ccc/resource/training/technical/product_training/0c/16/3b/b4/76/8a/47/51/STM32L4_Peripheral_SAI.pdf/files/STM32L4_Peripheral_SAI.pdf/jcr:content/translations/en.STM32L4_Peripheral_SAI.pdf) - In case we want to use I2S



