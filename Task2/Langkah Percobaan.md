# ESP32-S3 Dual Core - 8 Tasks FreeRTOS

**Judul:** Implementasi 8 Task dengan Priority Scheduling pada ESP32-S3 Dual Core menggunakan FreeRTOS

---

## Tujuan

* Mengimplementasikan sistem multitasking dengan 8 task berbeda pada ESP32-S3 dual core
* Mempelajari konsep priority scheduling di FreeRTOS
* Memahami distribusi task pada dual core processor
* Menguji komunikasi antar task menggunakan shared variables dan mutex
* Mengamati performa dan responsifitas sistem dengan berbagai priority level

---

## Alat & Simulasi

**Platform:** [Wokwi Simulator – ESP32-S3 8 Tasks FreeRTOS](https://wokwi.com/projects/YOUR_PROJECT_ID)

**Komponen:**

* ESP32-S3 DevKit
* 3x LED (Merah, Kuning, Hijau)
* Buzzer Piezo
* 2x Push Button
* Servo Motor SG90
* Stepper Motor dengan Driver
* Potentiometer 10kΩ
* Rotary Encoder
* OLED Display SSD1306 (128x64)
* Resistor dan kabel jumper

**Koneksi:**

| Komponen           | ESP32-S3 Pin | Keterangan                    |
| ------------------ | ------------ | ----------------------------- |
| LED 1 (Merah)      | GPIO 15      | Blink 1000ms                  |
| LED 2 (Kuning)     | GPIO 16      | Blink 700ms                   |
| LED 3 (Hijau)      | GPIO 17      | Blink 500ms                   |
| Buzzer             | GPIO 7       | Melody player                 |
| Button 1           | GPIO 6       | Input dengan pull-up internal |
| Button 2           | GPIO 5       | Input dengan pull-up internal |
| Servo (PWM)        | GPIO 12      | Kontrol sudut 0-180°          |
| Stepper DIR        | GPIO 14      | Direction pin                 |
| Stepper STEP       | GPIO 13      | Step pulse pin                |
| Potentiometer      | GPIO 3 (ADC) | Analog input 0-4095           |
| Encoder CLK        | GPIO 10      | Clock signal                  |
| Encoder DT         | GPIO 11      | Data signal                   |
| OLED SDA           | GPIO 8       | I2C Data                      |
| OLED SCL           | GPIO 9       | I2C Clock                     |
| VCC (5V)           | 5V           | Power supply                  |
| GND                | GND          | Ground bersama                |

---

## Arsitektur Task

### **CORE 0 - I/O & Display Tasks**

| Task               | Priority | Fungsi                                       | Delay       |
| ------------------ | -------- | -------------------------------------------- | ----------- |
| Task 1: LED Blink  | 1        | Blink 3 LED dengan interval berbeda          | 10ms        |
| Task 2: Buzzer     | 2        | Memutar melodi (Twinkle/Happy Birthday)      | 8ms         |
| Task 3: Button     | 3        | Monitoring 2 push button dengan debouncing   | 10ms        |
| Task 7: OLED       | 4        | Menampilkan info core di display             | 500ms       |

### **CORE 1 - Motor Control & Sensor Tasks**

| Task               | Priority | Fungsi                                       | Delay       |
| ------------------ | -------- | -------------------------------------------- | ----------- |
| Task 6: Pot        | 1        | Membaca nilai potentiometer (ADC)            | 1000ms      |
| Task 4: Servo      | 2        | Kontrol posisi servo motor                   | 10ms        |
| Task 5: Stepper    | 3        | Kontrol stepper motor (step & direction)     | 1ms         |
| Task 8: Encoder    | 4        | Membaca rotary encoder (up/down counter)     | 1ms         |

---

## Langkah Percobaan

### 1. **Setup Simulasi**
   * Buka link simulasi Wokwi
   * Tekan **Run Simulation** untuk memulai
   * Buka **Serial Monitor** (baud rate **115200**)

### 2. **Pengujian Task LED (Core 0)**
   * Amati 3 LED berkedip dengan interval berbeda:
     * LED 1: 1000ms (merah)
     * LED 2: 700ms (kuning)
     * LED 3: 500ms (hijau)

### 3. **Pengujian Task Buzzer (Core 0)**
   Masukkan perintah di Serial Monitor:
   * `0` atau `twinkle` → Memutar lagu "Twinkle Twinkle Little Star"
   * `1` atau `happy` → Memutar lagu "Happy Birthday"

### 4. **Pengujian Task Button (Core 0)**
   * Tekan Button 1 (GPIO 6) → Serial menampilkan "PRESSED/RELEASED"
   * Tekan Button 2 (GPIO 5) → Serial menampilkan "PRESSED/RELEASED"

### 5. **Pengujian Task OLED (Core 0)**
   Masukkan perintah di Serial Monitor:
   * `0` → Tampilkan "RTOS Core 0" di OLED
   * `1` → Tampilkan "RTOS Core 1" di OLED

### 6. **Pengujian Task Potentiometer (Core 1)**
   * Putar potentiometer di simulator
   * Amati nilai RAW (0-4095) dan persentase (0-100%) di Serial Monitor

### 7. **Pengujian Task Servo (Core 1)**
   Masukkan sudut (0-180) di Serial Monitor:
   * `90` → Servo bergerak ke 90°
   * `0` → Servo bergerak ke 0°
   * `180` → Servo bergerak ke 180°

### 8. **Pengujian Task Stepper (Core 1)**
   Masukkan jumlah langkah di Serial Monitor:
   * `200` → Stepper bergerak 200 langkah CW
   * `-200` → Stepper bergerak 200 langkah CCW
   * `1000` → Stepper bergerak 1000 langkah

### 9. **Pengujian Task Encoder (Core 1)**
   * Putar encoder CW → Counter bertambah
   * Putar encoder CCW → Counter berkurang
   * Ketik `reset` → Counter kembali ke 0

---

## Hasil Pengamatan

### **Output Serial Monitor Saat Startup:**

```
========================================
ESP32-S3 Dual Core - 8 Tasks FreeRTOS
========================================

OLED initialized
Creating tasks...

[Task 1] LED Blink - Core 0, Priority 1
[Task 2] Buzzer Melody - Core 0, Priority 2
Ketik '0'/'twinkle' untuk lagu 0, '1'/'happy' untuk lagu 1
[Task 3] Push Button - Core 0, Priority 3
[Task 7] OLED Display - Core 0, Priority 4
Ketik '0' untuk tampil Core0, '1' untuk Core1
[Task 4] Servo Motor - Core 1, Priority 2
Ketik sudut 0-180 untuk servo
[Task 5] Stepper Motor - Core 1, Priority 3
Ketik jumlah langkah (+ atau -) untuk stepper
[Task 8] Rotary Encoder - Core 1, Priority 4
Ketik 'reset' untuk reset encoder

========== All Tasks Created ==========
CORE 0: LED(P1), Buzzer(P2), Button(P3), OLED(P4)
CORE 1: Pot(P1), Servo(P2), Motor(P3), Encoder(P4)
=======================================

[Task 6] Potentiometer - Core 1, Priority 1
[POT] Raw: 0, Persentase: 0.0%
```

### **Tabel Hasil Pengujian:**

| Perintah Serial | Task Terkait | Respon di Serial Monitor                      |
| --------------- | ------------ | --------------------------------------------- |
| `0`             | Buzzer       | "-> Lagu: Twinkle Twinkle"                    |
| `1`             | Buzzer       | "-> Lagu: Happy Birthday"                     |
| `90`            | Servo        | "Servo -> 90°", "Servo selesai di 90°"        |
| `200`           | Stepper      | "Stepper -> 200 langkah"                      |
| `-100`          | Stepper      | "Stepper -> -100 langkah"                     |
| `0`             | OLED         | "OLED -> Core 0"                              |
| `1`             | OLED         | "OLED -> Core 1"                              |
| `reset`         | Encoder      | "Encoder direset ke 0"                        |
| Button Press    | Button       | "[Button 1] PRESSED"                          |
| Encoder Rotate  | Encoder      | "[Encoder] Count: 5"                          |
| Pot Rotate      | Pot          | "[POT] Raw: 2048, Persentase: 50.0%"          |

---

## Analisis Priority Scheduling

### **Konsep Priority di FreeRTOS:**

Priority yang lebih **TINGGI** (nilai lebih besar) memiliki **preemptive** terhadap priority rendah:

* **Priority 4**: Task paling penting, akan preempt priority 1-3
* **Priority 3**: Preempt priority 1-2
* **Priority 2**: Preempt priority 1
* **Priority 1**: Priority terendah

### **Strategi Distribusi Task:**

**CORE 0 (I/O & Display):**
- Tugas dengan I/O yang tidak time-critical
- Display dan user interface
- Priority rendah untuk LED, meningkat ke OLED

**CORE 1 (Motor Control):**
- Tugas yang membutuhkan timing presisi
- Kontrol motor real-time
- Priority rendah untuk sensor, tinggi untuk motor control

### **Penggunaan Mutex:**

Mutex `portMUX_TYPE mux` digunakan untuk melindungi shared variables:
- `currentSong` (buzzer)
- `encoderCount` (encoder)
- `targetAngle` (servo)
- `targetSteps` (stepper)
- `activeCore` (OLED)

---

## Fitur-Fitur Penting

### ✅ **Thread-Safe Communication**
- Menggunakan `portENTER_CRITICAL()` dan `portEXIT_CRITICAL()`
- Melindungi akses ke shared variables

### ✅ **Non-Blocking Input**
- Serial input di-handle tanpa blocking task lain
- Menggunakan `Serial.available()` dan buffer string

### ✅ **Smooth Motor Control**
- Servo bergerak smooth step-by-step (15ms delay)
- Stepper menggunakan `AccelStepper` untuk acceleration

### ✅ **Debouncing**
- Button dengan debouncing 50ms untuk stabilitas
- Encoder dengan polling untuk deteksi perubahan

### ✅ **Power Management**
- `vTaskDelay()` di setiap task untuk yield CPU
- Mencegah task monopoli processor

---

## Struktur Kode

```
ESP32-S3_8Tasks_FreeRTOS/
│
├── main.ino                 # Kode utama
│
├── Pin Definitions          # Definisi semua pin GPIO
├── Global Objects           # Stepper, Servo, Display, dll
├── Task Handles             # Handle untuk semua 8 task
├── Shared Variables         # Variabel yang di-share antar task
│
├── Task Functions:
│   ├── TaskLed()           # Core 0, Priority 1
│   ├── TaskBuzzer()        # Core 0, Priority 2
│   ├── TaskButton()        # Core 0, Priority 3
│   ├── TaskServo()         # Core 1, Priority 2
│   ├── TaskMotor()         # Core 1, Priority 3
│   ├── TaskPot()           # Core 1, Priority 1
│   ├── TaskOLED()          # Core 0, Priority 4
│   └── TaskEncoder()       # Core 1, Priority 4
│
├── setup()                  # Inisialisasi hardware & create tasks
└── loop()                   # Kosong (handled by tasks)
```

---

## Troubleshooting

| Problem                          | Solusi                                          |
| -------------------------------- | ----------------------------------------------- |
| Task tidak berjalan              | Cek stack size (min 4096 bytes)                 |
| Serial Monitor berantakan        | Tambah delay antar task creation                |
| Motor tidak bergerak             | Cek koneksi pin dan voltage supply              |
| OLED tidak muncul                | Cek I2C address (default 0x3C)                  |
| Encoder tidak akurat             | Tambah delay atau gunakan interrupt             |
| Buzzer tidak bunyi               | Pastikan pin support PWM/tone()                 |

---

## Kesimpulan

Percobaan ini berhasil mengimplementasikan:

1. ✅ **8 Task FreeRTOS** berjalan simultan pada ESP32-S3 dual core
2. ✅ **Priority Scheduling** dengan 4 level priority (P1-P4)
3. ✅ **Core Pinning** - Task terdistribusi optimal di 2 core
4. ✅ **Thread-Safe** communication dengan mutex
5. ✅ **Real-time Control** untuk servo dan stepper motor
6. ✅ **User Interface** via Serial Monitor dan OLED display

**Manfaat:**
- Sistem responsif dan multitasking efisien
- Kontrol motor presisi tanpa blocking task lain
- Mudah untuk scale dan tambah task baru
- Demonstrasi konsep FreeRTOS yang komprehensif

---

## Gambar Demo

### Diagram Blok Sistem
```
┌─────────────────────────────────────────────────┐
│           ESP32-S3 Dual Core System             │
├─────────────────────┬───────────────────────────┤
│     CORE 0          │        CORE 1             │
│  (I/O & Display)    │    (Motor & Sensor)       │
├─────────────────────┼───────────────────────────┤
│ P1: LED Blink       │ P1: Potentiometer         │
│ P2: Buzzer Melody   │ P2: Servo Motor           │
│ P3: Push Button     │ P3: Stepper Motor         │
│ P4: OLED Display    │ P4: Rotary Encoder        │
└─────────────────────┴───────────────────────────┘
          ↓                       ↓
    Shared Variables (Protected by Mutex)
```

### Screenshot Serial Monitor

### Video Demo

---

## Referensi

* [FreeRTOS Documentation](https://www.freertos.org/Documentation/RTOS_book.html)
* [ESP32-S3 Datasheet](https://www.espressif.com/sites/default/files/documentation/esp32-s3_datasheet_en.pdf)
* [AccelStepper Library](http://www.airspayce.com/mikem/arduino/AccelStepper/)
* [Adafruit SSD1306 Library](https://github.com/adafruit/Adafruit_SSD1306)

---

## Lisensi

MIT License - Bebas digunakan untuk pembelajaran dan pengembangan

---

**Dibuat oleh:** [Ihsanta Zaki]  
**Tanggal:** [11/11/2025]  
**Platform:** ESP32-S3 + FreeRTOS + Wokwi Simulator