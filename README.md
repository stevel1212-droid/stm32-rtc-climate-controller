# stm32-rtc-climate-controller

KiCad project of an STM32-based board climate controller utilizing an RTC module for time-scheduled environmental reading of temperature/humidity and optional small relay control.

<img width="1024" height="768" alt="image" src="https://github.com/user-attachments/assets/e3338b9d-6462-41b1-b807-ba323c85105a" />


### 🛠️ Tools & Plugins Used:

#### Hardware (KiCad)
* **Footprints & Symbols:** To quickly download components from JLCPCB, I used the [easyeda2kicad](https://github.com/uPesy/easyeda2kicad.py) tool.
* **Manufacturing Files:** To create the fabrication files for PCB and SMD assembly via JLCPCB, I used the [kicad-jlcpcb-tools](https://github.com/bouni/kicad-jlcpcb-tools) external plugin.

#### Firmware (Software)
* **IDE:** Developed using [Visual Studio Code](https://code.visualstudio.com/) with the **PlatformIO IDE** extension.
* **Framework:** Written in C/C++ using the **Arduino Framework** for the STM32F103C8 microcontroller.
* **Peripherals:** Includes support for an SSD1306 OLED display using Adafruit libraries.

---

### 🛒 Hardware Components & Equipment Used:

If you want to build or debug this project, here is the specific hardware and equipment used:

* **Display:** [0.96 inch OLED I2C Display SSD1306](https://www.amazon.com/dp/B0D2RMQQHR?ref_=ppx_hzsearch_conn_dt_b_fed_asin_title_3) - Used for showing temperature, humidity, and time data.
* **RTC Battery:** [Energizer CR2032 Lithium Battery](https://www.amazon.com/dp/B009108SGS?ref=nb_sb_ss_w_as-reorder_k1_1_5&th=1) - Provides backup power to the RTC module to keep time when the main board is powered off.
* **Debugger:** [ST-Link V2 Programmer/Debugger](https://www.amazon.com/dp/B0C9PKJF2T?ref_=ppx_hzsearch_conn_dt_b_fed_asin_title_1&th=1) - Used for flashing the firmware and debugging the STM32.
* **Soldering Iron:** [Pinecil Smart Soldering Iron (BB2)](https://www.amazon.com/dp/B08PZBPXLZ?ref_=ppx_hzsearch_conn_dt_b_fed_asin_title_1&th=1) - Highly recommended portable soldering iron used for assembling the board.
