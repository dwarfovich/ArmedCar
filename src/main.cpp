#include <ps2.hpp>
#include <Servo.h>

#define BAUD_RATE 57600ul
#define SERIAL_MONITOR_START_DELAY 300ul
#define READ_CONTROLLER_DATA_DELAY 50ul

#define SELECT_PIN 10
#define COMMAND_PIN 11
#define DATA_PIN 12
#define CLOCK_PIN 13
#define PRESSURE_MODE true
#define ENABLE_RUMBLE true
#define STICK_X_CENTER_VALUE 128
#define STICK_Y_CENTER_VALUE 127

#define PLIERS_PIN 8
#define HAND_ROTATION_PIN 14
#define HAND_VERTICAL_PIN_1 15
#define HAND_VERTICAL_PIN_2 9

#define LEFT_MOTOR_PWM_PIN 3
#define LEFT_MOTOR_IN1_PIN 1
#define LEFT_MOTOR_IN2_PIN 2
#define RIGHT_MOTOR_PWM_PIN 5
#define RIGHT_MOTOR_IN1_PIN 4
#define RIGHT_MOTOR_IN2_PIN 6
#define DEFAULT_ROTATION_AMPLITUDE 255

enum class Direction : uint8_t
{
    forward,
    backward
};

ps2::Controller     controller;
ps2::ErrorCode      configurationErrorCode;
ps2::ControllerType controllerType;

Servo pliersMotor;
Servo rotationMotor;
Servo verticalMotor1;
Servo verticalMotor2;

void stopProgram();
void setupMotors()
{
    pinMode(LEFT_MOTOR_PWM_PIN, OUTPUT);
    pinMode(LEFT_MOTOR_IN1_PIN, OUTPUT);
    pinMode(LEFT_MOTOR_IN2_PIN, OUTPUT);

    pinMode(RIGHT_MOTOR_PWM_PIN, OUTPUT);
    pinMode(RIGHT_MOTOR_IN1_PIN, OUTPUT);
    pinMode(RIGHT_MOTOR_IN2_PIN, OUTPUT);

    rotationMotor.attach(HAND_ROTATION_PIN);
    verticalMotor1.attach(HAND_VERTICAL_PIN_1);
    verticalMotor2.attach(HAND_VERTICAL_PIN_2);
    pliersMotor.attach(PLIERS_PIN);
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
            enableLeftMotor(DEFAULT_ROTATION_AMPLITUDE, Direction::backward);
            enableRightMotor(DEFAULT_ROTATION_AMPLITUDE, Direction::forward);
        } else {
            enableLeftMotor(DEFAULT_ROTATION_AMPLITUDE, Direction::forward);
            enableRightMotor(DEFAULT_ROTATION_AMPLITUDE, Direction::backward);
        }
    } else {
        if (stickX == STICK_X_CENTER_VALUE) {
            if (stickY > STICK_Y_CENTER_VALUE) {
                const uint8_t amplitude = map(stickY, STICK_Y_CENTER_VALUE, 255, 0, 255);
                enableLeftMotor(amplitude, Direction::backward);
                enableRightMotor(amplitude, Direction::backward);
            } else {
                const uint8_t amplitude = map(stickY, STICK_Y_CENTER_VALUE, 0, 0, 255);
                enableLeftMotor(amplitude, Direction::forward);
                enableRightMotor(amplitude, Direction::forward);
            }
        } else {
            if (stickY < STICK_Y_CENTER_VALUE) {
                const uint8_t amplitudeY = map(stickY, STICK_Y_CENTER_VALUE, 0, 0, 255);
                const uint8_t amplitudeX = map(stickX, 0, STICK_X_CENTER_VALUE, 0, amplitudeY);
                enableLeftMotor(amplitudeX, Direction::backward);
                enableRightMotor(amplitudeY, Direction::forward);
            } else {
                const uint8_t amplitudeY = map(stickY, STICK_Y_CENTER_VALUE, 255, 0, 255);
                const uint8_t amplitudeX = map(stickX, 0, STICK_X_CENTER_VALUE, 0, amplitudeY);
                enableLeftMotor(amplitudeX, Direction::forward);
                enableRightMotor(amplitudeY, Direction::backward);
            }
        }
    }
}

void setup()
{
    Serial.begin(BAUD_RATE);
    delay(SERIAL_MONITOR_START_DELAY);
    
    configurationErrorCode =
        controller.configure(CLOCK_PIN, COMMAND_PIN, SELECT_PIN, DATA_PIN, PRESSURE_MODE, ENABLE_RUMBLE);
    if (configurationErrorCode == ps2::ErrorCode::Success) {
        controllerType = controller.type();
    }
    if (configurationErrorCode != ps2::ErrorCode::Success || controllerType != ps2::ControllerType::WirelessDualShock) {
        Serial.print("Failed to configure controller, errorCode = ");
        Serial.print(static_cast<int>(configurationErrorCode));
        Serial.print(", controllerType = ");
        Serial.print(static_cast<int>(controllerType));
        Serial.print(". Program execution stopped...");
        stopProgram();
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
    moveCar(leftStickX, leftStickY);

    const auto rightStick = controller.analogButtonState(PSS_RX);
    if (rightStick < 92 || rightStick > 96) {
        const auto rotation = map(rightStick, 0, 255, 5, -5);
        rotationMotor.write(rotationMotor.read() + rotation);
    }

    if (controller.buttonPressed(PSB_TRIANGLE)) {
        verticalMotor1.write(verticalMotor1.read() + 3);
        verticalMotor2.write(verticalMotor2.read() + 3);
    } else if (controller.buttonPressed(PSB_CROSS)) {
        verticalMotor1.write(verticalMotor1.read() - 3);
        verticalMotor2.write(verticalMotor2.read() - 3);
    }

    if (controller.buttonPressed(PSB_CIRCLE)) {
        pliersMotor.write(pliersMotor.read() + 2);
    } else if (controller.buttonPressed(PSB_SQUARE)) {
        pliersMotor.write(pliersMotor.read() - 2);
    }

    delay(READ_CONTROLLER_DATA_DELAY);
}
