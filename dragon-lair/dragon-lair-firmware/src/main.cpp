#define BUZZER_PIN 2
#define FOREWARD_MOTOR_PIN 4
#define BACKWARD_MOTOR_PIN 5
#define ON_PIN 6
#define OPEN_PIN 7
#define TOGGLE_CLOSE_PIN 8
#define ABORT_PIN 9
#define TOGGLE_OPEN_PIN 10
#define CLOSE_PIN 11
#define CLOSE_LIMIT_PIN 12
#define OPEN_LIMIT_PIN 13
#define SAFE_TO_MOVE_1_PIN 14
#define SAFE_TO_MOVE_2_PIN 15
#define AD_IO_PIN 26
#define MAX_SPEED_PIN 28

#include <Arduino.h>



void blinkErrorCode(int blinks)
{
    while (true)
    {
        for (int i = 0; i < blinks; i++)
        {
            digitalWrite(LED_BUILTIN, HIGH);
            sleep_ms(200);
            digitalWrite(LED_BUILTIN, LOW);
            sleep_ms(200);
        }

        sleep_ms(5000);
    }
}

void setup()
{
    sleep_ms(5000);

    analogReadResolution(8);

    analogWriteResolution(8);
    analogWriteFreq(5000);

    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, LOW);

    pinMode(BUZZER_PIN, OUTPUT);
    digitalWrite(BUZZER_PIN, LOW);

    pinMode(FOREWARD_MOTOR_PIN, OUTPUT);
    analogWrite(FOREWARD_MOTOR_PIN, 0);

    pinMode(BACKWARD_MOTOR_PIN, OUTPUT);
    digitalWrite(BACKWARD_MOTOR_PIN, LOW);

    pinMode(ON_PIN, OUTPUT);
    digitalWrite(ON_PIN, LOW);

    pinMode(MAX_SPEED_PIN, INPUT);

    Serial.begin(9600);
}

bool forward = true;

void loop()
{
    int pin = forward ? FOREWARD_MOTOR_PIN : BACKWARD_MOTOR_PIN;

    digitalWrite(BACKWARD_MOTOR_PIN, LOW);
    digitalWrite(FOREWARD_MOTOR_PIN, LOW);

    Serial.println("Forward: " + String(forward));

    forward = !forward;

    digitalWrite(ON_PIN, HIGH);


    unsigned long start = millis();

    while (millis() - start < 5000)
    {
        analogWrite(LED_BUILTIN, 255);
        analogWrite(pin, 255);
        Serial.println(255);

        sleep_ms(100);
    }

    digitalWrite(ON_PIN, LOW);
    sleep_ms(2000);
}