#include <Arduino.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <AccelStepper.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <ESP32Servo.h>

// ========== PIN DEFINITIONS ==========
const int LED1_PIN = 15;
const int LED2_PIN = 16;
const int LED3_PIN = 17;
const int BUZZER_PIN = 7;
const int BUTTON1_PIN = 6;
const int BUTTON2_PIN = 5;
const int SERVO_PIN = 12;
const int STEP_PIN = 13;
const int DIR_PIN = 14;
const int POT_PIN = 3;
const int OLED_SCL = 9;
const int OLED_SDA = 8;
const int ENCODER_CLK = 10;
const int ENCODER_DT = 11;

// ========== OLED SETTINGS ==========
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

// ========== BUZZER SETTINGS ==========
const int BASE_FREQ = 2000;
const int TEMPO = 120;

// ========== MUSIC NOTES ==========
#define NOTE_C4  262
#define NOTE_D4  294
#define NOTE_E4  330
#define NOTE_F4  349
#define NOTE_G4  392
#define NOTE_A4  440
#define NOTE_B4  494
#define NOTE_C5  523
#define REST     0

// ========== MELODIES ==========
// Twinkle Twinkle Little Star
int melody0[] = {
  NOTE_C4, NOTE_C4, NOTE_G4, NOTE_G4, NOTE_A4, NOTE_A4, NOTE_G4,
  NOTE_F4, NOTE_F4, NOTE_E4, NOTE_E4, NOTE_D4, NOTE_D4, NOTE_C4,
  NOTE_G4, NOTE_G4, NOTE_F4, NOTE_F4, NOTE_E4, NOTE_E4, NOTE_D4,
  NOTE_G4, NOTE_G4, NOTE_F4, NOTE_F4, NOTE_E4, NOTE_E4, NOTE_D4,
  NOTE_C4, NOTE_C4, NOTE_G4, NOTE_G4, NOTE_A4, NOTE_A4, NOTE_G4,
  NOTE_F4, NOTE_F4, NOTE_E4, NOTE_E4, NOTE_D4, NOTE_D4, NOTE_C4
};

int dur0[] = {
  4,4,4,4,4,4,2,
  4,4,4,4,4,4,2,
  4,4,4,4,4,4,2,
  4,4,4,4,4,4,2,
  4,4,4,4,4,4,2,
  4,4,4,4,4,4,2
};

// Happy Birthday
int melody1[] = {
  NOTE_C4, NOTE_C4, NOTE_D4, NOTE_C4, NOTE_F4, NOTE_E4,
  NOTE_C4, NOTE_C4, NOTE_D4, NOTE_C4, NOTE_G4, NOTE_F4,
  NOTE_C4, NOTE_C4, NOTE_C5, NOTE_A4, NOTE_F4, NOTE_E4, NOTE_D4,
  NOTE_B4, NOTE_B4, NOTE_A4, NOTE_F4, NOTE_G4, NOTE_F4
};

int dur1[] = {
  8,8,4,4,4,2,
  8,8,4,4,4,2,
  8,8,4,4,4,4,2,
  8,8,4,4,4,2
};

// ========== GLOBAL OBJECTS ==========
AccelStepper stepper(AccelStepper::DRIVER, STEP_PIN, DIR_PIN);
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
Servo myservo;

// ========== TASK HANDLES ==========
TaskHandle_t TaskLedHandle = NULL;
TaskHandle_t TaskBuzzerHandle = NULL;
TaskHandle_t TaskButtonHandle = NULL;
TaskHandle_t TaskServoHandle = NULL;
TaskHandle_t TaskMotorHandle = NULL;
TaskHandle_t TaskPotHandle = NULL;
TaskHandle_t TaskOLEDHandle = NULL;
TaskHandle_t TaskEncoderHandle = NULL;

// ========== SHARED VARIABLES ==========
portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;

volatile int currentSong = 0;
volatile int encoderCount = 0;
volatile int targetAngle = 0;
volatile long targetSteps = 0;
volatile bool servoNewCommand = false;
volatile bool motorNewCommand = false;
volatile int activeCore = 0;

// ========== MELODY INFO ==========
int N0 = sizeof(melody0) / sizeof(melody0[0]);
int N1 = sizeof(melody1) / sizeof(melody1[0]);

// ========== HELPER FUNCTIONS ==========
int noteMsFromDuration(int durationValue) {
  int quarterMs = (60000 / TEMPO);
  float ms = quarterMs * (4.0 / durationValue);
  return (int)(ms + 0.5);
}

// ========== TASK 1: LED BLINK (Priority 1, Core 0) ==========
void TaskLed(void *pv) {
  Serial.printf("[Task 1] LED Blink - Core %d, Priority 1\n", xPortGetCoreID());

  const int led1Delay = 1000;
  const int led2Delay = 700;
  const int led3Delay = 500;

  unsigned long prev1 = 0, prev2 = 0, prev3 = 0;
  bool state1 = false, state2 = false, state3 = false;

  for (;;) {
    unsigned long now = millis();

    if (now - prev1 >= led1Delay) {
      prev1 = now;
      state1 = !state1;
      digitalWrite(LED1_PIN, state1);
    }

    if (now - prev2 >= led2Delay) {
      prev2 = now;
      state2 = !state2;
      digitalWrite(LED2_PIN, state2);
    }

    if (now - prev3 >= led3Delay) {
      prev3 = now;
      state3 = !state3;
      digitalWrite(LED3_PIN, state3);
    }

    vTaskDelay(pdMS_TO_TICKS(10));
  }
}

// ========== TASK 2: BUZZER MELODY (Priority 2, Core 0) ==========
void TaskBuzzer(void *pv) {
  Serial.printf("[Task 2] Buzzer Melody - Core %d, Priority 2\n", xPortGetCoreID());
  Serial.println("Ketik '0'/'twinkle' untuk lagu 0, '1'/'happy' untuk lagu 1");

  unsigned long noteEnd = 0;
  bool playing = false;
  int song = currentSong;
  int idx = 0;
  int pauseMs = 20;
  unsigned long pauseEnd = 0;
  bool inPause = false;

  for (;;) {
    unsigned long now = millis();

    // Cek input serial
    if (Serial.available()) {
      String line = Serial.readStringUntil('\n');
      line.trim();
      line.toLowerCase();
      
      if (line == "0" || line == "twinkle") {
        portENTER_CRITICAL(&mux);
        currentSong = 0;
        portEXIT_CRITICAL(&mux);
        Serial.println("-> Lagu: Twinkle Twinkle");
        song = 0;
        idx = 0;
        playing = false;
        inPause = false;
      } else if (line == "1" || line == "happy") {
        portENTER_CRITICAL(&mux);
        currentSong = 1;
        portEXIT_CRITICAL(&mux);
        Serial.println("-> Lagu: Happy Birthday");
        song = 1;
        idx = 0;
        playing = false;
        inPause = false;
      }
    }

    // Playback state machine
    if (!playing && !inPause) {
      int N = (song == 0) ? N0 : N1;
      if (idx >= N) {
        pauseEnd = now + 300;
        inPause = true;
        noTone(BUZZER_PIN);
      } else {
        int note = (song == 0) ? melody0[idx] : melody1[idx];
        int dur = (song == 0) ? dur0[idx] : dur1[idx];
        int ms = noteMsFromDuration(dur);
        
        if (note == REST) {
          noTone(BUZZER_PIN);
        } else {
          tone(BUZZER_PIN, note);
        }
        
        noteEnd = now + ms;
        playing = true;
      }
    } else if (playing) {
      if (now >= noteEnd) {
        noTone(BUZZER_PIN);
        inPause = true;
        pauseEnd = now + pauseMs;
        playing = false;
        idx++;
      }
    } else if (inPause) {
      if (now >= pauseEnd) {
        inPause = false;
        if (song != currentSong) {
          song = currentSong;
          idx = 0;
        }
      }
    }

    vTaskDelay(pdMS_TO_TICKS(8));
  }
}

// ========== TASK 3: PUSH BUTTON (Priority 3, Core 0) ==========
void TaskButton(void *pv) {
  Serial.printf("[Task 3] Push Button - Core %d, Priority 3\n", xPortGetCoreID());

  const unsigned long DEBOUNCE_MS = 50;
  int last1 = HIGH, stable1 = HIGH;
  int last2 = HIGH, stable2 = HIGH;
  unsigned long t1 = 0, t2 = 0;

  for (;;) {
    int r1 = digitalRead(BUTTON1_PIN);
    if (r1 != last1) t1 = millis();
    if ((millis() - t1) > DEBOUNCE_MS && r1 != stable1) {
      stable1 = r1;
      Serial.printf("[Button 1] %s\n", (stable1 == LOW ? "PRESSED" : "RELEASED"));
    }
    last1 = r1;

    int r2 = digitalRead(BUTTON2_PIN);
    if (r2 != last2) t2 = millis();
    if ((millis() - t2) > DEBOUNCE_MS && r2 != stable2) {
      stable2 = r2;
      Serial.printf("[Button 2] %s\n", (stable2 == LOW ? "PRESSED" : "RELEASED"));
    }
    last2 = r2;

    vTaskDelay(pdMS_TO_TICKS(10));
  }
}

// ========== TASK 4: SERVO MOTOR (Priority 2, Core 1) ==========
void TaskServo(void *pv) {
  Serial.printf("[Task 4] Servo Motor - Core %d, Priority 2\n", xPortGetCoreID());
  Serial.println("Ketik sudut 0-180 untuk servo");

  int currentAngle = 90;
  myservo.write(currentAngle);

  String buffer = "";

  for (;;) {
    while (Serial.available()) {
      char c = Serial.read();
      if (c == '\n' || c == '\r') {
        if (buffer.length() > 0) {
          int angle = buffer.toInt();
          if (angle >= 0 && angle <= 180) {
            portENTER_CRITICAL(&mux);
            targetAngle = angle;
            servoNewCommand = true;
            portEXIT_CRITICAL(&mux);
            Serial.printf("Servo -> %d°\n", angle);
          }
        }
        buffer = "";
      } else {
        buffer += c;
      }
    }

    if (servoNewCommand) {
      int target;
      portENTER_CRITICAL(&mux);
      target = targetAngle;
      servoNewCommand = false;
      portEXIT_CRITICAL(&mux);

      int step = (target > currentAngle) ? 1 : -1;
      for (int pos = currentAngle; pos != target; pos += step) {
        myservo.write(pos);
        vTaskDelay(pdMS_TO_TICKS(15));
      }
      currentAngle = target;
      Serial.printf("Servo selesai di %d°\n", currentAngle);
    }

    vTaskDelay(pdMS_TO_TICKS(10));
  }
}

// ========== TASK 5: STEPPER MOTOR (Priority 3, Core 1) ==========
void TaskMotor(void *pv) {
  Serial.printf("[Task 5] Stepper Motor - Core %d, Priority 3\n", xPortGetCoreID());
  Serial.println("Ketik jumlah langkah (+ atau -) untuk stepper");

  String buffer = "";

  for (;;) {
    while (Serial.available()) {
      char c = Serial.read();
      if (c == '\n' || c == '\r') {
        if (buffer.length() > 0) {
          long steps = buffer.toInt();
          portENTER_CRITICAL(&mux);
          targetSteps = steps;
          motorNewCommand = true;
          portEXIT_CRITICAL(&mux);
          Serial.printf("Stepper -> %ld langkah\n", steps);
        }
        buffer = "";
      } else {
        buffer += c;
      }
    }

    if (motorNewCommand) {
      long stepsToMove;
      portENTER_CRITICAL(&mux);
      stepsToMove = targetSteps;
      motorNewCommand = false;
      portEXIT_CRITICAL(&mux);

      long targetPos = stepper.currentPosition() + stepsToMove;
      stepper.moveTo(targetPos);
    }

    stepper.run();
    vTaskDelay(pdMS_TO_TICKS(1));
  }
}

// ========== TASK 6: POTENTIOMETER (Priority 1, Core 1) ==========
void TaskPot(void *pv) {
  // Delay awal agar tidak bentrok dengan task lain saat startup
  vTaskDelay(pdMS_TO_TICKS(500));
  
  Serial.printf("[Task 6] Potentiometer - Core %d, Priority 1\n", xPortGetCoreID());

  analogReadResolution(12);
  analogSetPinAttenuation(POT_PIN, ADC_11db);

  for (;;) {
    int potValue = analogRead(POT_PIN);
    float pct = (potValue / 4095.0f) * 100.0f;

    Serial.printf("[POT] Raw: %d, Persentase: %.1f%%\n", potValue, pct);

    vTaskDelay(pdMS_TO_TICKS(1000)); // Ubah dari 500ms ke 1000ms agar tidak terlalu sering print
  }
}

// ========== TASK 7: OLED DISPLAY (Priority 4, Core 0) ==========
void TaskOLED(void *pv) {
  Serial.printf("[Task 7] OLED Display - Core %d, Priority 4\n", xPortGetCoreID());
  Serial.println("Ketik '0' untuk tampil Core0, '1' untuk Core1");

  const char* txtCore0 = "RTOS Core 0";
  const char* txtCore1 = "RTOS Core 1";

  for (;;) {
    if (Serial.available()) {
      String s = Serial.readStringUntil('\n');
      s.trim();
      if (s == "0") {
        portENTER_CRITICAL(&mux);
        activeCore = 0;
        portEXIT_CRITICAL(&mux);
        Serial.println("OLED -> Core 0");
      } else if (s == "1") {
        portENTER_CRITICAL(&mux);
        activeCore = 1;
        portEXIT_CRITICAL(&mux);
        Serial.println("OLED -> Core 1");
      }
    }

    display.clearDisplay();
    display.setTextSize(2);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 20);
    
    const char* txt = (activeCore == 0) ? txtCore0 : txtCore1;
    display.print(txt);
    display.display();

    vTaskDelay(pdMS_TO_TICKS(500));
  }
}

// ========== TASK 8: ROTARY ENCODER (Priority 4, Core 1) ==========
void TaskEncoder(void *pv) {
  Serial.printf("[Task 8] Rotary Encoder - Core %d, Priority 4\n", xPortGetCoreID());
  Serial.println("Ketik 'reset' untuk reset encoder");

  String buffer = "";
  int lastState = digitalRead(ENCODER_CLK);

  for (;;) {
    while (Serial.available()) {
      char c = Serial.read();
      if (c == '\n' || c == '\r') {
        buffer.trim();
        if (buffer.equalsIgnoreCase("reset")) {
          portENTER_CRITICAL(&mux);
          encoderCount = 0;
          portEXIT_CRITICAL(&mux);
          Serial.println("Encoder direset ke 0");
        }
        buffer = "";
      } else {
        buffer += c;
      }
    }

    int currentState = digitalRead(ENCODER_CLK);
    if (currentState != lastState) {
      if (digitalRead(ENCODER_DT) != currentState) {
        portENTER_CRITICAL(&mux);
        encoderCount++;
        portEXIT_CRITICAL(&mux);
      } else {
        portENTER_CRITICAL(&mux);
        encoderCount--;
        portEXIT_CRITICAL(&mux);
      }
      Serial.printf("[Encoder] Count: %d\n", encoderCount);
    }
    lastState = currentState;

    vTaskDelay(pdMS_TO_TICKS(1));
  }
}

// ========== SETUP ==========
void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println("\n========================================");
  Serial.println("ESP32-S3 Dual Core - 8 Tasks FreeRTOS");
  Serial.println("========================================\n");

  // Pin Setup
  pinMode(LED1_PIN, OUTPUT);
  pinMode(LED2_PIN, OUTPUT);
  pinMode(LED3_PIN, OUTPUT);
  pinMode(BUTTON1_PIN, INPUT_PULLUP);
  pinMode(BUTTON2_PIN, INPUT_PULLUP);
  pinMode(ENCODER_CLK, INPUT_PULLUP);
  pinMode(ENCODER_DT, INPUT_PULLUP);

  // Buzzer Setup (menggunakan tone() standard Arduino)
  pinMode(BUZZER_PIN, OUTPUT);

  // Servo Setup
  myservo.setPeriodHertz(50);
  myservo.attach(SERVO_PIN, 500, 2400);
  myservo.write(90);

  // Stepper Setup
  stepper.setMaxSpeed(1000);
  stepper.setAcceleration(500);

  // OLED Setup
  Wire.begin(OLED_SDA, OLED_SCL);
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("OLED initialization failed!");
  } else {
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.println("ESP32-S3 RTOS");
    display.display();
    Serial.println("OLED initialized");
  }

  // ========== CREATE TASKS ==========
  Serial.println("Creating tasks...\n");
  delay(100);
  
  // CORE 0 Tasks (Lower priority untuk I/O)
  xTaskCreatePinnedToCore(TaskLed,    "LED_Task",    4096, NULL, 1, &TaskLedHandle,    0);
  delay(50);
  xTaskCreatePinnedToCore(TaskBuzzer, "Buzzer_Task", 4096, NULL, 2, &TaskBuzzerHandle, 0);
  delay(50);
  xTaskCreatePinnedToCore(TaskButton, "Button_Task", 4096, NULL, 3, &TaskButtonHandle, 0);
  delay(50);
  xTaskCreatePinnedToCore(TaskOLED,   "OLED_Task",   4096, NULL, 4, &TaskOLEDHandle,   0);
  delay(50);

  // CORE 1 Tasks (Higher priority untuk kontrol motor)
  xTaskCreatePinnedToCore(TaskPot,     "Pot_Task",     4096, NULL, 1, &TaskPotHandle,     1);
  delay(50);
  xTaskCreatePinnedToCore(TaskServo,   "Servo_Task",   4096, NULL, 2, &TaskServoHandle,   1);
  delay(50);
  xTaskCreatePinnedToCore(TaskMotor,   "Motor_Task",   4096, NULL, 3, &TaskMotorHandle,   1);
  delay(50);
  xTaskCreatePinnedToCore(TaskEncoder, "Encoder_Task", 4096, NULL, 4, &TaskEncoderHandle, 1);
  
  delay(200);
  Serial.println("\n========== All Tasks Created ==========");
  Serial.println("CORE 0: LED(P1), Buzzer(P2), Button(P3), OLED(P4)");
  Serial.println("CORE 1: Pot(P1), Servo(P2), Motor(P3), Encoder(P4)");
  Serial.println("=======================================\n");
  delay(100);
}

void loop() {
  // Loop kosong karena semua dihandle oleh tasks
  vTaskDelay(pdMS_TO_TICKS(1000));
}