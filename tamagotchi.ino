#include <Adafruit_GFX.h>
#include <MCUFRIEND_kbv.h>
#include "TouchScreen.h"
#include <Math.h>  // Include for using trigonometric functions
#include <limits.h>  // For INT_MAX

// Pin Definitions for the TouchScreen
#define YP A2  // Touchscreen Y+
#define XM A3  // Touchscreen X-
#define YM 8   // Touchscreen Y-
#define XP 9   // Touchscreen X+

// Color Definitions
#define BLACK 0x0000
#define NAVY 0x000F
#define DARKGREEN 0x03E0
#define DARKCYAN 0x03EF
#define MAROON 0x7800
#define PURPLE 0x780F
#define OLIVE 0x7BE0
#define LIGHTGREY 0xC618
#define DARKGREY 0x7BEF
#define BLUE 0x001F
#define GREEN 0x07E0
#define CYAN 0x07FF
#define RED 0xF800
#define MAGENTA 0xF81F
#define YELLOW 0xFFE0
#define WHITE 0xFFFF
#define ORANGE 0xFD20
#define GREENYELLOW 0xAFE5
#define PINK 0xF81F
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
const long levelInterval = 1800000; // Interval of 30 min in milliseconds (1 hour)
float characterAngle = 0;

// Button area settings for feeding the pet
#define FEED_X1 50
#define FEED_Y1 160
#define FEED_X2 250
#define FEED_Y2 210

// Variables for masking
int lastEggX = FEED_X1 + 100; // Initialize with the first egg position
int lastEggY = FEED_Y2 + 100; // Initialize with the first egg position

void setup() {
    Serial.begin(9600);
    tft.reset();
    uint16_t ID = tft.readID();
    Serial.print("Display ID: ");
    Serial.println(ID);
    
    tft.begin(ID);
    drawPetStatus(); // Draw the initial UI
    drawEgg(FEED_X1 + 100, FEED_Y2 + 100, characterAngle); // Position egg right below the feed button
    
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
            if ( level == 1 ) {
              characterAngle += 90;
              if ( characterAngle >= 360 ) {
                characterAngle = 0;
              }
            }else {
              characterAngle = 0;
            }

            drawEgg(FEED_X1 + 100, FEED_Y2 + 100, characterAngle); // Position egg right below the feed button
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
            // Draw the Egg Below
        drawEgg(FEED_X1 + 100, FEED_Y2 + 100, characterAngle); // Position egg right below the feed button
    }


    // Timer for level up
    if (currentMillis - levelMillis >= levelInterval) {
        levelMillis = currentMillis; // Save the last time level was increased
        if (health > 0) {
            level++; // Increase level if health is above 0
        }
        drawPetStatus(); // Refresh UI to show updated level
        drawEgg(FEED_X1 + 100, FEED_Y2 + 100, characterAngle); // Position egg right below the feed button
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
    if ( level == 1 ) {
      tft.println("rotate egg");
    }else{
      tft.println("Feed Pet");
    }
    
}

// Function to draw a filled egg (ellipse)
void drawEgg(int x, int y, float angle) {
    // Clear the last egg drawn
    tft.fillRect(lastEggX - 60, lastEggY - 60, 120, 120, BLACK); // Clear area where the last egg was
    
    int eggWidth = 30;  // Adjusted Width of the egg
    int eggHeight = 40; // Adjusted Height of the egg

    // Convert angle to radians for trigonometric functions
    float radians = angle * (PI / 180.0);

    // Draw level 2 sprite 
    if (level >= 2) {
        //draw cat head 
        tft.fillCircle(x, y - 10, 25, LIGHTGREY);
        // Draw cat eyes 
        drawSpot(x - 10, y - 10, 5, GREENYELLOW); 
        drawSpot(x + 10, y - 10, 5, GREENYELLOW); 
        // Draw cat ears as triangles
        drawTriangle(x - 20, y - 25, x - 10, y - 45, x, y - 25, LIGHTGREY);  // Left ear
        drawTriangle(x , y - 25, x + 10, y - 45, x + 20, y - 25, LIGHTGREY);  // Left ear
    }
    if (level >= 3) {
      //draw arms 
      drawRotatedRectangle(x - 30, y , 10, 40, 45.0, LIGHTGREY);
      drawRotatedRectangle(x + 30, y , 10, 40, -45.0, LIGHTGREY);
    }
    if (level >= 4) {
      //draw legs 
      drawRotatedRectangle(x - 20, y + 30 , 10, 40, 20.0, LIGHTGREY);
      drawRotatedRectangle(x + 20, y + 30 , 10, 40, -20.0, LIGHTGREY);
    }

    // Draw the filled egg shape
    for (int i = -eggWidth; i <= eggWidth; i++) {
        // Calculate the height based on the elliptical shape
        int h = (int)(eggHeight * sqrt(1 - (double)(i * i) / (eggWidth * eggWidth))); 

        // Calculate the rotated positions
        int rotatedX = x + i * cos(radians) - h * sin(radians);
        int rotatedY = y + i * sin(radians) + h * cos(radians);

        // Draw bottom of egg
        if ( level <= 3 ) {
          for (int j = 0; j <= h; j++) {
              int upperRotatedX = x + i * cos(radians) - j * sin(radians);
              int upperRotatedY = y + i * sin(radians) + j * cos(radians);
              tft.drawPixel(upperRotatedX, upperRotatedY, WHITE); // Upper part of the egg
          }
        }
        
        // Draw top of egg
        if ( level == 1 ) {
          for (int j = 0; j <= h; j++) {
              int lowerRotatedX = x + i * cos(radians) + j * sin(radians);
              int lowerRotatedY = y + i * sin(radians) - j * cos(radians);
              tft.drawPixel(lowerRotatedX, lowerRotatedY, WHITE); // Lower part of the egg
          }
        }

    }

    lastEggX = x; // Update last egg position
    lastEggY = y; // Update last egg position


}

// Function to draw a filled triangle
void drawTriangle(int x1, int y1, int x2, int y2, int x3, int y3, uint16_t color) {
    // Use a simple method to draw the filled triangle
    for (int y = min(y1, min(y2, y3)); y <= max(y1, max(y2, y3)); y++) {
        // Calculate the x bounds of the triangle for this y
        int xLeft = x1, xRight = x1;

        if (y2 != y1) {
            float slope1 = (float)(x2 - x1) / (y2 - y1);
            xLeft = x1 + (int)((y - y1) * slope1);
        }
        if (y3 != y1) {
            float slope2 = (float)(x3 - x1) / (y3 - y1);
            xRight = x1 + (int)((y - y1) * slope2);
        }
        
        if (y3 != y2) {
            float slope3 = (float)(x3 - x2) / (y3 - y2);
            xLeft = min(xLeft, x2 + (int)((y - y2) * slope3));
            xRight = max(xRight, x2 + (int)((y - y2) * slope3));
        }

        // Draw the line segment for the current y
        for (int x = xLeft; x <= xRight; x++) {
            tft.drawPixel(x, y, color);
        }
    }
}

// Function to draw a green circle (small spot)
void drawSpot(int x, int y,  int radius, uint16_t color) {
    for (int i = -radius; i <= radius; i++) {
        for (int j = -radius; j <= radius; j++) {
            if (i * i + j * j <= radius * radius) { // Check if within the circle
                tft.drawPixel(x + i, y + j, color); // Draw the pixel
            }
        }
    }
}

// Function to rotate a point around a center point
void rotatePoint(int x, int y, float angle, float centerX, float centerY, int &rotatedX, int &rotatedY) {
    float radians = angle * (PI / 180.0); // Convert angle to radians
    rotatedX = centerX + (x - centerX) * cos(radians) - (y - centerY) * sin(radians);
    rotatedY = centerY + (x - centerX) * sin(radians) + (y - centerY) * cos(radians);
}

// Function to draw a rotated rectangle
void drawRotatedRectangle(int centerX, int centerY, int width, int height, float angle, uint16_t color) {
    // Compute the rectangle's corner points before rotation
    int halfWidth = width / 2;
    int halfHeight = height / 2;

    // Define the four corners of the rectangle relative to the center
    int corners[4][2] = {
        {centerX - halfWidth, centerY - halfHeight}, // Top-left
        {centerX + halfWidth, centerY - halfHeight}, // Top-right
        {centerX + halfWidth, centerY + halfHeight}, // Bottom-right
        {centerX - halfWidth, centerY + halfHeight}  // Bottom-left
    };

    // Create an array to hold the rotated coordinates
    int rotatedCorners[4][2];

    // Rotate each corner around the center point
    for (int i = 0; i < 4; i++) {
        rotatePoint(corners[i][0], corners[i][1], angle, centerX, centerY, rotatedCorners[i][0], rotatedCorners[i][1]);
    }

    // Draw the edges of the rectangle using line drawing logic
    for (int i = 0; i < 4; i++) {
        int next = (i + 1) % 4; // Get the next corner index
        drawLine(rotatedCorners[i][0], rotatedCorners[i][1], rotatedCorners[next][0], rotatedCorners[next][1], color);
    }

    // Optionally fill the rectangle using a fill algorithm (if desired)
    fillRotatedRectangle(rotatedCorners, color);
}

// Function to draw a line between two points
void drawLine(int x1, int y1, int x2, int y2, uint16_t color) {
    // Calculate differences
    int dx = x2 - x1;
    int dy = y2 - y1;
    int steps = abs(dx) > abs(dy) ? abs(dx) : abs(dy); // Number of steps to draw

    // Increment for each step
    float xIncrement = dx / (float)steps;
    float yIncrement = dy / (float)steps;

    float x = x1;
    float y = y1;

    // Draw the line pixel by pixel
    for (int i = 0; i <= steps; i++) {
        tft.drawPixel((int)x, (int)y, color); // Draw pixel
        x += xIncrement; // Increment x
        y += yIncrement; // Increment y
    }
}

// Function to optionally fill the rotated rectangle (simple fill method)
void fillRotatedRectangle(int rotatedCorners[4][2], uint16_t color) {
    // This function can be implemented using scanline or other methods
    // For simplicity, you can use a bounding box fill or another approach based on your graphics library's capabilities.
    // Here is a simple example of filling between y-min and y-max boundaries for the X range. 

    int minY = min(rotatedCorners[0][1], min(rotatedCorners[1][1], min(rotatedCorners[2][1], rotatedCorners[3][1])));
    int maxY = max(rotatedCorners[0][1], max(rotatedCorners[1][1], max(rotatedCorners[2][1], rotatedCorners[3][1])));

    // Fill between the y-bounds for x-bounds
    for (int y = minY; y <= maxY; y++) {
        // Calculate the leftmost and rightmost x-bounds at this y
        int leftX = INT_MAX, rightX = INT_MIN;

        for (int i = 0; i < 4; i++) {
            int j = (i + 1) % 4; // Next vertex
            // Check if the current edge intersects with the current y-line
            if ((rotatedCorners[i][1] <= y && rotatedCorners[j][1] > y) || (rotatedCorners[j][1] <= y && rotatedCorners[i][1] > y)) {
                // Calculate the x-coordinate of the intersection
                float slope = (float)(rotatedCorners[j][0] - rotatedCorners[i][0]) / (rotatedCorners[j][1] - rotatedCorners[i][1]);
                int intersectX = rotatedCorners[i][0] + (int)(slope * (y - rotatedCorners[i][1]));

                // Update left and right x-bounds
                leftX = min(leftX, intersectX);
                rightX = max(rightX, intersectX);
            }
        }

        // Fill the horizontal line segment between leftX and rightX at current y
        for (int x = leftX; x <= rightX; x++) {
            tft.drawPixel(x, y, color); // Draw pixel
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