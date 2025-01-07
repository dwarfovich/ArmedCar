#include <ps2.hpp>
#include <Servo.h>

constexpr uint8_t       selectPin               = 10;
constexpr uint8_t       commandPin              = 11;
constexpr uint8_t       dataPin                 = 12;
constexpr uint8_t       clockPin                = 13;
constexpr bool          pressureMode            = true;
constexpr bool          enableRumble            = true;
constexpr unsigned long baudRate                = 57600;
constexpr unsigned long serialMonitorStartDelay = 300;
constexpr unsigned long readControllerDataDelay = 50;
#define STICK_X_CENTER_VALUE 128
#define STICK_Y_CENTER_VALUE 127

constexpr int pliersPin        = 8;
constexpr int handRotationPin  = 14;
constexpr int handVerticalPin1 = 15;
constexpr int handVerticalPin2 = 9;

int LEFT_MOTOR_PWM_PIN  = 3;
int LEFT_MOTOR_IN1_PIN  = 1;
int LEFT_MOTOR_IN2_PIN  = 2;
int RIGHT_MOTOR_PWM_PIN = 5;
int RIGHT_MOTOR_IN1_PIN = 4;
int RIGHT_MOTOR_IN2_PIN = 6;

ps2::Controller     controller;
ps2::ErrorCode      configurationErrorCode;
ps2::ControllerType controllerType;

Servo verticalMotor1;
Servo verticalMotor2;
Servo rotationMotor;
Servo pliersMotor;

#define LEFT_MOTOR_FORWARD

enum class Direction : uint8_t
{
    forward,
    backward
};

void stopProgram();
void setupMotors()
{
    pinMode(LEFT_MOTOR_PWM_PIN, OUTPUT);
    pinMode(LEFT_MOTOR_IN1_PIN, OUTPUT);
    pinMode(LEFT_MOTOR_IN2_PIN, OUTPUT);

    pinMode(RIGHT_MOTOR_PWM_PIN, OUTPUT);
    pinMode(RIGHT_MOTOR_IN1_PIN, OUTPUT);
    pinMode(RIGHT_MOTOR_IN2_PIN, OUTPUT);

    rotationMotor.attach(handRotationPin);
    verticalMotor1.attach(handVerticalPin1);
    verticalMotor2.attach(handVerticalPin2);
    pliersMotor.attach(pliersPin);
}

void enableLeftMotor(int speed, Direction direction)
{
    bool inPin1 = HIGH;
    bool inPin2 = LOW;
    if (direction == Direction::backward) {
        inPin1 = LOW;
        inPin2 = HIGH;
    }
    digitalWrite(LEFT_MOTOR_IN1_PIN, inPin1);
    digitalWrite(LEFT_MOTOR_IN2_PIN, inPin2);
    analogWrite(LEFT_MOTOR_PWM_PIN, speed);
}

void enableRightMotor(int speed, Direction direction)
{
    bool inPin1 = HIGH;
    bool inPin2 = LOW;
    if (direction == Direction::backward) {
        inPin1 = LOW;
        inPin2 = HIGH;
    }
    digitalWrite(RIGHT_MOTOR_IN1_PIN, inPin1);
    digitalWrite(RIGHT_MOTOR_IN2_PIN, inPin2);
    analogWrite(RIGHT_MOTOR_PWM_PIN, speed);
}

void moveCar(uint8_t stickX, uint8_t stickY)
{
    if (stickY == STICK_Y_CENTER_VALUE) {
        if (stickX == STICK_X_CENTER_VALUE) {
            enableLeftMotor(0, Direction::forward);
            enableRightMotor(0, Direction::forward);
        } else if (stickX < STICK_X_CENTER_VALUE) {
            enableLeftMotor(200, Direction::backward);
            enableRightMotor(200, Direction::forward);
        } else {
            enableLeftMotor(200, Direction::forward);
            enableRightMotor(200, Direction::backward);
        }
    } else {
        if (stickX == STICK_X_CENTER_VALUE) {
            if (stickY > STICK_Y_CENTER_VALUE) {
                const uint8_t amplitude = map(stickY, 127, 255, 0, 255);
                Serial.print("Amp = ");
                Serial.println(amplitude);
                enableLeftMotor(amplitude, Direction::backward);
                enableRightMotor(amplitude, Direction::backward);
            } else {
                const uint8_t amplitude = map(stickY, 127, 0, 0, 255);
                Serial.print("Amp = ");
                Serial.println(amplitude);
                enableLeftMotor(amplitude, Direction::forward);
                enableRightMotor(amplitude, Direction::forward);
            }
        } else if (stickX < STICK_X_CENTER_VALUE) {
            enableLeftMotor(200, Direction::backward);
            enableRightMotor(200, Direction::forward);
        } else {
            enableLeftMotor(200, Direction::forward);
            enableRightMotor(200, Direction::backward);
        }
    }
}

void setup()
{
    Serial.begin(baudRate);
    delay(serialMonitorStartDelay);
    configurationErrorCode = controller.configure(clockPin, commandPin, selectPin, dataPin, pressureMode, enableRumble);
    if (configurationErrorCode == ps2::ErrorCode::Success) {
        controllerType = controller.type();
    }
    if (configurationErrorCode != ps2::ErrorCode::Success || controllerType != ps2::ControllerType::WirelessDualShock) {
        Serial.print("Failed to configure controller, errorCode = ");
        Serial.print(static_cast<int>(configurationErrorCode));
        Serial.print(", controllerType = ");
        Serial.print(static_cast<int>(controllerType));
        Serial.print(". Program execution stopped...");
        // stopProgram();
    } else {
        Serial.print("Configuration successful.");
    }
    setupMotors();
}

void stopProgram()
{
    while (true) {
    };
}

void loop()
{
    controller.readData();

    const auto leftStickX = controller.analogButtonState(PSS_LX);
    const auto leftStickY = controller.analogButtonState(PSS_LY);
    Serial.print(leftStickX);
    Serial.print(" ");
    Serial.println(leftStickY);
    moveCar(leftStickX, leftStickY);

    auto rightStick = controller.analogButtonState(PSS_RX);
    if (rightStick < 92 || rightStick > 96) {
        auto rotation = map(rightStick, 0, 256, 5, -5);
        rotationMotor.write(rotationMotor.read() + rotation);
    }

    if (controller.buttonPressed(PSB_TRIANGLE)) {
        verticalMotor1.write(verticalMotor1.read() + 3);
        verticalMotor2.write(verticalMotor2.read() + 3);
        Serial.println("TRIANGLE pressed");
        // pliersMotor.write(pliersMotor.read() + 2);
    } else if (controller.buttonPressed(PSB_CROSS)) {
        verticalMotor1.write(verticalMotor1.read() - 3);
        // pliersMotor.write(pliersMotor.read() - 2);
        verticalMotor2.write(verticalMotor2.read() - 3);
        Serial.println("CROSS pressed");
    }

    if (controller.buttonPressed(PSB_CIRCLE)) {
        pliersMotor.write(pliersMotor.read() + 2);
        Serial.println("PSB_CIRCLE pressed");
    } else if (controller.buttonPressed(PSB_SQUARE)) {
        pliersMotor.write(pliersMotor.read() - 2);
        Serial.println("PSB_SQUARE pressed");
    }

    // if (leftStick < 92 || leftStick > 96) {
    //     analogWrite(LEFT_MOTOR_PIN_1, leftStick);
    // }

    delay(readControllerDataDelay);
}
