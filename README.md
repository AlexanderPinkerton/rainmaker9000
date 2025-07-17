# 🌧️ Rainmaker 9000

**Rainmaker 9000** is a gravity-fed, automatic plant watering system powered by an ESP32 microcontroller with a touchscreen interface. It lets you configure and automate watering schedules for 2 independently controlled solenoid valves—no plumbing skills required.

This project is ideal for makers, tinkerers, and home gardeners who want to combine DIY electronics with smart automation.

---

## 🛠️ Features

* 💧 Gravity-fed system for silent, energy-efficient watering
* ⏱️ Per-valve scheduling: set volume in mL per day or week
* 🔌 Touchscreen interface with intuitive controls (built using LVGL)
* 🧰 Fully open source: firmware, 3D models, and schematic included

---

## 📁 Repository Structure

```
rainmaker9000/
├── rainmaker9000.ino        # Arduino firmware for the ESP32
├── models/                  # 3D-printable STL files for the housing & modules
├── schematic.png            # Wiring diagram for electronics
├── image1.png               # Photo of the completed project
└── README.md                # This file
```

---

## 📸 Images

### 🔌 System Schematic

![Wiring Schematic](./schematic.png)

### 🌿 Completed Build

![Finished Project](./image1.png)

### Check out the youtube video!
### [Youtube Link](https://www.youtube.com/watch?v=TsRYO3gziak)

### You can also find the project on Printables
### [Printables Link](https://www.printables.com/model/1356419-rainmaker-9000-a-diy-smart-plant-watering-system)

---

## 🧠 How It Works

1. A **top-mounted water reservoir** feeds a water rail via gravity.
2. The ESP32 controls 12V relays which then control **solenoid valves**.
4. The **touchscreen interface** allows users to configure:
   * Milliliters per watering cycle
   * Frequency: daily or weekly
5. The system saves user settings and tracks the last watering time.

---

## 🧪 Getting Started

### 📦 What You Need

* 1x ESP32 CYD Dev Board with touchscreen - [Amazon Link](https://www.amazon.com/dp/B0CLR7MQ91?ref=ppx_yo2ov_dt_b_fed_asin_title&th=1)
* 12V DC power supply - [Amazon Link](https://www.amazon.com/dp/B01GEA8PQA?ref=ppx_yo2ov_dt_b_fed_asin_title)
* 2x 12V Solenoid valves - [Amazon Link](https://www.amazon.com/dp/B084YTNG2P?ref=ppx_yo2ov_dt_b_fed_asin_title)
* 2x 12V Relay modules - [Amazon Link](https://www.amazon.com/dp/B095YFJ69T?ref=ppx_yo2ov_dt_b_fed_asin_title&th=1)
* 1/4 OD Tubing & fittings - [Amazon Link](https://www.amazon.com/dp/B08BLC8PZ2?ref=ppx_yo2ov_dt_b_fed_asin_title&th=1)
* 5V Waterproof Buck Converter - [Amazon Link](https://www.amazon.com/dp/B0CMZWN7WS?ref=ppx_yo2ov_dt_b_fed_asin_title)
* Random screws from your garage
* 3D-printed housing (in `models/`)

### 🚀 Upload Firmware

1. Open `rainmaker9000.ino` in Arduino IDE.
2. Install dependencies (LVGL v9+, TFT_eSPI, XPT2046_Touchscreen, etc.) - You will need to configure TFT_eSPI for this project.
3. Set your board and port.
4. Upload the firmware to your ESP32.

---

## 🖨️ 3D Printing

All STL files for the Rainmaker 9000 modular enclosure and parts are in the `/models` folder. Files are optimized for FDM printing.

---

## 🧩 Roadmap / Coming Soon

* Modular design with expanable numbers of valves
* Moisture sensor integration
* Wi-Fi + mobile dashboard (optional module)
* Kit version for public release

---

## 📜 License

This project is licensed under the MIT License.
Feel free to use, modify, and distribute it.

---

## 🤝 Contributing

Pull requests, bug reports, and suggestions are welcome!
Feel free to fork this repo or open an issue if you have ideas or need help.
