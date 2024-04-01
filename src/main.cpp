#include <Arduino_FreeRTOS.h>
#include <LiquidCrystal.h>
#include "semphr.h"
#define SIGNALRED 9
#define GREEN 11
#define YELLOW 12
#define RED 13

const int rs = 7, en = 6, d4 = 5, d5 = 4, d6 = 3, d7 = 8;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);
TaskHandle_t greenfixedblink_Handle, greenlit_Handle, yellow_Handle, red_Handle,
    pedastriancrossing_Handle;
int i;                       // i is the counter variable
int button = 9, buzzer = 10; // define button and buzzer digitalPin on Arduino UNO
const byte interruptPin = 2;
volatile bool redLightCond = false;        // default red light condition
volatile bool pedastrianLightCond = false; // default pedestrian light condition
volatile int z;                            // lcd display number which can vary according to each LED task
SemaphoreHandle_t xMutex;

void setup()
{
    lcd.begin(16, 2);                    // set up the LCD's number of columns and rows
    pinMode(buzzer, OUTPUT);             // buzzer usage
    pinMode(interruptPin, INPUT_PULLUP); // interrupt set up
    attachInterrupt(digitalPinToInterrupt(2), changePedastrianTime, LOW);
    // interrupt set up

    xMutex = xSemaphoreCreateMutex();

 xTaskCreate(greenLedControllerTask,"FIXEDGREEN LIGHT 
Task",128,NULL,2,&greenlit_Handle);
 xTaskCreate(greenLedFixedBlinkingTask,"FIXED GREEN LIGHT 
Task",128,NULL,2,&greenfixedblink_Handle);
 xTaskCreate(yellowLedControllerTask,"YELLOW LIGHT 
Task",128,NULL,2,&yellow_Handle);
 xTaskCreate(redLedControllerTask,"RED LIGHT Task",128,NULL,2,&red_Handle);
 xTaskCreate(pedastrainCrossingTask,"PEDASTRIAN CROSSING 
Task",128,NULL,2,&pedastriancrossing_Handle);
}

void changePedastrianTime()
{
    pinMode(SIGNALRED, OUTPUT);
    redLightCond = true;

    digitalWrite(SIGNALRED, HIGH);
    digitalWrite(SIGNALRED, LOW);
}

void greenLedControllerTask(void *pvParameters)
{
    pinMode(GREEN, OUTPUT);
    z = 4;
    vTaskPrioritySet(greenlit_Handle, 3);
    lcd.setCursor(0, 0);
    lcd.write("Time Left G:");
    lcd.setCursor(0, 1);
    lcd.print(z);

    for (i = 0; i < 2; i++)
    {
        digitalWrite(GREEN, HIGH);
        delay(1000);
        lcd.setCursor(0, 1);
        z = z - 1;
        lcd.print(z);
    }

    vTaskDelay(1);
    vTaskPrioritySet(greenlit_Handle, 2);
    vTaskPrioritySet(greenfixedblink_Handle, 3);
}

void greenLedFixedBlinkingTask(void *pvParameters)
{
    pinMode(GREEN, OUTPUT);
    vTaskPrioritySet(greenfixedblink_Handle, 3);

    lcd.setCursor(0, 0);
    lcd.print("Time Left G:");

    for (i = 0; i < 2; i++)
    {
        digitalWrite(GREEN, HIGH);
        delay(500);
        digitalWrite(GREEN, LOW);
        delay(500);
        lcd.setCursor(0, 1);
        z = z - 1;
        lcd.print(z);
    }

    vTaskDelay(1);
    vTaskPrioritySet(greenfixedblink_Handle, 2);
    vTaskPrioritySet(yellow_Handle, 3);
}

void yellowLedControllerTask(void *pvParameters)
{
    pinMode(YELLOW, OUTPUT);
    z = 2;
    vTaskPrioritySet(yellow_Handle, 3);
    lcd.setCursor(0, 0);
    lcd.print("Time Left Y:");
    lcd.setCursor(0, 1);
    lcd.print(z);

    for (i = 0; i < 2; i++)
    {
        digitalWrite(YELLOW, HIGH);
        delay(1000);
        lcd.setCursor(0, 1);
        z = z - 1;
        lcd.print(z);
    }

    vTaskDelay(1);
    digitalWrite(YELLOW, LOW);
    vTaskPrioritySet(yellow_Handle, 2);
    vTaskPrioritySet(red_Handle, 3);
}

void redLedControllerTask(void *pvParameters)
{
    pinMode(RED, OUTPUT);
    vTaskPrioritySet(red_Handle, 3);
    xSemaphoreTake(xMutex, portMAX_DELAY);

    if (redLightCond == false)
    {
        z = 6;
        lcd.setCursor(0, 0);
        lcd.print("Time Left R:");
        lcd.setCursor(0, 1);
        lcd.print(z);

        for (i = 0; i < 6; i++)
        {
            digitalWrite(RED, HIGH);
            delay(1000);
            lcd.setCursor(0, 1);
            z = z - 1;
            lcd.print(z);
        }

        digitalWrite(RED, LOW);
        xSemaphoreGive(xMutex);
        vTaskDelay(1);
    }

    else if (redLightCond == true)
    {
        z = 6;
        lcd.setCursor(0, 0);
        lcd.print("Time Left R:");
        lcd.setCursor(0, 1);
        lcd.print(z);

        for (i = 0; i < 3; i++)
        {
            digitalWrite(RED, HIGH);
            delay(1000);
            digitalWrite(RED, LOW);
            lcd.setCursor(0, 1);
            z = z - 1;
            lcd.print(z);
        }

        for (i = 0; i < 3; i++)
        {
            digitalWrite(RED, HIGH);
            delay(500);
            digitalWrite(RED, LOW);
            delay(500);
            lcd.setCursor(0, 1);
            z = z - 1;
            lcd.print(z);
        }
        pedastrianLightCond = true;
        xSemaphoreGive(xMutex);
    }

    vTaskDelay(1);
    vTaskPrioritySet(red_Handle, 2);
    vTaskPrioritySet(pedastriancrossing_Handle, 3);
}

void pedastrainCrossingTask(void *pvParameters)
{
    pinMode(GREEN, OUTPUT);
    vTaskPrioritySet(pedastriancrossing_Handle, 3);
    xSemaphoreTake(xMutex, portMAX_DELAY);

    if (pedastrianLightCond == true)
    {
        z = 8;
        lcd.setCursor(0, 0);
        lcd.write("PedastrianC:");
        lcd.setCursor(0, 1);
        lcd.print(z);

        for (i = 0; i < 8; i++)
        {
            digitalWrite(GREEN, HIGH);
            tone(buzzer, 800);
            delay(500);
            noTone(buzzer);
            delay(500);
            lcd.setCursor(0, 1);
            z = z - 1;
            lcd.print(z);
        }
        xSemaphoreGive(xMutex);
        vTaskDelay(1);
    }

    else if (pedastrianLightCond == false)
    {
        z = 3;
        lcd.setCursor(0, 0);
        lcd.write("PedastrianC:");
        lcd.setCursor(0, 1);
        lcd.print(z);

        for (i = 0; i < 3; i++)
        {
            digitalWrite(GREEN, HIGH);
            tone(buzzer, 800);
            delay(500);
            noTone(buzzer);
            delay(500);
            lcd.setCursor(0, 1);
            z = z - 1;
            lcd.print(z);
        }
        xSemaphoreGive(xMutex);
    }

    vTaskDelay(1);
    vTaskPrioritySet(pedastriancrossing_Handle, 2);
    vTaskPrioritySet(greenlit_Handle, 3);
}

void loop()
{
}
