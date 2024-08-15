

#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "pitches.h"
 
// notes in the melody:
int melody[] = {
  NOTE_C5, NOTE_D5, NOTE_E5, NOTE_F5, NOTE_G5, NOTE_A5, NOTE_B5, NOTE_C6};
int duration = 500;  // 500 miliseconds
int player1Plus = 2;
int player1Minus = 3;
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
// The pins for I2C are defined by the Wire-library. 
// On an arduino UNO:       A4(SDA), A5(SCL)
// On an arduino MEGA 2560: 20(SDA), 21(SCL)
// On an arduino LEONARDO:   2(SDA),  3(SCL), ...
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32s
#define TCAADDR 0x70

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

Adafruit_SSD1306 display2(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#define NUMFLAKES     10 // Number of snowflakes in the animation example

#define LOGO_HEIGHT   32
#define LOGO_WIDTH    32
struct Player
{
  unsigned int health = 40;
  int boss_damage = 0;
  bool alive = true;
  char name;
};
struct Player Player_1, Player_2;
// 'skull-crossbones-outline-icon', 256x256px
const unsigned char epd_bitmap [] PROGMEM = {
	0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0xc0, 0x00, 0x00, 0x1f, 0xf8, 0x00, 0x00, 0x3f, 0xfc, 0x00, 
	0x00, 0x78, 0x1e, 0x00, 0x00, 0xf0, 0x0f, 0x00, 0x00, 0xe0, 0x07, 0x00, 0x01, 0xc0, 0x03, 0x80, 
	0x01, 0xdc, 0x3b, 0x80, 0x01, 0x9e, 0x79, 0x80, 0x01, 0x9e, 0x79, 0x80, 0x01, 0x9c, 0x39, 0x80, 
	0x01, 0xc0, 0x03, 0x80, 0x01, 0xc1, 0x83, 0x80, 0x00, 0xe1, 0x87, 0x00, 0x00, 0xe0, 0x07, 0x00, 
	0x00, 0x76, 0x6e, 0x00, 0x00, 0x76, 0x6e, 0x00, 0x00, 0x76, 0x6e, 0x00, 0x00, 0x7f, 0xfe, 0x00, 
	0x18, 0x7f, 0xfe, 0x10, 0x1f, 0xe0, 0x07, 0xf8, 0x1f, 0xf8, 0x1f, 0xf8, 0x03, 0xff, 0xff, 0x80, 
	0x00, 0x3f, 0xfc, 0x00, 0x00, 0xff, 0xff, 0x00, 0x0f, 0xfc, 0x3f, 0xf0, 0x1f, 0xc0, 0x03, 0xf8, 
	0x1e, 0x00, 0x00, 0x78, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

static const unsigned char PROGMEM logo_bmp[] =
{ 0b00000000, 0b11000000,
  0b00000001, 0b11000000,
  0b00000001, 0b11000000,
  0b00000011, 0b11100000,
  0b11110011, 0b11100000,
  0b11111110, 0b11111000,
  0b01111110, 0b11111111,
  0b00110011, 0b10011111,
  0b00011111, 0b11111100,
  0b00001101, 0b01110000,
  0b00011011, 0b10100000,
  0b00111111, 0b11100000,
  0b00111111, 0b11110000,
  0b01111100, 0b11110000,
  0b01110000, 0b01110000,
  0b00000000, 0b00110000 };

void tcaselect(uint8_t i) {
  if (i > 7) return;

  Wire.beginTransmission(TCAADDR);
  Wire.write(1 << i);
  Wire.endTransmission();
}

void setup() {
  Serial.begin(9600);
  Serial.println("program started");
  Player_2.health = 30;
  pinMode(player1Plus, INPUT_PULLUP);  
  pinMode(player1Minus, INPUT_PULLUP); 

  tcaselect(1);
  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }
  // Show initial display buffer contents on the screen --
  // the library initializes this with an Adafruit splash screen.
  display.display();
  delay(2000); // Pause for 2 seconds
  // Clear the buffer
  display.clearDisplay();

  tcaselect(2);
  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display2.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }
  // Show initial display buffer contents on the screen --
  // the library initializes this with an Adafruit splash screen.
  display2.display();
  delay(2000); // Pause for 2 seconds
  // Clear the buffer
  display2.clearDisplay();

}

void loop() {
  tcaselect(1);
  displayPlayer1Health();
  tcaselect(2);
  displayPlayer2Health();
  if (digitalRead(player1Plus) == LOW)
  {
    printf("plus hit");
    incrementPlayerHealth(Player_1);
  }
  if (digitalRead(player1Minus) == LOW)
  {
    printf("minus hit");
    decrementPlayerHealth(Player_1);
  }
}

bool incrementPlayerHealth(Player &playerData) {
  playerData.health++;
  delay(100);
  return true;
}

bool decrementPlayerHealth(Player &playerData) {
  playerData.health--;
  if (playerData.health <= 0) {
    playerData.alive = false;
    animateDeath();
    delay(5000);
  }
  delay(100);
  return true;
}

void animateDeath(void) {
  tcaselect(1);
  display.clearDisplay();

  display.drawBitmap(
    (display.width()  - LOGO_WIDTH ) / 2,
    (display.height() - LOGO_HEIGHT) / 2,
    epd_bitmap, LOGO_WIDTH, LOGO_HEIGHT, 1);
  display.display();
  delay(3000);
}

void resetGame(void) {
  Player_1.health = 40;
  Player_1.boss_damage = 0;
  Player_2.health = 40;
  Player_2.boss_damage = 0;
}

void playDeathMusic(void) {
  for (int thisNote = 0; thisNote < 8; thisNote++) {
    // pin8 output the voice, every scale is 0.5 sencond
    tone(8, melody[thisNote], duration);
     
    // Output the voice after several minutes
    delay(1000);
  }
}

void playerResetMusic(void) {
  for (int thisNote = 0; thisNote < 8; thisNote++) {
    // pin8 output the voice, every scale is 0.5 sencond
    tone(8, melody[thisNote], duration);
     
    // Output the voice after several minutes
    delay(1000);
  }
}


void displayPlayer1Health(void) {
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(50, 10);
  display.print(Player_1.health);
  display.display();
}

void displayPlayer2Health(void) {
  display2.setTextSize(2);
  display2.setTextColor(SSD1306_WHITE);
  display2.setCursor(50, 10);
  display2.print(Player_2.health);
  display2.display();
}



#define XPOS   0 // Indexes into the 'icons' array in function below
#define YPOS   1
#define DELTAY 2

void testanimate(const uint8_t *bitmap, uint8_t w, uint8_t h) {
  int8_t f, icons[NUMFLAKES][3];

  // Initialize 'snowflake' positions
  for(f=0; f< NUMFLAKES; f++) {
    icons[f][XPOS]   = random(1 - LOGO_WIDTH, display.width());
    icons[f][YPOS]   = -LOGO_HEIGHT;
    icons[f][DELTAY] = random(1, 6);
    Serial.print(F("x: "));
    Serial.print(icons[f][XPOS], DEC);
    Serial.print(F(" y: "));
    Serial.print(icons[f][YPOS], DEC);
    Serial.print(F(" dy: "));
    Serial.println(icons[f][DELTAY], DEC);
  }

  for(;;) { // Loop forever...
    display.clearDisplay(); // Clear the display buffer

    // Draw each snowflake:
    for(f=0; f< NUMFLAKES; f++) {
      display.drawBitmap(icons[f][XPOS], icons[f][YPOS], bitmap, w, h, SSD1306_WHITE);
    }

    display.display(); // Show the display buffer on the screen
    delay(200);        // Pause for 1/10 second

    // Then update coordinates of each flake...
    for(f=0; f< NUMFLAKES; f++) {
      icons[f][YPOS] += icons[f][DELTAY];
      // If snowflake is off the bottom of the screen...
      if (icons[f][YPOS] >= display.height()) {
        // Reinitialize to a random position, just off the top
        icons[f][XPOS]   = random(1 - LOGO_WIDTH, display.width());
        icons[f][YPOS]   = -LOGO_HEIGHT;
        icons[f][DELTAY] = random(1, 6);
      }
    }
  }
}
