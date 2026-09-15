#include <Arduino.h>

// --- تعريف بنّات الموتورات الأربعة (كل عجلة على مخرج مستقل) ---

// 1. العجلة اليسار الأمامية (Front-Left) - الدرايفر الأول
#define FL_IN1  26
#define FL_IN2  25
#define FL_ENA  27

// 2. العجلة اليسار الخلفية (Back-Left) - الدرايفر الأول (تم اختيار GPIOs إضافية آمنة)
#define BL_IN1  16
#define BL_IN2  17
#define BL_ENB  4   // (أو استخدم أي بن PWM متاحة)

// 3. العجلة اليمين الأمامية (Front-Right) - الدرايفر الثاني
#define FR_IN1  33
#define FR_IN2  32
#define FR_EN_A 14

// 4. العجلة اليمين الخلفية (Back-Right) - الدرايفر الثاني
#define BR_IN1  18
#define BR_IN2  19
#define BR_EN_B 23

// --- تعريف الحساسات (نفس التوصيات السابقة) ---
#define FRONT_TRIG  12
#define FRONT_ECHO  13

void setup() {
    Serial.begin(115200);

    // ضبط بنات المواتير كلها كـ Outputs
    pinMode(FL_IN1, OUTPUT); pinMode(FL_IN2, OUTPUT); pinMode(FL_ENA, OUTPUT);
    pinMode(BL_IN1, OUTPUT); pinMode(BL_IN2, OUTPUT); pinMode(BL_ENB, OUTPUT);
    
    pinMode(FR_IN1, OUTPUT); pinMode(FR_IN2, OUTPUT); pinMode(FR_EN_A, OUTPUT);
    pinMode(BR_IN1, OUTPUT); pinMode(BR_IN2, OUTPUT); pinMode(BR_EN_B, OUTPUT);

    // ضبط بنات الحساسات
    pinMode(FRONT_TRIG, OUTPUT);
    pinMode(FRONT_ECHO, INPUT);

    Serial.println("Banana Destroyer 3000 - 4 Independent Motors Initialized!");
}

void loop() {
    // مثال: التحرك للأمام بكل العجلات بأقصى سرعة
    moveForward(200);
    delay(2000);
    
    // التوقف لثانية
    stopRobot();
    delay(1000);
}

// دالة التحرك للأمام لكل العجلات معاً
void setMotorSpeed(int speed) {
    // سرعات المحركات (يمكنك ضبطها هنا)
    analogWrite(FL_ENA, speed);
    analogWrite(BL_ENB, speed);
    analogWrite(FR_EN_A, speed);
    analogWrite(BR_EN_B, speed);
}

void moveForward(int speed) {
    setMotorSpeed(speed);

    // يسار أمامي وخلفي يتحركان للأمام
    digitalWrite(FL_IN1, HIGH); digitalWrite(FL_IN2, LOW);
    digitalWrite(BL_IN1, HIGH); digitalWrite(BL_IN2, LOW);

    // يمين أمامي وخلفي يتحركان للأمام
    digitalWrite(FR_IN1, HIGH); digitalWrite(FR_IN2, LOW);
    digitalWrite(BR_IN1, HIGH); digitalWrite(BR_IN2, LOW);
}

void stopRobot() {
    digitalWrite(FL_IN1, LOW); digitalWrite(FL_IN2, LOW);
    digitalWrite(BL_IN1, LOW); digitalWrite(BL_IN2, LOW);
    
    digitalWrite(FR_IN1, LOW); digitalWrite(FR_IN2, LOW);
    digitalWrite(BR_IN1, LOW); digitalWrite(BR_IN2, LOW);
}
