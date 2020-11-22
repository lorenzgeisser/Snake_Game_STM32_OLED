#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define INITIAL_X 0
#define INITIAL_Y 0

#define SPEED 100

#define BUTTON_UP PA4
#define BUTTON_DOWN PA6
#define BUTTON_LEFT PA3
#define BUTTON_RIGHT PA7
#define BUTTON_CENTER PA5

#define DIRECTION_UP 0
#define DIRECTION_DOWN 1
#define DIRECTION_LEFT 2
#define DIRECTION_RIGHT 3

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

#define FIELD_HIGHT 12
#define FIELD_LENGTH 25

#define STATE_INTRO 0
#define STATE_GAME 1
#define STATE_GAME_OVER 2

#define BORDER 4

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

uint8_t direction;
uint16_t speed;
uint16_t length;

uint8_t headPositionX;
uint8_t headPositionY;

uint8_t foodPositionX;
uint8_t foodPositionY;

uint8_t state = STATE_INTRO;

uint16_t field[FIELD_HIGHT][FIELD_LENGTH];

void ReplaceFood()
{
    do
    {
        foodPositionY = random(0, FIELD_HIGHT);
        foodPositionX = random(0, FIELD_LENGTH);
    } while (field[foodPositionY][foodPositionX] != 0);
}
void EmptyField()
{
    for (int y = 0; y < FIELD_HIGHT; y++)
    {
        for (int x = 0; x < FIELD_LENGTH; x++)
        {
            field[y][x] = 0;
        }
    }
}

void ReadButtons()
{
    if (!digitalRead(BUTTON_UP))
    {
        direction = DIRECTION_UP;
    }
    else if (!digitalRead(BUTTON_DOWN))
    {
        direction = DIRECTION_DOWN;
    }
    else if (!digitalRead(BUTTON_LEFT))
    {
        direction = DIRECTION_LEFT;
    }
    else if (!digitalRead(BUTTON_RIGHT))
    {
        direction = DIRECTION_RIGHT;
    }
}

void displayField()
{
    display.clearDisplay();
    display.drawRect(0, 0, display.width(), display.height() - 1, WHITE);
    for (int y = 0; y < FIELD_HIGHT; y++)
    {
        for (int x = 0; x < FIELD_LENGTH; x++)
        {
            if (field[y][x])
            {
                display.fillRect(2 + (x * 5), 2 + (y * 5), 4, 4, WHITE);
            }
        }
    }

    display.fillRoundRect(2 + (foodPositionX * 5), 2 + (foodPositionY * 5), 4, 4, 2, WHITE);

    display.display();
}

void Intro()
{
    display.clearDisplay();

    display.setTextSize(2);      // Normal 1:1 pixel scale
    display.setTextColor(WHITE); // Draw white text
    display.setCursor(0, 0);     // Start at top-left corner
    display.print("Ready");

    display.display();

    if (!digitalRead(BUTTON_CENTER))
    {
        state = STATE_GAME;
    }
}

uint8_t eat()
{
    uint8_t returnValue = false;

    if ((headPositionY == foodPositionY) && (headPositionX == foodPositionX))
    {
        length++;
        ReplaceFood();
        speed--;
        returnValue = true;
    }

    return returnValue;
}

void removeLastBlock()
{
    for (int y = 0; y < FIELD_HIGHT; y++)
    {
        for (int x = 0; x < FIELD_LENGTH; x++)
        {
            if (field[y][x] == (length + 1))
            {
                field[y][x] = 0;
            }
        }
    }
}
uint8_t Move()
{
    uint8_t eatReturn = false;

    for (int y = 0; y < FIELD_HIGHT; y++)
    {
        for (int x = 0; x < FIELD_LENGTH; x++)
        {
            if (field[y][x])
            {
                field[y][x]++;
                eatReturn = eat();
            }
        }
    }

    switch (direction)
    {
    case DIRECTION_DOWN:
        if (headPositionY == (FIELD_HIGHT - 1))
        {
            return 0;
        }
        headPositionY++;
        break;

    case DIRECTION_UP:
        if (headPositionY == 0)
        {
            return 0;
        }

        headPositionY--;
        break;

    case DIRECTION_LEFT:
        if (headPositionX == 0)
        {
            return 0;
        }

        headPositionX--;
        break;

    case DIRECTION_RIGHT:
        if (headPositionX == (FIELD_LENGTH - 1))
        {
            return 0;
        }
        headPositionX++;

        break;

    default:
        break;
    }

    if (field[headPositionY][headPositionX])
    {
        return 0;
    }

    if (!eatReturn)
    {
        removeLastBlock();
    }

    field[headPositionY][headPositionX] = 1;

    return true;
}

void PlaceHead()
{
    headPositionX = random(BORDER, FIELD_LENGTH - BORDER);
    headPositionY = random(BORDER, FIELD_HIGHT - BORDER);

    field[headPositionY][headPositionX] = 1;
}

void ResetGame()
{
    EmptyField();
    PlaceHead();
    ReplaceFood();

    direction = random(0, 4);
    length = 1;
    speed = 200;
}

void Game()
{
    static uint32_t oldMillis = millis();
    uint32_t currentMillis = millis();

    ReadButtons();

    if ((currentMillis - oldMillis) >= speed)
    {
        if (Move() == false)
        {
            state = STATE_GAME_OVER;
        }
        else
        {
            displayField();
        }

        oldMillis = currentMillis;
    }
}

void GameOver()
{
    display.clearDisplay();

    display.setTextSize(2);      // Normal 1:1 pixel scale
    display.setTextColor(WHITE); // Draw white text
    display.setCursor(0, 0);     // Start at top-left corner
    display.println("Game Over!");
    display.println();
    display.print("Score: ");
    display.println(length);

    display.display();

    if (!digitalRead(BUTTON_CENTER))
    {
        state = STATE_GAME;
        ResetGame();
    }
}

void setup()
{
    Serial.begin(115200);

    pinMode(BUTTON_UP, INPUT_PULLUP);
    pinMode(BUTTON_DOWN, INPUT_PULLUP);
    pinMode(BUTTON_LEFT, INPUT_PULLUP);
    pinMode(BUTTON_RIGHT, INPUT_PULLUP);
    pinMode(BUTTON_CENTER, INPUT_PULLUP);

    display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
    display.clearDisplay();
    display.display();

    ResetGame();
}

void loop()
{
    switch (state)
    {
    case STATE_INTRO:
        Intro();
        break;

    case STATE_GAME:
        Game();
        break;

    case STATE_GAME_OVER:
        GameOver();
        break;
    }
}
