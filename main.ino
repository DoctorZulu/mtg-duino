

#include <SPI.h>
#include <Wire.h>
#include <stdlib.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <time.h>
#include <Arduino.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include "vars.h"
#include <ESPAsyncWebServer.h>
#include "pitches.h"


AsyncWebServer server(80);

const char* ssid = SSID;
const char* password = PASSWORD;

const char* PARAM_INPUT_1 = "player";
const char* PARAM_INPUT_2 = "name";

int resetMelody[] = {
  NOTE_C5, NOTE_E5, NOTE_G5, NOTE_C6};
int resetRhythm[] = { 100, 100, 100, 400 };
int deathMelody[] = {
  NOTE_DS4, NOTE_D4, NOTE_CS4, NOTE_C4, NOTE_CS4, NOTE_C4, NOTE_CS4, NOTE_C4, NOTE_CS4, NOTE_C4, NOTE_CS4, NOTE_C4
};
int deathRhythm[] = {500, 500, 500, 100, 100, 150, 200, 250, 300, 400, 500, 2000};



int player1Minus = 3;
int player1Plus = 4;
int player1Alt = 2;
int player2Minus = 7;
int player2Plus = 10;
int player2Alt = 5;

int buzzerPin = 6;

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels

#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32s
#define TCAADDR 0x70

//Array of displays eventually this number can be adjusted along with player count
Adafruit_SSD1306 displays[2] = { {SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET}, {SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET} };

#define LOGO_HEIGHT   32
#define LOGO_WIDTH    32
#define STARTING_HEALTH     40 // Reset pin # (or -1 if sharing Arduino reset pin)
#define CMDR_DAMAGE 0 ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32s
#define ALIVE true
#define SCREEN_STATE 0

struct Player
{
  unsigned int health;
  int cmdr_damage;
  bool alive;
  String name;
  int display;
  bool screenState;
};
Player players[2] = { {STARTING_HEALTH, CMDR_DAMAGE, ALIVE, "Player 1", 1, SCREEN_STATE}, {STARTING_HEALTH, CMDR_DAMAGE, ALIVE, "Player 2", 0, SCREEN_STATE} }; // Array of Player structs
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

  <div>
    <input id="pName" name="pName" placeholder="Player Name"></input>
    <input id="pNumber" name="pNumber" placeholder="Player Number"></input>
    <button onclick="setName()">Set Name</button>
  </div>
  
<script>
function toggleReset() {
  var xhr = new XMLHttpRequest();
  xhr.open("GET", "/reset", true)
  xhr.send();
}
function setName() {
  var xhr = new XMLHttpRequest();
  let pName = document.getElementById("pName").value;
  let pNumber = document.getElementById("pNumber").value;
  console.log("/update_name?player=" + pNumber + "&name=" + pName)
  xhr.open("GET", "/update_name?player=" + pNumber + "&name=" + pName, true)
  xhr.send();
}
</script>
</body>
</html>
)rawliteral";

unsigned long timer = 0; // the time the delay started

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
  Serial.println("setup online");
  timer = millis();   // start delay for non blocking functions
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
      int playerNum = inputMessage1.toInt();
      playerNum--;
      updatePlayerName(inputMessage2, players[playerNum]);
    }
    else {
      Serial.printf("No Param sent\n");
    }
    request->send(200, "text/plain", "OK");
  });
  playResetMusic();
  //players[0].display = 0;
  //players[1].display = 1;
  pinMode(player1Plus, INPUT_PULLUP);  
  pinMode(player1Minus, INPUT_PULLUP);
  pinMode(player2Plus, INPUT_PULLUP);  
  pinMode(player2Minus, INPUT_PULLUP);


  tcaselect(0);
  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!displays[0].begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }
  // Show initial display buffer contents on the screen --
  // the library initializes this with an Adafruit splash screen.
  displays[0].display();
  delay(200); // Pause for 2 seconds
  // Clear the buffer
  displays[0].clearDisplay();

  tcaselect(1);
  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!displays[1].begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }
  // Show initial display buffer contents on the screen --
  // the library initializes this with an Adafruit splash screen.
  displays[1].display();
  delay(500); // Pause for 2 seconds
  // Clear the buffer
  displays[1].clearDisplay();
  server.onNotFound(notFound);
  server.begin();

  int r_display = returnRandomNumber(2);
  displayFirst(r_display);

  delay(200);

}

void loop() {
  unsigned long currentTime = millis();
  tcaselect(players[0].display);
  if (players[0].screenState == 0){
    displayPlayerData(players[0]);
  } else {
    displayPlayerCmdrDamage(players[0]);
  }
  tcaselect(players[1].display);
  if (players[1].screenState == 0){
    displayPlayerData(players[1]);
  } else {
    displayPlayerCmdrDamage(players[1]);
  }
  if ((digitalRead(player1Alt) == LOW) && ((currentTime - timer) >= 100))
  {
    Serial.println("screenstate changed");
    toggleScreen(players[0]);
    timer = currentTime;
  }
  if (digitalRead(player2Alt) == LOW && ((currentTime - timer) >= 100))
  {
    Serial.println("screenstate changed");
    toggleScreen(players[1]);
    timer = currentTime;
  }
  if (digitalRead(player1Plus) == LOW && ((currentTime - timer) >= 100))
  {
    if (players[0].screenState == 0) {
      incrementPlayerHealth(players[0]);
    } else {
      incrementCmdrDamage(players[0]);
    }
    timer = currentTime;
  }
  if (digitalRead(player1Minus) == LOW && ((currentTime - timer) >= 100))
  {
    if (players[0].screenState == 0) {
      decrementPlayerHealth(players[0]);
    } else {
      decrementCmdrDamage(players[0]);
    }
    timer = currentTime;
  }
  if (digitalRead(player2Plus) == LOW && ((currentTime - timer) >= 100))
  {
    if (players[1].screenState == 0) {
      incrementPlayerHealth(players[1]);
    } else {
      incrementCmdrDamage(players[1]);
    }
    timer = currentTime;
  }
  if (digitalRead(player2Minus) == LOW && ((currentTime - timer) >= 100))
  {
    if (players[1].screenState == 0) {
      decrementPlayerHealth(players[1]);
    } else {
      decrementCmdrDamage(players[1]);
    }
    timer = currentTime;
  }
}

void toggleScreen(Player &playerData) {
    playerData.screenState = !playerData.screenState;
    //delay(100);
}
void updatePlayerName(String playerName, Player &playerData) {
  playerData.name = playerName;
}

void displayFirst(int displayNumber) {
  displayCountdown();
  delay(100);
  displays[0].setTextSize(1);
  displays[0].setTextColor(SSD1306_WHITE);
  displays[0].setCursor(10, 5);
  displays[1].clearDisplay();
  displays[1].setTextSize(1);
  displays[1].setTextColor(SSD1306_WHITE);
  displays[1].setCursor(10, 5);
  if (displayNumber == 1) {
    tcaselect(players[0].display);
    displays[0].clearDisplay();
    displays[0].print("You are first!");
    displays[0].display();
    tcaselect(players[1].display);
    displays[1].clearDisplay();
    displays[1].print("Womp Womp!");
    displays[1].display();
  } else {
    tcaselect(players[1].display);
    displays[1].print("You are first!");
    displays[1].display();
    tcaselect(players[0].display);
    displays[0].clearDisplay();
    displays[0].print("Womp Womp");
    displays[0].display();
  };
  delay(5000);
}

bool incrementPlayerHealth(Player &playerData) {
  playerData.health++;
  //delay(100);
  return true;
}

bool incrementCmdrDamage(Player &playerData) {
  playerData.health--;
  playerData.cmdr_damage++;
  if (playerData.cmdr_damage >= 21) {
    playerData.alive = false;
    animateDeath(playerData.display);
    resetGame();
  }
  //delay(100);
  return true;
}

bool decrementPlayerHealth(Player &playerData) {
  playerData.health--;
  if (playerData.health <= 0) {
    playerData.alive = false;
    animateDeath(playerData.display);
    resetGame();
  }
  //delay(100);
  return true;
}

bool decrementCmdrDamage(Player &playerData) {
  playerData.health++;
  playerData.cmdr_damage--;
  //delay(100);
  return true;
}

void resetGame(void) {
  players[0].health = 40;
  players[0].cmdr_damage = 0;
  players[0].screenState = 0;
  players[1].health = 40;
  players[1].cmdr_damage = 0;
  players[1].screenState = 0;
  playResetMusic();
  showStart = true;
}

void displayCountdown(void) {
  tcaselect(players[0].display);
  displays[0].clearDisplay();
  displays[0].setTextSize(1);
  displays[0].setTextColor(SSD1306_WHITE);
  displays[0].setCursor(10, 5);
  displays[0].print("Rolling for first!");
  displays[0].display();
  tcaselect(players[1].display);
  displays[1].clearDisplay();
  displays[1].setTextSize(1);
  displays[1].setTextColor(SSD1306_WHITE);
  displays[1].setCursor(10, 5);
  displays[1].print("Rolling for first!");
  displays[1].display();
  delay(2000);
  for (int i = 5; i > 0; i--) {
    tcaselect(players[1].display);
    displays[0].clearDisplay();
    displays[0].setTextSize(2);
    displays[0].setTextColor(SSD1306_WHITE);
    displays[0].setCursor(50, 10);
    displays[0].print(i);
    displays[0].display();
    tcaselect(players[0].display);
    displays[1].clearDisplay();
    displays[1].setTextSize(2);
    displays[1].setTextColor(SSD1306_WHITE);
    displays[1].setCursor(50, 10);
    displays[1].print(i);
    displays[1].display();
    delay(500);
  }
}

void animateDeath(int displayNumber) {
  tcaselect(displayNumber);
  displays[0].clearDisplay();
  displays[0].drawBitmap(
    (displays[0].width()  - LOGO_WIDTH ) / 2,
    (displays[0].height() - LOGO_HEIGHT) / 2,
    epd_bitmap, LOGO_WIDTH, LOGO_HEIGHT, 1);
  displays[0].display();
  playDeathMusic();
  displays[0].startscrollright(0x00, 0x0F);
  delay(1000);
  displays[0].stopscroll();
  delay(1000);
  displays[0].startscrollleft(0x00, 0x0F);
  delay(2000);
  displays[0].stopscroll();
/*   delay(1000);
  displays[0].startscrolldiagright(0x00, 0x07);
  delay(2000);
  displays[0].startscrolldiagleft(0x00, 0x07);
  delay(2000);
  displays[0].stopscroll(); */
  delay(1000);

}

void playDeathMusic(void) {
  for (int i = 0; i < 12; i++) {
    tone(buzzerPin, deathMelody[i], deathRhythm[i]);
    //delay(deathRhythm[i]);
  }
}

void playResetMusic(void) {
  for (int i = 0; i < 4; i++) {
    tone(buzzerPin, resetMelody[i], resetRhythm[i]);
    //delay(resetRhythm[i]);
  }
}


void displayPlayerData(Player &playerData) {
  displays[playerData.display].clearDisplay();
  displays[playerData.display].setTextSize(1);
  displays[playerData.display].setTextColor(SSD1306_WHITE);
  displays[playerData.display].setCursor(50, 0);
  displays[playerData.display].print(playerData.name);
  displays[playerData.display].setTextSize(2);
  displays[playerData.display].setTextColor(SSD1306_WHITE);
  displays[playerData.display].setCursor(50, 10);
  displays[playerData.display].print(playerData.health);
  displays[playerData.display].display();
}


void displayPlayerCmdrDamage(Player &playerData) {
  displays[playerData.display].clearDisplay();
  displays[playerData.display].setTextSize(1);
  displays[playerData.display].setTextColor(SSD1306_WHITE);
  displays[playerData.display].setCursor(5,10);
  displays[playerData.display].print("CMDR:");
  displays[playerData.display].setTextSize(2);
  displays[playerData.display].setCursor(50, 10);
  displays[playerData.display].print(playerData.cmdr_damage);
  displays[playerData.display].display();
}

