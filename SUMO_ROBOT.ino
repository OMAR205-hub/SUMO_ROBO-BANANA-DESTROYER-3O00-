#include <Arduino.h>

// ============================================================================
// PIN CONFIGURATION
// ============================================================================

// Motor Driver Pins
#define LEFT_IN1    26
#define LEFT_IN2    25
#define LEFT_ENA    27

#define RIGHT_IN1   33
#define RIGHT_IN2   32
#define RIGHT_ENB   14

// Ultrasonic Sensor Pins
#define FRONT_TRIG  16
#define FRONT_ECHO  34

#define LEFT_TRIG   17
#define LEFT_ECHO   35

#define RIGHT_TRIG  18
#define RIGHT_ECHO  36

// IR Edge Sensor Pins (Digital)
#define IR_FRONT_LEFT   19
#define IR_FRONT_RIGHT  21
#define IR_BACK_LEFT    22
#define IR_BACK_RIGHT   23

// ============================================================================
// COMPETITION PARAMETERS
// ============================================================================

const unsigned long START_DELAY_MS = 3000;

const int SEARCH_SPEED = 150;
const int NORMAL_SPEED = 210;
const int ATTACK_SPEED = 255;
const int ESCAPE_SPEED = 255;

const float DETECTION_RANGE = 50.0;
const float CLOSE_RANGE = 20.0;

// Since we are using digitalRead, the threshold is typically HIGH (when detecting white line/edge) 
// or LOW depending on your sensor module. Adjust if needed.
const int IR_THRESHOLD = HIGH; 

// ============================================================================
// ROBOT STATE
// ============================================================================

enum RobotState
{
    STARTING,
    SEARCHING,
    ATTACKING,
    BACKING_UP,
    TURNING,
    EDGE_AVOIDANCE
};

RobotState currentState = STARTING;

// ============================================================================
// SENSOR DATA
// ============================================================================

float frontDistance = 300.0;
float leftDistance  = 300.0;
float rightDistance = 300.0;

int frontLeftIR  = LOW;
int frontRightIR = LOW;
int backLeftIR   = LOW;
int backRightIR  = LOW;

// ============================================================================
// SETUP
// ============================================================================

void setup()
{
    pinMode(LEFT_IN1, OUTPUT);
    pinMode(LEFT_IN2, OUTPUT);
    pinMode(LEFT_ENA, OUTPUT);

    pinMode(RIGHT_IN1, OUTPUT);
    pinMode(RIGHT_IN2, OUTPUT);
    pinMode(RIGHT_ENB, OUTPUT);

    pinMode(FRONT_TRIG, OUTPUT);
    pinMode(FRONT_ECHO, INPUT);
    pinMode(LEFT_TRIG, OUTPUT);
    pinMode(LEFT_ECHO, INPUT);
    pinMode(RIGHT_TRIG, OUTPUT);
    pinMode(RIGHT_ECHO, INPUT);

    pinMode(IR_FRONT_LEFT, INPUT);
    pinMode(IR_FRONT_RIGHT, INPUT);
    pinMode(IR_BACK_LEFT, INPUT);
    pinMode(IR_BACK_RIGHT, INPUT);

    // Stop motors initially
    digitalWrite(LEFT_IN1, LOW);
    digitalWrite(LEFT_IN2, LOW);
    digitalWrite(RIGHT_IN1, LOW);
    digitalWrite(RIGHT_IN2, LOW);
    analogWrite(LEFT_ENA, 0);
    analogWrite(RIGHT_ENB, 0);

    currentState = STARTING;
    delay(START_DELAY_MS);
    currentState = SEARCHING;
}

// ============================================================================
// MAIN CONTROL LOOP
// ============================================================================

void loop()
{
    // Edge detection has the highest priority
    frontLeftIR  = digitalRead(IR_FRONT_LEFT);
    frontRightIR = digitalRead(IR_FRONT_RIGHT);
    backLeftIR   = digitalRead(IR_BACK_LEFT);
    backRightIR  = digitalRead(IR_BACK_RIGHT);

    if (frontLeftIR == IR_THRESHOLD || frontRightIR == IR_THRESHOLD || 
        backLeftIR == IR_THRESHOLD || backRightIR == IR_THRESHOLD)
    {
        handleEdgeAvoidance();
        return;
    }

    // Check for an opponent
    frontDistance = measureDistance(FRONT_TRIG, FRONT_ECHO);
    leftDistance  = measureDistance(LEFT_TRIG, LEFT_ECHO);
    rightDistance = measureDistance(RIGHT_TRIG, RIGHT_ECHO);

    if (frontDistance <= DETECTION_RANGE || leftDistance <= DETECTION_RANGE || rightDistance <= DETECTION_RANGE)
    {
        attackOpponent();
    }
    else
    {
        searchForOpponent();
    }

    delay(10);
}

// ============================================================================
// MOTOR CONTROL FUNCTIONS
// ============================================================================

void setMotor(int motor, int speed, bool forward)
{
    speed = constrain(speed, 0, 255);

    if (motor == 0)
    {
        analogWrite(LEFT_ENA, speed);
        digitalWrite(LEFT_IN1, forward ? HIGH : LOW);
        digitalWrite(LEFT_IN2, forward ? LOW : HIGH);
    }
    else
    {
        analogWrite(RIGHT_ENB, speed);
        digitalWrite(RIGHT_IN1, forward ? HIGH : LOW);
        digitalWrite(RIGHT_IN2, forward ? LOW : HIGH);
    }
}

void moveForward(int speed)
{
    setMotor(0, speed, true);
    setMotor(1, speed, true);
    currentState = ATTACKING;
}

void moveBackward(int speed)
{
    setMotor(0, speed, false);
    setMotor(1, speed, false);
    currentState = BACKING_UP;
}

void turnLeft(int speed)
{
    setMotor(0, speed, false);
    setMotor(1, speed, true);
    currentState = TURNING;
}

void turnRight(int speed)
{
    setMotor(0, speed, true);
    setMotor(1, speed, false);
    currentState = TURNING;
}

void spinLeft(int speed)
{
    setMotor(0, speed, false);
    setMotor(1, speed, true);
    currentState = SEARCHING;
}

void spinRight(int speed)
{
    setMotor(0, speed, true);
    setMotor(1, speed, false);
    currentState = SEARCHING;
}

// ============================================================================
// ULTRASONIC SENSOR SYSTEM
// ============================================================================

float measureDistance(int trigPin, int echoPin)
{
    digitalWrite(trigPin, LOW);
    delayMicroseconds(2);
    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin, LOW);

    unsigned long duration = pulseIn(echoPin, HIGH, 25000);

    if (duration == 0) return 300.0;

    float distance = (duration * 0.0343) / 2.0;

    if (distance < 2.0 || distance > 300.0) return 300.0;

    return distance;
}

// ============================================================================
// EDGE AVOIDANCE
// ============================================================================

void handleEdgeAvoidance()
{
    currentState = EDGE_AVOIDANCE;

    if (frontLeftIR == IR_THRESHOLD && frontRightIR == IR_THRESHOLD)
    {
        moveBackward(ESCAPE_SPEED);
        delay(300);
        spinRight(ESCAPE_SPEED);
        delay(220);
        return;
    }

    if (frontLeftIR == IR_THRESHOLD)
    {
        moveBackward(ESCAPE_SPEED);
        delay(220);
        spinRight(ESCAPE_SPEED);
        delay(180);
        return;
    }

    if (frontRightIR == IR_THRESHOLD)
    {
        moveBackward(ESCAPE_SPEED);
        delay(220);
        spinLeft(ESCAPE_SPEED);
        delay(180);
        return;
    }

    if (backLeftIR == IR_THRESHOLD && backRightIR == IR_THRESHOLD)
    {
        moveForward(NORMAL_SPEED);
        delay(250);
        return;
    }

    if (backLeftIR == IR_THRESHOLD)
    {
        moveForward(NORMAL_SPEED);
        delay(150);
        turnRight(NORMAL_SPEED);
        delay(100);
        return;
    }

    if (backRightIR == IR_THRESHOLD)
    {
        moveForward(NORMAL_SPEED);
        delay(150);
        turnLeft(NORMAL_SPEED);
        delay(100);
        return;
    }
}

// ============================================================================
// ATTACK & SEARCH STRATEGY
// ============================================================================

void attackOpponent()
{
    if (frontDistance <= CLOSE_RANGE)
    {
        moveForward(ATTACK_SPEED);
    }
    else if (frontDistance <= DETECTION_RANGE)
    {
        moveForward(NORMAL_SPEED);
    }
    else if (leftDistance <= DETECTION_RANGE && leftDistance < rightDistance)
    {
        turnLeft(NORMAL_SPEED);
    }
    else if (rightDistance <= DETECTION_RANGE)
    {
        turnRight(NORMAL_SPEED);
    }
}

void searchForOpponent()
{
    static unsigned long searchTimer = 0;
    static bool rotateRight = true;

    if (searchTimer == 0) searchTimer = millis();

    if (millis() - searchTimer >= 2000)
    {
        rotateRight = !rotateRight;
        searchTimer = millis();
    }

    if (rotateRight) spinRight(SEARCH_SPEED);
    else spinLeft(SEARCH_SPEED);

    currentState = SEARCHING;
}
        spinLeft(SEARCH_SPEED);
    }


    currentState = SEARCHING;
}
