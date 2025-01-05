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

constexpr int pliersPin          = 9;
constexpr int handRotationPin    = 14;
constexpr int handVerticalPin1   = 15;
constexpr int handVerticalPin2    = 8;
constexpr int leftWheelMotorPin  = 3;
constexpr int rightWheelMotorPin = 5;

ps2::Controller     controller;
ps2::ErrorCode      configurationErrorCode;
ps2::ControllerType controllerType;

Servo leftMotor;
Servo rightMotor;
Servo verticalMotor1;
Servo verticalMotor2;
Servo rotationMotor;
Servo pliersMotor;

void stopProgram();
void setupMotors();

void setup()
{
    Serial.begin(baudRate);
    delay(serialMonitorStartDelay);
    configurationErrorCode = controller.configure(clockPin, commandPin, selectPin, dataPin, pressureMode, enableRumble);
    if (configurationErrorCode == ps2::ErrorCode::Success) {
        controllerType = controller.type();
    }
    if (configurationErrorCode != ps2::ErrorCode::Success || controllerType != ps2::ControllerType::WirelessDualShock){
        Serial.print("Failed to configure controller, errorCode = ");
        Serial.print(static_cast<int>(configurationErrorCode));
        Serial.print(", controllerType = " );
        Serial.print(static_cast<int>(controllerType));
        Serial.print(". Program execution stopped...");
        stopProgram();
    } else{
        Serial.print("Configuration successful.");
    }
    setupMotors();
}

void stopProgram(){
    while(true){};
}

void setupMotors()
{
    rotationMotor.attach(handRotationPin);
    verticalMotor1.attach(handVerticalPin1);
}

void loop()
{
        controller.readData();

        const auto rightStickX = controller.analogButtonState(PSS_RX);
        const auto rotationX = map(rightStickX, 0, 256, 3, -3);
        Serial.print(rightStickX);
        Serial.print(" ");
        Serial.print(rotationX);
        Serial.println(";");
        rotationMotor.write(rotationMotor.read() + rotationX);

        const auto rightStickY = controller.analogButtonState(PSS_RY);
        const auto rotationY = map(rightStickY, 0, 256, -3, 3);
        Serial.print(rightStickY);
        Serial.print(" - ");
        Serial.print(rotationY);
        Serial.println(";");
        verticalMotor1.write(verticalMotor1.read() + rotationY);

        delay(readControllerDataDelay);
}
