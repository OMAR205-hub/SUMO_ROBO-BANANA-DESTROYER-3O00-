```cpp
/*
 * ============================================================================
 *                           ESP32 SUMO ROBOT
 *                        FINAL COMPETITION CODE
 * ============================================================================
 *
 * Controller:
 *   ESP32
 *
 * Motor Drivers:
 *   2 x L298N
 *
 * Motors:
 *   4 x DC Gear Motors
 *   Left Front + Left Rear  -> L298N #1
 *   Right Front + Right Rear -> L298N #2
 *
 * Ultrasonic Sensors:
 *   Front / Left / Right
 *
 * IR Edge Sensors:
 *   Front Left / Front Right
 *   Back Left / Back Right
 *
 * Control Priority:
 *   1. Edge Protection
 *   2. Opponent Detection
 *   3. Attack
 *   4. Search
 *
 * ============================================================================
 */

#include <Arduino.h>


// ============================================================================
//                         COMPETITION PARAMETERS
// ============================================================================

// Official starting delay
const unsigned long START_DELAY_MS = 3000;

// Opponent detection limits
const int FRONT_ATTACK_DISTANCE = 35;
const int SIDE_DETECTION_DISTANCE = 45;

// Motor speeds
const int ATTACK_SPEED = 255;
const int SEARCH_SPEED = 175;
const int SIDE_SEARCH_SPEED = 190;
const int ESCAPE_SPEED = 255;
const int TURN_SPEED = 240;

// TCRT5000 logic
// Change to HIGH only if your sensor gives HIGH on the edge.
const int EDGE_DETECTED = LOW;


// ============================================================================
//                              MOTOR PINS
// ============================================================================

// ----------------------------- L298N #1 ------------------------------------
// LEFT SIDE
// Front Left + Back Left connected together

#define LEFT_IN1  26
#define LEFT_IN2  25
#define LEFT_EN   27


// ----------------------------- L298N #2 ------------------------------------
// RIGHT SIDE
// Front Right + Back Right connected together

#define RIGHT_IN1 33
#define RIGHT_IN2 32
#define RIGHT_EN  14


// ============================================================================
//                         ULTRASONIC SENSOR PINS
// ============================================================================

// FRONT
#define FRONT_TRIG 12
#define FRONT_ECHO 13

// LEFT
#define LEFT_TRIG  4
#define LEFT_ECHO 18

// RIGHT
#define RIGHT_TRIG 19
#define RIGHT_ECHO 5


// ============================================================================
//                           IR EDGE SENSOR PINS
// ============================================================================

// FRONT
#define IR_FRONT_LEFT  15
#define IR_FRONT_RIGHT 21

// BACK
#define IR_BACK_LEFT   22
#define IR_BACK_RIGHT  23


// ============================================================================
//                              PWM SETTINGS
// ============================================================================

const int PWM_FREQUENCY = 20000;
const int PWM_RESOLUTION = 8;


// ============================================================================
//                         FUNCTION DECLARATIONS
// ============================================================================

// Initialization
void setupMotorPins();
void setupSensorPins();
void setupPWM();

// Motor control
void setMotorSpeed(int leftSpeed, int rightSpeed);

void moveForward(int speed);
void moveBackward(int speed);

void spinLeft(int speed);
void spinRight(int speed);

void stopRobot();

// Edge detection
bool frontLeftEdge();
bool frontRightEdge();
bool backLeftEdge();
bool backRightEdge();

bool anyEdgeDetected();

// Edge escape
void escapeFromEdge();

// Ultrasonic
int getDistance(int trigPin, int echoPin);

int getFrontDistance();
int getLeftDistance();
int getRightDistance();

// Strategy
void attackOpponent();
void searchOpponent();
void attackLeft();
void attackRight();


// ============================================================================
//                                  SETUP
// ============================================================================

void setup()
{
    Serial.begin(115200);

    // -------------------- Initialize hardware ------------------------------

    setupMotorPins();
    setupSensorPins();
    setupPWM();

    // Safety: robot must be stopped at startup
    stopRobot();

    Serial.println();
    Serial.println("==============================================");
    Serial.println("           ESP32 SUMO ROBOT");
    Serial.println("          FINAL COMPETITION CODE");
    Serial.println("==============================================");
    Serial.println("System initialized successfully.");
    Serial.println();

    // -------------------- Competition start ---------------------------------

    Serial.print("Starting in ");
    Serial.print(START_DELAY_MS / 1000);
    Serial.println(" seconds...");

    delay(START_DELAY_MS);

    Serial.println("GO!");
    Serial.println();
}


// ============================================================================
//                                  LOOP
// ============================================================================

void loop()
{
    // ========================================================================
    // PRIORITY 1: EDGE PROTECTION
    // ========================================================================

    if (anyEdgeDetected())
    {
        escapeFromEdge();

        // Immediately return to the main strategy
        return;
    }


    // ========================================================================
    // PRIORITY 2: READ OPPONENT SENSORS
    // ========================================================================

    int frontDistance = getFrontDistance();
    int leftDistance  = getLeftDistance();
    int rightDistance = getRightDistance();


    // ========================================================================
    // PRIORITY 3: OPPONENT ATTACK
    // ========================================================================

    // Front opponent
    if (frontDistance > 0 &&
        frontDistance <= FRONT_ATTACK_DISTANCE)
    {
        attackOpponent();
        return;
    }


    // ========================================================================
    // SIDE DETECTION
    // ========================================================================

    // Opponent detected on the left
    if (leftDistance > 0 &&
        leftDistance <= SIDE_DETECTION_DISTANCE)
    {
        attackLeft();
        return;
    }


    // Opponent detected on the right
    if (rightDistance > 0 &&
        rightDistance <= SIDE_DETECTION_DISTANCE)
    {
        attackRight();
        return;
    }


    // ========================================================================
    // PRIORITY 4: SEARCH
    // ========================================================================

    searchOpponent();

    delay(10);
}


// ============================================================================
//                         MOTOR INITIALIZATION
// ============================================================================

void setupMotorPins()
{
    pinMode(LEFT_IN1, OUTPUT);
    pinMode(LEFT_IN2, OUTPUT);

    pinMode(RIGHT_IN1, OUTPUT);
    pinMode(RIGHT_IN2, OUTPUT);
}


// ============================================================================
//                         SENSOR INITIALIZATION
// ============================================================================

void setupSensorPins()
{
    // Ultrasonic
    pinMode(FRONT_TRIG, OUTPUT);
    pinMode(FRONT_ECHO, INPUT);

    pinMode(LEFT_TRIG, OUTPUT);
    pinMode(LEFT_ECHO, INPUT);

    pinMode(RIGHT_TRIG, OUTPUT);
    pinMode(RIGHT_ECHO, INPUT);

    digitalWrite(FRONT_TRIG, LOW);
    digitalWrite(LEFT_TRIG, LOW);
    digitalWrite(RIGHT_TRIG, LOW);


    // IR edge sensors
    pinMode(IR_FRONT_LEFT, INPUT);
    pinMode(IR_FRONT_RIGHT, INPUT);

    pinMode(IR_BACK_LEFT, INPUT);
    pinMode(IR_BACK_RIGHT, INPUT);
}


// ============================================================================
//                              PWM SETUP
// ============================================================================

void setupPWM()
{
    /*
     * ESP32 Arduino Core 3.x
     *
     * ledcAttach(pin, frequency, resolution)
     */

    ledcAttach(LEFT_EN, PWM_FREQUENCY, PWM_RESOLUTION);
    ledcAttach(RIGHT_EN, PWM_FREQUENCY, PWM_RESOLUTION);

    ledcWrite(LEFT_EN, 0);
    ledcWrite(RIGHT_EN, 0);
}


// ============================================================================
//                           MOTOR SPEED CONTROL
// ============================================================================

void setMotorSpeed(int leftSpeed, int rightSpeed)
{
    leftSpeed  = constrain(leftSpeed, 0, 255);
    rightSpeed = constrain(rightSpeed, 0, 255);

    ledcWrite(LEFT_EN, leftSpeed);
    ledcWrite(RIGHT_EN, rightSpeed);
}


// ============================================================================
//                              MOVE FORWARD
// ============================================================================

void moveForward(int speed)
{
    setMotorSpeed(speed, speed);

    // Left side forward
    digitalWrite(LEFT_IN1, HIGH);
    digitalWrite(LEFT_IN2, LOW);

    // Right side forward
    digitalWrite(RIGHT_IN1, HIGH);
    digitalWrite(RIGHT_IN2, LOW);
}


// ============================================================================
//                             MOVE BACKWARD
// ============================================================================

void moveBackward(int speed)
{
    setMotorSpeed(speed, speed);

    // Left side backward
    digitalWrite(LEFT_IN1, LOW);
    digitalWrite(LEFT_IN2, HIGH);

    // Right side backward
    digitalWrite(RIGHT_IN1, LOW);
    digitalWrite(RIGHT_IN2, HIGH);
}


// ============================================================================
//                               SPIN LEFT
// ============================================================================

void spinLeft(int speed)
{
    setMotorSpeed(speed, speed);

    // Left side backward
    digitalWrite(LEFT_IN1, LOW);
    digitalWrite(LEFT_IN2, HIGH);

    // Right side forward
    digitalWrite(RIGHT_IN1, HIGH);
    digitalWrite(RIGHT_IN2, LOW);
}


// ============================================================================
//                              SPIN RIGHT
// ============================================================================

void spinRight(int speed)
{
    setMotorSpeed(speed, speed);

    // Left side forward
    digitalWrite(LEFT_IN1, HIGH);
    digitalWrite(LEFT_IN2, LOW);

    // Right side backward
    digitalWrite(RIGHT_IN1, LOW);
    digitalWrite(RIGHT_IN2, HIGH);
}


// ============================================================================
//                              STOP ROBOT
// ============================================================================

void stopRobot()
{
    // Disable PWM
    ledcWrite(LEFT_EN, 0);
    ledcWrite(RIGHT_EN, 0);

    // Stop left side
    digitalWrite(LEFT_IN1, LOW);
    digitalWrite(LEFT_IN2, LOW);

    // Stop right side
    digitalWrite(RIGHT_IN1, LOW);
    digitalWrite(RIGHT_IN2, LOW);
}


// ============================================================================
//                         IR EDGE SENSOR FUNCTIONS
// ============================================================================

bool frontLeftEdge()
{
    return digitalRead(IR_FRONT_LEFT) == EDGE_DETECTED;
}


bool frontRightEdge()
{
    return digitalRead(IR_FRONT_RIGHT) == EDGE_DETECTED;
}


bool backLeftEdge()
{
    return digitalRead(IR_BACK_LEFT) == EDGE_DETECTED;
}


bool backRightEdge()
{
    return digitalRead(IR_BACK_RIGHT) == EDGE_DETECTED;
}


bool anyEdgeDetected()
{
    return frontLeftEdge()  ||
           frontRightEdge() ||
           backLeftEdge()   ||
           backRightEdge();
}


// ============================================================================
//                           EDGE ESCAPE STRATEGY
// ============================================================================

void escapeFromEdge()
{
    bool frontLeft  = frontLeftEdge();
    bool frontRight = frontRightEdge();

    bool backLeft   = backLeftEdge();
    bool backRight  = backRightEdge();


    // ========================================================================
    // FRONT EDGE
    // ========================================================================

    if (frontLeft || frontRight)
    {
        // Move away from the edge
        moveBackward(ESCAPE_SPEED);
        delay(350);

        stopRobot();
        delay(20);


        // If the left front sensor detected the edge,
        // turn toward the right.
        if (frontLeft && !frontRight)
        {
            spinRight(TURN_SPEED);
            delay(300);
        }

        // If the right front sensor detected the edge,
        // turn toward the left.
        else if (frontRight && !frontLeft)
        {
            spinLeft(TURN_SPEED);
            delay(300);
        }

        // Both front sensors detected edge
        else
        {
            spinRight(TURN_SPEED);
            delay(350);
        }
    }


    // ========================================================================
    // BACK EDGE
    // ========================================================================

    else if (backLeft || backRight)
    {
        // Move forward away from the rear edge
        moveForward(ESCAPE_SPEED);
        delay(350);

        stopRobot();
        delay(20);


        // Rear-left edge -> turn right
        if (backLeft && !backRight)
        {
            spinRight(TURN_SPEED);
            delay(300);
        }

        // Rear-right edge -> turn left
        else if (backRight && !backLeft)
        {
            spinLeft(TURN_SPEED);
            delay(300);
        }

        // Both rear sensors detected edge
        else
        {
            spinLeft(TURN_SPEED);
            delay(350);
        }
    }


    stopRobot();
    delay(20);
}


// ============================================================================
//                       ULTRASONIC DISTANCE FUNCTION
// ============================================================================

int getDistance(int trigPin, int echoPin)
{
    // Clear trigger
    digitalWrite(trigPin, LOW);
    delayMicroseconds(2);

    // 10 us trigger pulse
    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin, LOW);


    // Read echo
    unsigned long duration =
        pulseIn(echoPin, HIGH, 12000);


    // No echo
    if (duration == 0)
    {
        return -1;
    }


    // Convert microseconds to centimeters
    int distance = duration * 0.0343 / 2;


    // Reject invalid values
    if (distance <= 0 || distance > 200)
    {
        return -1;
    }


    return distance;
}


// ============================================================================
//                          SENSOR READ FUNCTIONS
// ============================================================================

int getFrontDistance()
{
    return getDistance(FRONT_TRIG, FRONT_ECHO);
}


int getLeftDistance()
{
    return getDistance(LEFT_TRIG, LEFT_ECHO);
}


int getRightDistance()
{
    return getDistance(RIGHT_TRIG, RIGHT_ECHO);
}


// ============================================================================
//                             ATTACK STRATEGY
// ============================================================================

void attackOpponent()
{
    // Maximum forward power
    moveForward(ATTACK_SPEED);
}


void attackLeft()
{
    /*
     * Opponent detected on the left.
     *
     * Rotate toward the opponent first,
     * then continue forward.
     */

    spinLeft(SIDE_SEARCH_SPEED);
    delay(120);

    moveForward(ATTACK_SPEED);
}


void attackRight()
{
    /*
     * Opponent detected on the right.
     *
     * Rotate toward the opponent first,
     * then continue forward.
     */

    spinRight(SIDE_SEARCH_SPEED);
    delay(120);

    moveForward(ATTACK_SPEED);
}


// ============================================================================
//                              SEARCH STRATEGY
// ============================================================================

void searchOpponent()
{
    /*
     * Continuous rotation allows the three ultrasonic sensors
     * to scan the arena for the opponent.
     */

    spinLeft(SEARCH_SPEED);
}
```

### الـPinout المستخدم في الكود

| Component          | Signal  | ESP32 GPIO |
| ------------------ | ------- | ---------: |
| **Left L298N**     | IN1     |     **26** |
|                    | IN2     |     **25** |
|                    | ENA     |     **27** |
| **Right L298N**    | IN1     |     **33** |
|                    | IN2     |     **32** |
|                    | ENA/ENB |     **14** |
| **Front HC-SR04**  | TRIG    |     **12** |
|                    | ECHO    |     **13** |
| **Left HC-SR04**   | TRIG    |      **4** |
|                    | ECHO    |     **18** |
| **Right HC-SR04**  | TRIG    |     **19** |
|                    | ECHO    |      **5** |
| **Front Left IR**  | S       |     **15** |
| **Front Right IR** | S       |     **21** |
| **Back Left IR**   | S       |     **22** |
| **Back Right IR**  | S       |     **23** |

### الاستراتيجية بقت كده

**1. Edge Detection أولًا**
أي TCRT5000 يكتشف الحافة → الروبوت يهرب فورًا، وبيحدد اتجاه الهروب حسب مكان الحساس.

**2. Front Opponent**
لو الـFront ultrasonic شاف الخصم على ≤ **35 cm** → هجوم بأقصى سرعة.

**3. Side Opponent**
لو الـLeft أو Right ultrasonic شاف الخصم على ≤ **45 cm** → يلف ناحيته ثم يهجم.

**4. Search**
لو مفيش خصم → دوران مستمر للبحث.

### ⚠️ مهم جدًا قبل المسابقة

فيه **3 حاجات لازم تتأكد منها فعليًا**:

1. **TCRT5000:** الكود يفترض أن `LOW = Edge Detected`. لو قراءتك معكوسة غيّر:

```cpp
const int EDGE_DETECTED = LOW;
```

إلى:

```cpp
const int EDGE_DETECTED = HIGH;
```

2. **اتجاه المواتير:** جرّب `moveForward()` والعجل مرفوع. لازم الأربع مواتير يتحركوا بحيث الروبوت يتقدم. لو ناحية كاملة عكس، نعكس `IN1/IN2` للناحية دي.

3. **HC-SR04 ECHO:** الثلاثة `ECHO` لازم يدخلوا للـESP32 بحد أقصى **3.3V**. استخدم Voltage Divider، خصوصًا:
   `GPIO13`, `GPIO18`, `GPIO5`.

**والـPinout نفسه في الكود مطابق تمامًا للـPinout الذي أرسلته، ولم أغير أي GPIO.**
