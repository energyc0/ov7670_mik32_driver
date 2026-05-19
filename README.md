# OV7670 DRIVER FOR MIK32 ELBEAR ACE-UNO
This is an OV7670 driver for MIK32 ELBEAR ACE-UNO RISC-V microcontroller.
This project is not completed yet, because there are some bugs with camera. Tests of *"color bars"* are passed, but *"fading color bars"* and *"shiting-1"* are not.
Driver uses [Adafruit](https://github.com/adafruit/Adafruit_OV7670), [MIK32 HAL](https://github.com/MikronMIK32/mik32-hal) and [MIK32 HAL-shared](https://github.com/MikronMIK32/mik32v2-shared) libraries.
There was implemented I2C bit-bang for SCCB, becuase hardware I2C cannot handle NACK the way I needed.

### PINS SETUP
Pins for ov7670 on MIK32 ELBEAR UNO:
- **D0** is **D3** - PORT_0_0
- **D1** is **D5** - PORT_0_1
- **D2** is **D6** - PORT_0_2
- **D3** is **D9** - PORT_0_3
- **D4** is **A2** - PORT_0_4
- **D5** is **RX** - PORT_0_5
- **D6** is **A4** - PORT_0_9
- **D7** is **D2** - PORT_0_10
- **SIOC** is **SCL** - PORT_1_13
- **SIOD** is **SDA** - PORT_1_12
- **HREF** is **D13** - PORT_1_2
- **VSYNC** is **D12** - PORT_1_0
- **XCLK** is **D10** - PORT_1_3
- **PCLK** is **D11** - PORT_1_1
- **RESET** is **3.3V**
- **PWDN** is **GND**
Microcontroller pins' output is 3.3V, so there is no need for resistors.

### CONTACTS

+ **Email:** maksim.gavrilov.0@mail.ru
+ **Telegram:** @energyc0
+ **Full name:** Maxim Gavrilov
