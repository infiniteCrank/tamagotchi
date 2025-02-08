#include <Adafruit_GFX.h>
#include <MCUFRIEND_kbv.h>
#include "TouchScreen.h"

// Pin Definitions for the TouchScreen
#define YP A2  // Touchscreen Y+
#define XM A3  // Touchscreen X-
#define YM 8   // Touchscreen Y-
#define XP 9   // Touchscreen X+

// Color Definitions
#define BLACK   0x0000
#define GREEN   0x07E0
#define WHITE   0xFFFF

// Touchscreen initialization with screen resistance (300 ohms in this case)
TouchScreen ts = TouchScreen(XP, YP, XM, YM, 300);
MCUFRIEND_kbv tft;

// Pet Status Variables
int hunger = 10;
int happiness = 0;
int health = 10; // New health variable
int level = 1;   // New level variable
const int maxHunger = 10;       // Set max hunger level
const int maxHappiness = 10;    // Set max happiness level
const int maxHealth = 10;        // Set max health level
unsigned long previousMillis = 0; // Save the last time happiness was decreased
const long interval = 60000;     // Interval of 1 minute in milliseconds
unsigned long levelMillis = 0;   // Timer for level up
const long levelInterval = 3600000; // Interval of 1 hour in milliseconds (1 hour)

// Timer for egg jiggle
unsigned long jiggleMillis = 0; // Track the last jiggle time
const long jiggleInterval = 1000; // Interval for jiggle effect (1 second)


// Button area settings for feeding the pet
#define FEED_X1 50
#define FEED_Y1 160
#define FEED_X2 250
#define FEED_Y2 210

void setup() {
    Serial.begin(9600);
    tft.reset();
    uint16_t ID = tft.readID();
    Serial.print("Display ID: ");
    Serial.println(ID);
    
    tft.begin(ID);
    drawPetStatus(); // Draw the initial UI
}

void loop() {
    // Get touch input from the touchscreen
    TSPoint p = ts.getPoint();
    
    // Set pin modes to avoid interference after reading the touch
    pinMode(XM, OUTPUT); 
    pinMode(YP, OUTPUT);
    // Serial.print("X = "); Serial.print(p.x);
    // Serial.print("\tY = "); Serial.print(p.y);
    // Serial.print("\tPressure = "); Serial.println(p.z);
    // Only process if the touch pressure is above the threshold
    if (p.z > 150) {
        //x = 400 to 420  y = 380 - 410 pressure > 150
        // Check if the touch coordinates fall within the button area
        if (p.x >= 350 && p.x <= 420 && p.y >= 250 && p.y <= 780) {
            feedPet(); // Feed the pet
            drawPetStatus(); // Refresh UI to show updated values
            delay(300); // Debounce delay
        }
    }

    // Timer for decreasing happiness
    unsigned long currentMillis = millis(); // Get current time
    if (currentMillis - previousMillis >= interval) {
        previousMillis = currentMillis; // Save the last time happiness was decreased
        if (happiness > 0) {
            happiness--; // Decrease happiness if it's above 0
        }
        if (hunger < maxHunger) {
          hunger ++;
        }
        if (hunger == maxHunger && happiness == 0 && health > 0){
          health --;
        }
        if (health < maxHealth && hunger < maxHunger && happiness > 0 ){
          health ++;
        }
        drawPetStatus(); // Refresh UI to show updated happiness
    }

    // Timer for level up
    if (currentMillis - levelMillis >= levelInterval) {
        levelMillis = currentMillis; // Save the last time level was increased
        if (health > 0) {
            level++; // Increase level if health is above 0
        }
        drawPetStatus(); // Refresh UI to show updated level
    }

    delay(50); // Short delay to reduce CPU load
}

// Function to display the pet's status
void drawPetStatus() {
    tft.fillScreen(BLACK); // Clear the screen to black

    // Print Hunger and Happiness
    tft.setTextColor(WHITE);
    tft.setTextSize(2);
    tft.setCursor(10, 10);
    tft.print("Hunger: ");
    tft.println(hunger);
    tft.setCursor(10, 50);
    tft.print("Happiness: ");
    tft.println(happiness);
    tft.setCursor(10, 90); // New line for health
    tft.print("Health: ");
    tft.println(health);
    tft.setCursor(180, 10); // Position for level
    tft.print("Level: ");
    tft.println(level); // Display the level of the pet
  
    // Draw the Feed Button
    tft.fillRect(FEED_X1, FEED_Y1, (FEED_X2 - FEED_X1), (FEED_Y2 - FEED_Y1), GREEN);
    tft.setCursor(FEED_X1 + 10, FEED_Y1 + 10);
    tft.setTextColor(BLACK);
    tft.setTextSize(2);
    tft.println("Feed Pet");

    // Draw the Egg Below
    drawEgg(FEED_X1 + 100, FEED_Y2 + 100); // Position egg right below the feed button
}

// Function to draw a filled egg with spots using drawPixel
void drawEgg(int x, int y) {
    int eggWidth = 30;  // Width of the egg
    int eggHeight = 40; // Height of the egg

    // Draw the filled egg shape
    for (int i = -eggWidth; i <= eggWidth; i++) {
        // Calculate the height based on the ellipse formula
        int h = (int)(eggHeight * sqrt(1 - (double)(i * i) / (eggWidth * eggWidth))); 

        // Draw the upper part of the egg (negative y direction)
        for (int j = 0; j <= h; j++) {
            tft.drawPixel(x + i, y - j, WHITE); // Draw the pixels for the upper part
        }

        // Draw the lower part of the egg (positive y direction)
        for (int j = 0; j <= h; j++) {
            tft.drawPixel(x + i, y + j, WHITE); // Draw the pixels for the lower part
        }
    }

    // Draw spots on the egg using small circles
    drawSpot(x - 10, y, GREEN);      // Spot 1
    drawSpot(x + 10, y - 5, GREEN);  // Spot 2
    drawSpot(x - 5, y + 5, GREEN);   // Spot 3
    drawSpot(x + 5, y + 10, GREEN);  // Spot 4
}

// Function to draw a green circle (small spot)
void drawSpot(int x, int y, uint16_t color) {
    int radius = 5; // Radius of the spot
    for (int i = -radius; i <= radius; i++) {
        for (int j = -radius; j <= radius; j++) {
            if (i * i + j * j <= radius * radius) { // Check if within the circle
                tft.drawPixel(x + i, y + j, color); // Draw the pixel
            }
        }
    }
}

// Function to feed the pet
void feedPet() {
    if (hunger > 0) {
        hunger--; // Increase hunger level
    }
    if (happiness < maxHappiness) {
        happiness++; // Increase happiness level
    }
}