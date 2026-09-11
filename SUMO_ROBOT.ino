
#include <Arduino.h>


// ============================================================================
// PIN CONFIGURATION
// ============================================================================

// ----------------------------------------------------------------------------
// Motor Driver Pins
// ----------------------------------------------------------------------------

// Left side motors
#define LEFT_IN1    26
#define LEFT_IN2    25
#define LEFT_ENA    27

// Right side motors
#define RIGHT_IN1   33
#define RIGHT_IN2   32
#define RIGHT_ENB   14


// ----------------------------------------------------------------------------
// Ultrasonic Sensor Pins
// ----------------------------------------------------------------------------

// Front sensor
#define FRONT_TRIG  16
#define FRONT_ECHO  34

// Left sensor
#define LEFT_TRIG   17
#define LEFT_ECHO   35

// Right sensor
#define RIGHT_TRIG  18
#define RIGHT_ECHO  36


// ----------------------------------------------------------------------------
// IR Edge Sensor Pins
// ----------------------------------------------------------------------------

#define IR_FRONT_LEFT   19
#define IR_FRONT_RIGHT  21
#define IR_BACK_LEFT    22
#define IR_BACK_RIGHT   23


// ============================================================================
// COMPETITION PARAMETERS
// ============================================================================

// Maximum permitted start delay is 5 seconds.
// The robot starts autonomously after 3 seconds.
const unsigned long START_DELAY_MS = 3000;


// Motor speed settings
const int SEARCH_SPEED = 150;
const int NORMAL_SPEED = 210;
const int ATTACK_SPEED = 255;
const int ESCAPE_SPEED = 255;


// Opponent detection distances
const float DETECTION_RANGE = 50.0;
const float CLOSE_RANGE = 20.0;


// IR edge detection threshold
//
// This value must be calibrated using the actual competition surface.
const int IR_THRESHOLD = 2000;


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

int frontLeftIR  = 0;
int frontRightIR = 0;
int backLeftIR   = 0;
int backRightIR  = 0;


// ============================================================================
// SETUP
// ============================================================================

void setup()
{
    initializeMotorPins();
    initializeUltrasonicPins();
    initializeIRPins();

    stopMotors();

    currentState = STARTING;

    // Competition start delay.
    // The robot remains stationary during this period.
    delay(START_DELAY_MS);

    currentState = SEARCHING;
}


// ============================================================================
// MAIN CONTROL LOOP
// ============================================================================

void loop()
{
    // Edge detection always has the highest priority.
    updateIRSensors();

    if (isEdgeDetected())
    {
        handleEdgeAvoidance();
        return;
    }


    // Check for an opponent.
    updateUltrasonicSensors();

    if (isOpponentDetected())
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
// INITIALIZATION
// ============================================================================

void initializeMotorPins()
{
    pinMode(LEFT_IN1, OUTPUT);
    pinMode(LEFT_IN2, OUTPUT);
    pinMode(LEFT_ENA, OUTPUT);

    pinMode(RIGHT_IN1, OUTPUT);
    pinMode(RIGHT_IN2, OUTPUT);
    pinMode(RIGHT_ENB, OUTPUT);
}


void initializeUltrasonicPins()
{
    pinMode(FRONT_TRIG, OUTPUT);
    pinMode(FRONT_ECHO, INPUT);

    pinMode(LEFT_TRIG, OUTPUT);
    pinMode(LEFT_ECHO, INPUT);

    pinMode(RIGHT_TRIG, OUTPUT);
    pinMode(RIGHT_ECHO, INPUT);
}


void initializeIRPins()
{
    pinMode(IR_FRONT_LEFT, INPUT);
    pinMode(IR_FRONT_RIGHT, INPUT);

    pinMode(IR_BACK_LEFT, INPUT);
    pinMode(IR_BACK_RIGHT, INPUT);
}


// ============================================================================
// MOTOR CONTROL
// ============================================================================

void setMotor(int motor, int speed, bool forward)
{
    speed = constrain(speed, 0, 255);

    if (motor == 0)
    {
        // Left side
        analogWrite(LEFT_ENA, speed);

        if (forward)
        {
            digitalWrite(LEFT_IN1, HIGH);
            digitalWrite(LEFT_IN2, LOW);
        }
        else
        {
            digitalWrite(LEFT_IN1, LOW);
            digitalWrite(LEFT_IN2, HIGH);
        }
    }
    else
    {
        // Right side
        analogWrite(RIGHT_ENB, speed);

        if (forward)
        {
            digitalWrite(RIGHT_IN1, HIGH);
            digitalWrite(RIGHT_IN2, LOW);
        }
        else
        {
            digitalWrite(RIGHT_IN1, LOW);
            digitalWrite(RIGHT_IN2, HIGH);
        }
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


void stopMotors()
{
    digitalWrite(LEFT_IN1, LOW);
    digitalWrite(LEFT_IN2, LOW);

    digitalWrite(RIGHT_IN1, LOW);
    digitalWrite(RIGHT_IN2, LOW);

    analogWrite(LEFT_ENA, 0);
    analogWrite(RIGHT_ENB, 0);
}


// ============================================================================
// ULTRASONIC SENSOR SYSTEM
// ============================================================================

void updateUltrasonicSensors()
{
    frontDistance = measureDistance(FRONT_TRIG, FRONT_ECHO);
    leftDistance  = measureDistance(LEFT_TRIG, LEFT_ECHO);
    rightDistance = measureDistance(RIGHT_TRIG, RIGHT_ECHO);
}


float measureDistance(int trigPin, int echoPin)
{
    digitalWrite(trigPin, LOW);
    delayMicroseconds(2);

    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin, LOW);

    unsigned long duration = pulseIn(echoPin, HIGH, 25000);

    // No valid echo received.
    if (duration == 0)
    {
        return 300.0;
    }

    float distance = (duration * 0.0343) / 2.0;

    // Reject invalid readings.
    if (distance < 2.0 || distance > 300.0)
    {
        return 300.0;
    }

    return distance;
}


// ============================================================================
// IR EDGE SENSOR SYSTEM
// ============================================================================

void updateIRSensors()
{
    frontLeftIR  = analogRead(IR_FRONT_LEFT);
    frontRightIR = analogRead(IR_FRONT_RIGHT);

    backLeftIR   = analogRead(IR_BACK_LEFT);
    backRightIR  = analogRead(IR_BACK_RIGHT);
}


bool isEdgeDetected()
{
    return
        frontLeftIR  > IR_THRESHOLD ||
        frontRightIR > IR_THRESHOLD ||
        backLeftIR   > IR_THRESHOLD ||
        backRightIR  > IR_THRESHOLD;
}


// ============================================================================
// EDGE AVOIDANCE
// ============================================================================

void handleEdgeAvoidance()
{
    currentState = EDGE_AVOIDANCE;


    // ------------------------------------------------------------------------
    // Both front sensors detect the border.
    // ------------------------------------------------------------------------

    if (frontLeftIR > IR_THRESHOLD &&
        frontRightIR > IR_THRESHOLD)
    {
        moveBackward(ESCAPE_SPEED);
        delay(300);

        spinRight(ESCAPE_SPEED);
        delay(220);

        return;
    }


    // ------------------------------------------------------------------------
    // Front-left sensor detects the border.
    // ------------------------------------------------------------------------

    if (frontLeftIR > IR_THRESHOLD)
    {
        moveBackward(ESCAPE_SPEED);
        delay(220);

        spinRight(ESCAPE_SPEED);
        delay(180);

        return;
    }


    // ------------------------------------------------------------------------
    // Front-right sensor detects the border.
    // ------------------------------------------------------------------------

    if (frontRightIR > IR_THRESHOLD)
    {
        moveBackward(ESCAPE_SPEED);
        delay(220);

        spinLeft(ESCAPE_SPEED);
        delay(180);

        return;
    }


    // ------------------------------------------------------------------------
    // Both rear sensors detect the border.
    // ------------------------------------------------------------------------

    if (backLeftIR > IR_THRESHOLD &&
        backRightIR > IR_THRESHOLD)
    {
        moveForward(NORMAL_SPEED);
        delay(250);

        return;
    }


    // ------------------------------------------------------------------------
    // Rear-left sensor detects the border.
    // ------------------------------------------------------------------------

    if (backLeftIR > IR_THRESHOLD)
    {
        moveForward(NORMAL_SPEED);
        delay(150);

        turnRight(NORMAL_SPEED);
        delay(100);

        return;
    }


    // ------------------------------------------------------------------------
    // Rear-right sensor detects the border.
    // ------------------------------------------------------------------------

    if (backRightIR > IR_THRESHOLD)
    {
        moveForward(NORMAL_SPEED);
        delay(150);

        turnLeft(NORMAL_SPEED);
        delay(100);

        return;
    }
}


// ============================================================================
// OPPONENT DETECTION
// ============================================================================

bool isOpponentDetected()
{
    return
        frontDistance <= DETECTION_RANGE ||
        leftDistance  <= DETECTION_RANGE ||
        rightDistance <= DETECTION_RANGE;
}


// ============================================================================
// ATTACK STRATEGY
// ============================================================================

void attackOpponent()
{
    // ------------------------------------------------------------------------
    // Opponent directly ahead and very close.
    // ------------------------------------------------------------------------

    if (frontDistance <= CLOSE_RANGE)
    {
        moveForward(ATTACK_SPEED);
        return;
    }


    // ------------------------------------------------------------------------
    // Opponent detected in front.
    // ------------------------------------------------------------------------

    if (frontDistance <= DETECTION_RANGE)
    {
        moveForward(NORMAL_SPEED);
        return;
    }


    // ------------------------------------------------------------------------
    // Opponent detected on the left.
    // ------------------------------------------------------------------------

    if (leftDistance <= DETECTION_RANGE &&
        leftDistance < rightDistance)
    {
        turnLeft(NORMAL_SPEED);
        return;
    }


    // ------------------------------------------------------------------------
    // Opponent detected on the right.
    // ------------------------------------------------------------------------

    if (rightDistance <= DETECTION_RANGE)
    {
        turnRight(NORMAL_SPEED);
        return;
    }
}


// ============================================================================
// SEARCH STRATEGY
// ============================================================================

void searchForOpponent()
{
    static unsigned long searchTimer = 0;
    static bool rotateRight = true;


    if (searchTimer == 0)
    {
        searchTimer = millis();
    }


    // Reverse search direction every two seconds.
    if (millis() - searchTimer >= 2000)
    {
        rotateRight = !rotateRight;
        searchTimer = millis();
    }


    if (rotateRight)
    {
        spinRight(SEARCH_SPEED);
    }
    else
    {
        spinLeft(SEARCH_SPEED);
    }


    currentState = SEARCHING;
}