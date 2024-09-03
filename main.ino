

#include <SPI.h>
#include <Wire.h>
#include <stdlib.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <time.h>
#include <Arduino.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include "pitches.h"

AsyncWebServer server(80);

const char* ssid = "";
const char* password = "";

const char* PARAM_INPUT_1 = "player";
const char* PARAM_INPUT_2 = "name";

int resetMelody[] = {
  NOTE_C5, NOTE_E5, NOTE_G5, NOTE_C6};
int resetRhythm[] = { 100, 100, 100, 400 };
int deathMelody[] = {
  NOTE_DS4, NOTE_D4, NOTE_CS4, NOTE_C4, NOTE_CS4, NOTE_C4, NOTE_CS4, NOTE_C4, NOTE_CS4, NOTE_C4, NOTE_CS4, NOTE_C4
};
int deathRhythm[] = {500, 500, 500, 100, 100, 150, 200, 250, 300, 400, 500, 2000};

int duration = 100;  // 500 miliseconds
int player1Minus = 7;
int player1Plus = 10;
int player2Minus = 3;
int player2Plus = 4;

int buzzerPin = 6;

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels

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
  String name;
  int display;
};
struct Player Player_1, Player_2;
bool showStart = false;
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

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <title>Dan's MTG Health Counter Helper</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <link rel="icon" href="data:,">
  <style>
    html {font-family: Arial; display: inline-block; text-align: center;}
    h2 {font-size: 3.0rem;}
    p {font-size: 3.0rem;}
    body {max-width: 600px; margin:0px auto; padding-bottom: 25px;}
    button {background-color: #04AA6D;border: none;color: white;padding: 15px 32px;text-align: center;text-decoration: none;display: inline-block;font-size: 16px;
}
  </style>
</head>
<body>
  <h2>Dan's MTG Health Counter Helper</h2>
  <button onclick="toggleReset()">Reset Game</button>
<script>function toggleReset() {
  var xhr = new XMLHttpRequest();
  xhr.open("GET", "/reset", true)
  xhr.send();
}
</script>
</body>
</html>
)rawliteral";

void notFound(AsyncWebServerRequest *request) {
    request->send(404, "text/plain", "Not found");
}

void tcaselect(uint8_t i) {
  if (i > 7) return;

  Wire.beginTransmission(TCAADDR);
  Wire.write(1 << i);
  Wire.endTransmission();
}

int returnRandomNumber(int numPlayers) {
  return random(1, numPlayers + 1);
}

void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  if (WiFi.waitForConnectResult() != WL_CONNECTED) {
      Serial.printf("WiFi Failed!\n");
      return;
  }

  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "text/html", index_html);
  });
  server.on("/reset", HTTP_GET, [](AsyncWebServerRequest *request){
    resetGame();
    request->send(200, "text/plain", "Hello, world");
  });
  // Send a GET request to <ESP_IP>/update_name?player=<inputMessage1>&name=<inputMessage2>
  server.on("/update_name", HTTP_GET, [] (AsyncWebServerRequest *request) {
    String inputMessage1;
    String inputMessage2;
    // GET input1 value on <ESP_IP>/update_name?player=<inputMessage1>&name=<inputMessage2>
    if (request->hasParam(PARAM_INPUT_1) && request->hasParam(PARAM_INPUT_2)) {
      inputMessage1 = request->getParam(PARAM_INPUT_1)->value();
      inputMessage2 = request->getParam(PARAM_INPUT_2)->value();
      updatePlayerName(inputMessage2, Player_1);
      Serial.print(Player_1.name);
    }
    else {
      Serial.printf("No Param sent\n");
    }
    request->send(200, "text/plain", "OK");
  });
  playResetMusic();
  Player_1.display = 0;
  Player_2.display = 1;
  pinMode(player1Plus, INPUT_PULLUP);  
  pinMode(player1Minus, INPUT_PULLUP);
  pinMode(player2Plus, INPUT_PULLUP);  
  pinMode(player2Minus, INPUT_PULLUP);


  tcaselect(0);
  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }
  // Show initial display buffer contents on the screen --
  // the library initializes this with an Adafruit splash screen.
  display.display();
  delay(200); // Pause for 2 seconds
  // Clear the buffer
  display.clearDisplay();

  tcaselect(1);
  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display2.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }
  // Show initial display buffer contents on the screen --
  // the library initializes this with an Adafruit splash screen.
  display2.display();
  delay(500); // Pause for 2 seconds
  // Clear the buffer
  display2.clearDisplay();
  server.onNotFound(notFound);
  server.begin();

  int r_display = returnRandomNumber(2);
  displayFirst(r_display);

  delay(200);

}

void loop() {
  tcaselect(0);
  displayPlayer1Health();
  tcaselect(1);
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
  if (digitalRead(player2Plus) == LOW)
  {
    printf("plus hit");
    incrementPlayerHealth(Player_2);
  }
  if (digitalRead(player2Minus) == LOW)
  {
    printf("minus hit");
    decrementPlayerHealth(Player_2);
  }
}

void updatePlayerName(String playerName, Player &playerData) {
  playerData.name = playerName;
}
void displayFirst(int displayNumber) {
  displayCountdown();
  delay(100);
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(10, 5);
  display2.clearDisplay();
  display2.setTextSize(1);
  display2.setTextColor(SSD1306_WHITE);
  display2.setCursor(10, 5);
  if (displayNumber == 1) {
    tcaselect(0);
    display.clearDisplay();
    display.print("You are first!");
    display.display();
    tcaselect(1);
    display2.clearDisplay();
    display2.print("Womp Womp!");
    display2.display();
  } else {
    tcaselect(1);
    display2.print("You are first!");
    display2.display();
    tcaselect(0);
    display.clearDisplay();
    display.print("Womp Womp");
    display.display();
  };
  delay(5000);
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
    animateDeath(playerData.display);
    resetGame();
  }
  delay(100);
  return true;
}

void resetGame(void) {
  Player_1.health = 40;
  Player_1.boss_damage = 0;
  Player_2.health = 40;
  Player_2.boss_damage = 0;
  playResetMusic();
  showStart = true;
}

void displayCountdown(void) {
  tcaselect(0);
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(10, 5);
  display.print("Rolling for first!");
  display.display();
  tcaselect(1);
  display2.clearDisplay();
  display2.setTextSize(1);
  display2.setTextColor(SSD1306_WHITE);
  display2.setCursor(10, 5);
  display2.print("Rolling for first!");
  display2.display();
  delay(2000);
  for (int i = 5; i > 0; i--) {
    tcaselect(0);
    display.clearDisplay();
    display.setTextSize(2);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(10, 5);
    display.print(i);
    display.display();
    tcaselect(1);
    display2.clearDisplay();
    display2.setTextSize(2);
    display2.setTextColor(SSD1306_WHITE);
    display2.setCursor(10, 5);
    display2.print(i);
    display2.display();
    delay(500);
  }
}

void animateDeath(int displayNumber) {
  tcaselect(displayNumber);
  display.clearDisplay();
  display.drawBitmap(
    (display.width()  - LOGO_WIDTH ) / 2,
    (display.height() - LOGO_HEIGHT) / 2,
    epd_bitmap, LOGO_WIDTH, LOGO_HEIGHT, 1);
  display.display();
  playDeathMusic();
  display.startscrollright(0x00, 0x0F);
  delay(1000);
  display.stopscroll();
  delay(1000);
  display.startscrollleft(0x00, 0x0F);
  delay(2000);
  display.stopscroll();
/*   delay(1000);
  display.startscrolldiagright(0x00, 0x07);
  delay(2000);
  display.startscrolldiagleft(0x00, 0x07);
  delay(2000);
  display.stopscroll(); */
  delay(1000);

}

void playDeathMusic(void) {
  for (int i = 0; i < 12; i++) {
    tone(buzzerPin, deathMelody[i], deathRhythm[i]);
    delay(deathRhythm[i]);
  }
}

void playResetMusic(void) {
  for (int i = 0; i < 4; i++) {
    tone(buzzerPin, resetMelody[i], resetRhythm[i]);
    delay(resetRhythm[i]);
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
  display2.clearDisplay();
  display2.setTextSize(2);
  display2.setTextColor(SSD1306_WHITE);
  display2.setCursor(50, 10);
  display2.print(Player_2.health);
  display2.display();
}
