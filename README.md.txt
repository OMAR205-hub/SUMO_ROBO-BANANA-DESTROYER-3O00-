# Autonomous Sumo Robot - ESP32 Control System

Final competition repository for an autonomous Sumo Robot engineered for high-performance ring combat.

---

## 🛠 Hardware Specifications & Component List

| Component | Description / Specification | Quantity |
| :--- | :--- | :---: |
| **Microcontroller** | ESP32 DIP-30 CP2102 Micro USB Development Board | 1 |
| **Motor Drivers** | L298N Dual H-Bridge Motor Driver Module | 2 |
| **Motors** | 25GA370 DC Gear Motors (12V, 250 RPM) | 4 |
| **Wheels** | High-traction Robot Tires (65mm) | 4 |
| **Distance Sensors** | HC-SR04 Ultrasonic Distance Sensors | 3 |
| **Edge Sensors** | TCRT5000 IR Line Sensors (FC-123) | 4 |
| **Power Management** | LM2596 DC-DC Buck Converter (Step-Down) | 1 |
| **Battery Pack** | Rechargeable Li-ion 12V 3S-1P + BMS (1500mAh) | 1 |
| **Power Switch** | KCD3 ON/OFF Rocker Switch (3 Pins) | 1 |
| **Charger** | 3S Lithium Battery Charger (12.6V 2A, 5.5x2.1mm) | 1 |

---

## 🔌 Pin Mapping (ESP32)

### Motors (2 x L298N Drivers)
* **Left Motors:** `IN1 -> GPIO 26`, `IN2 -> GPIO 25`, `ENA -> GPIO 27`
* **Right Motors:** `IN1 -> GPIO 33`, `IN2 -> GPIO 32`, `ENB -> GPIO 14`

### Distance Sensors (HC-SR04 Ultrasonic)
* **Front:** `TRIG -> GPIO 16`, `ECHO -> GPIO 34`
* **Left:** `TRIG -> GPIO 17`, `ECHO -> GPIO 35`
* **Right:** `TRIG -> GPIO 18`, `ECHO -> GPIO 36`

### Edge Sensors (TCRT5000 IR)
* **Front-Left:** `GPIO 19`
* **Front-Right:** `GPIO 21`
* **Back-Left:** `GPIO 22`
* **Back-Right:** `GPIO 23`

---

## 🎯 Control Logic & Competition Rules
1. **Mandatory Start Delay:** 3-second delay upon activation before any movement occurs.
2. **Priority Edge Avoidance:** Immediate reversal and spin upon detecting the ring boundary (white line).
3. **Target Acquisition:** Autonomous 360° scanning using front, left, and right ultrasonic sensors.
4. **Adaptive Attack:** Dynamic speed adjustment (Search -> Normal -> Full Attack Speed) based on proximity.