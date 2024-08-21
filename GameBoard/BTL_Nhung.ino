#include <SSD1306.h>
#include <Preferences.h>
#include "asset.h"

#define LEFT_BUTTON_PIN 23
#define RIGHT_BUTTON_PIN 33
#define BOOT_BUTTON_PIN 0
#define BUZZER_PIN 18

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

#define MENU_ITEM_COUNT 4
#define MENU_ITEM_DISPLAY_COUNT 4

#define MENU_PAGE 0
#define PLAY_GAME_PAGE 1
#define END_GAME_PAGE 2

#define TUBE_DISTANCE 32
#define TUBE_WIDTH 6
#define PATH_WIDTH 30

#define RACING_CAR_ROW_COUNT 3
#define RACING_CAR_DISTANCE 32

#define FRUIT_COLUMN_COUNT 4

#define CACTUS_DISTANCE 47

// IIC default address is 0x3c
SSD1306 display(0x3c, 21, 22);

// Preferences to read/write flash memory
Preferences preferences;

const unsigned long DEBOUNCE_DELAY = 250;  // Thời gian debounce (ms)
unsigned long lastLeftButtonPressTime = 0;
unsigned long lastRightButtonPressTime = 0;
unsigned long lastCreateBulletTime = 0;
unsigned long keyPressTime = 0;
unsigned long millis_value = 0;

char menuItems[MENU_ITEM_COUNT][20] = {
  "Flappy Bird", "Racing Car", "Catch The Fruits", "Dinosaur Run"
};
char *gameName;
int choosenItemIndex = 0, firstItemIndex = 0, lastItemIndex = MENU_ITEM_COUNT;
int page = MENU_PAGE;

int gameState = 0;  // 0-Màn hình bắt đầu, 1-Màn hình chơi, 2-Màn hình kết thúc
float speed = 0.03;
int score = 0;
bool hasScored[10] = { false };
unsigned int highScore;
float playerX, playerY;
bool isBuzzerOn = false;
// bool increaseSpeed = false;

// Flappy Bird________________________________________
// At each time, there are only a maximum of 4 tubes on the screen
float tubeX[4];
int bottomTubeHeight[4];
bool isFlyingUp = false;
// ___________________________________________________

// Racing car_________________________________________
float obstacleCarsX[RACING_CAR_ROW_COUNT], obstacleCarsY[RACING_CAR_ROW_COUNT], bulletX = 128.0, bulletY = -10.00;
int carRow = 1;
bool isExistBullet = false;
int obstacleCarType[RACING_CAR_ROW_COUNT] = { 1 };
int obstacleCarWidth[2] = { Car_width, Truck_width };
// ___________________________________________________

// Catch the fruits___________________________________
float fruitColumnX[FRUIT_COLUMN_COUNT], fruitColumnY[FRUIT_COLUMN_COUNT];
int basketColumn = 1;
bool isRockColumn[FRUIT_COLUMN_COUNT] = { false };
int fruitType[FRUIT_COLUMN_COUNT] = { -1 };
// ___________________________________________________

// Dinosaur run_______________________________________
float cactusX[3], groundX[2];
int topCactusY[3], cactusDisplay[3];
int cactusWidth[4] = { Cactus1Tall_width, Cactus2Short_width, Cactus3Tall_width, Cactus1Tall_width };
int cactusHeight[4] = { Cactus1Tall_height, Cactus2Short_height, Cactus3Tall_height, Cactus1Tall_height };
const unsigned char *CactusArray[] = { Cactus1Tall, Cactus2Short, Cactus3Tall, Cactus1Tall };
bool isJumping = false, isFalling = false;
float speedUp, speedDown;
float speedUpArray[] = { 0.012, 0.021, 0.039, 0.07 }, speedDownArray[] = { 0.013, 0.02, 0.026, 0.03 };
unsigned long dinoTopTime = 0;
unsigned int dinoTopDelay = 0;
bool dinoState = true;
unsigned long stateTime = 0;
// ___________________________________________________

void setup() {
  pinMode(LEFT_BUTTON_PIN, INPUT_PULLUP);
  pinMode(RIGHT_BUTTON_PIN, INPUT_PULLUP);
  pinMode(BUZZER_PIN, OUTPUT);

  display.init();

  // Make all asset in the game is right side up
  display.flipScreenVertically();
}

void loop() {
  display.clear();
  display.setFont(ArialMT_Plain_10);
  display.setColor(WHITE);
  millis_value = millis();

  // Menu page
  if (page == MENU_PAGE) {
    DisplayMenuPage();
  }

  // Main page
  else if (page == PLAY_GAME_PAGE) {
    // Game Racing Car
    if (choosenItemIndex == 0) {
      FlappyBird();
    } else if (choosenItemIndex == 1) {
      RacingCar();
    } else if (choosenItemIndex == 2) {
      CatchTheFruits();
    } else {
      DinosaurRun();
    }
  }

  // End game page
  else if (page == END_GAME_PAGE) {
    DisplayEndGamePage();
  }

  display.display();
}

// Hiển thị màn hình menu
void DisplayMenuPage() {
  for (int i = 0; i < MENU_ITEM_COUNT; i++) {
    if (i == choosenItemIndex) {
      display.setFont(ArialMT_Plain_16);
    }
    display.drawString(0, i * 16 + 1, menuItems[i]);
    display.setFont(ArialMT_Plain_10);
  }

  // Ấn nút bên phải để chuyển game trong menu
  if (digitalRead(RIGHT_BUTTON_PIN) == LOW && millis_value - lastRightButtonPressTime > DEBOUNCE_DELAY) {
    lastRightButtonPressTime = millis_value;
    choosenItemIndex = (choosenItemIndex + 1) % MENU_ITEM_COUNT;
  }

  // Ấn nút bên trái để chọn game
  if (digitalRead(LEFT_BUTTON_PIN) == LOW && millis_value - lastLeftButtonPressTime > DEBOUNCE_DELAY) {
    lastLeftButtonPressTime = millis_value;
    gameName = menuItems[choosenItemIndex];
    preferences.begin(gameName, false);
    // Get the high score, if the key does not exist, return a default value of 0
    highScore = preferences.getUInt("highScore", 0);
    // Close the Preferences
    preferences.end();
    page = PLAY_GAME_PAGE;
  }
  display.display();
}

// Hiển thị màn hình kết thúc
void DisplayEndGamePage() {
  display.setFont(ArialMT_Plain_16);
  display.drawString(0, 0, "Your score: " + String(score));
  display.drawString(0, 20, "High score: " + String(highScore));

  display.setFont(ArialMT_Plain_10);
  display.drawString(10, 44, "Restart");
  display.drawString(90, 44, "Exit");

  display.setFont(ArialMT_Plain_10);
  display.drawString(5, 54, "Click BOOT to reset score");

  if (digitalRead(LEFT_BUTTON_PIN) == LOW) {
    page = PLAY_GAME_PAGE;
    gameState = 0;
    delay(200);
  }

  if (digitalRead(RIGHT_BUTTON_PIN) == LOW) {
    page = MENU_PAGE;
    gameState = 0;
    delay(200);
  }

  // If BOOT button is pressed, reset high score in game and in the flash memory
  if (digitalRead(BOOT_BUTTON_PIN) == LOW) {
    score = 0;
    highScore = 0;

    // Write high score to flash memory
    preferences.begin(gameName, false);
    preferences.putUInt("highScore", highScore);
    preferences.end();
  }
}

// Game Flappy bird
void FlappyBird() {
  // Display start screen
  if (gameState == 0) {
    // Reinitialize in-game status
    score = 0;
    speed = 0.01;
    playerX = 28.88;
    playerY = 28.00;

    display.setFont(ArialMT_Plain_16);
    display.drawString(0, 4, "Flappy ");

    display.drawXbm(64, 0, Building_width, Building_height, Building);
    display.drawXbm(playerX, playerY, Flappy_width, Flappy_height, Flappy);
    display.setColor(WHITE);

    display.setFont(ArialMT_Plain_10);
    display.drawString(0, 46, "Start");
    display.drawString(46, 46, "Exit");

    // Reinitialize all tubes
    for (int i = 0; i < 4; i++) {
      tubeX[i] = 128 + ((i + 1) * TUBE_DISTANCE);
      bottomTubeHeight[i] = random(8, 32);
      hasScored[i] = false;
    }

    if (digitalRead(LEFT_BUTTON_PIN) == LOW && millis_value - lastLeftButtonPressTime > DEBOUNCE_DELAY) {
      lastLeftButtonPressTime = millis_value;
      gameState = 1;
      delay(100);
    }

    if (digitalRead(RIGHT_BUTTON_PIN) == LOW && millis_value - lastRightButtonPressTime > DEBOUNCE_DELAY) {
      lastRightButtonPressTime = millis_value;
      page = MENU_PAGE;
      gameState = 0;
      delay(100);
    }
  }

  // Display in-game screen
  else if (gameState == 1) {
    // Display score
    display.setFont(ArialMT_Plain_10);
    display.drawString(3, 0, String(score));

    // Setup variables and flags if button is pressed
    if (digitalRead(LEFT_BUTTON_PIN) == LOW) {
      keyPressTime = millis();
      isFlyingUp = true;
      isBuzzerOn = true;
    }

    // Display bird and tubes
    display.setColor(WHITE);
    display.drawXbm(playerX, playerY, Flappy_width, Flappy_height, Flappy);

    for (int i = 0; i < 4; i++) {
      display.fillRect(tubeX[i], 0, TUBE_WIDTH, bottomTubeHeight[i]);
      display.fillRect(tubeX[i], bottomTubeHeight[i] + PATH_WIDTH, TUBE_WIDTH, SCREEN_HEIGHT - bottomTubeHeight[i] - PATH_WIDTH);
    }

    for (int i = 0; i < 4; i++) {
      // Move all tube to the left
      tubeX[i] -= speed;

      // If a tube pass the bird, add a point
      if (tubeX[i] < playerX && !hasScored[i]) {
        score++;
        hasScored[i] = true;

        // Increase speed every 10 tubes
        if (score % 10 == 0) {
          speed += 0.01;
        }
      }

      // If a tube pass the screen, reinitialize that tube on the right of the screen
      if (tubeX[i] + TUBE_WIDTH < 0) {
        bottomTubeHeight[i] = random(8, 32);
        tubeX[i] = 128;
        hasScored[i] = false;
      }
    }

    // The buzzer will make sound for 10 milliseconds
    if ((keyPressTime + 10) < millis()) {
      isBuzzerOn = false;
    }

    ClickSound();

    // The bird will fly up for 80 milliseconds
    if ((keyPressTime + 80) < millis()) {
      isFlyingUp = false;
    }

    // Bird fly up (y-axis is reverted)
    if (isFlyingUp) {
      playerY -= 0.025;
    } else {
      playerY += 0.015;
    }


    // Check if out of bound on vertical axis
    if ((playerY + Flappy_height) >= 64 || playerY < 0) {
      EndingSound();

      if (score > highScore) {
        highScore = score;

        // Write new high score to flash memory
        preferences.begin("Flappy", false);
        preferences.putUInt("highScore", highScore);
        preferences.end();
      }

      gameState = 2;
    }

    // Check for collision with tube
    for (int i = 0; i < 4; i++) {
      if (tubeX[i] <= (playerX + 7) && (playerX + 7) <= tubeX[i] + 6) {
        if (playerY <= bottomTubeHeight[i] || (playerY + Flappy_height) >= (bottomTubeHeight[i] + PATH_WIDTH)) {

          EndingSound();

          if (score > highScore) {
            highScore = score;

            // Write new high score to flash memory
            preferences.begin(gameName, false);
            preferences.putUInt("highScore", highScore);
            preferences.end();
          }

          gameState = 2;

          delay(50);
        }
      }
    }

    // Display boundary
    display.drawRect(0, 0, 128, 64);
  }

  // Display ending (score) screen
  else {
    page = END_GAME_PAGE;
  }
}

// Game Racing car
void RacingCar() {
  // Display start screen
  if (gameState == 0) {
    // Reinitialize in-game status
    score = 0;
    speed = 0.03;
    playerX = 22.00;
    playerY = 32.00;

    display.setFont(ArialMT_Plain_16);
    display.drawString(0, 4, "Racing Car");

    display.drawXbm(80, 25, Car_width, Car_height, CarReverse);
    display.drawXbm(playerX, playerY, Car_width, Car_height, Car);
    display.setColor(WHITE);
    // display.fillRect(0, SCREEN_HEIGHT - 5, SCREEN_WIDTH, 5);

    display.setFont(ArialMT_Plain_10);
    display.drawString(10, 50, "Start");
    display.drawString(90, 50, "Exit");

    if (digitalRead(LEFT_BUTTON_PIN) == LOW && millis_value - lastLeftButtonPressTime > DEBOUNCE_DELAY) {
      lastLeftButtonPressTime = millis_value;
      gameState = 1;
      carRow = 1;
      lastLeftButtonPressTime = millis_value;
      playerX = 10.00;
      playerY = carRow * 21 + 3;
      // Init the obstacle cars
      for (int i = 0; i < RACING_CAR_ROW_COUNT; i++) {
        obstacleCarsX[i] = 128 + ((i + 1) * RACING_CAR_DISTANCE * random(i + 1, 8));
        obstacleCarsY[i] = i * 21 + 2;
        hasScored[i] = false;
      }
      delay(100);
    }

    if (digitalRead(RIGHT_BUTTON_PIN) == LOW && millis_value - lastRightButtonPressTime > DEBOUNCE_DELAY) {
      lastRightButtonPressTime = millis_value;
      page = MENU_PAGE;
      gameState = 0;
      delay(100);
    }
  }

  else if (gameState == 1) {
    // Display score
    display.setFont(ArialMT_Plain_10);
    display.drawString(3, 0, String(score));
    display.fillRect(0, 20, 128, 2);
    display.fillRect(0, 41, 128, 2);
    unsigned long millis_value = millis();
    if (digitalRead(LEFT_BUTTON_PIN) == LOW && millis_value - lastCreateBulletTime > DEBOUNCE_DELAY && score >= 3) {
      bool checkWhiteButton = false;
      for (int i = 0; i < DEBOUNCE_DELAY; i++) {
        if (digitalRead(RIGHT_BUTTON_PIN) == LOW) {
          checkWhiteButton = true;
          break;
        }
      }
      if (checkWhiteButton) {
        lastCreateBulletTime = millis_value;
        lastLeftButtonPressTime = millis_value;
        lastRightButtonPressTime = millis_value;
        score -= 3;
        bulletX = playerX + Car_width;
        bulletY = playerY + 5;
        isExistBullet = true;
      }
    }

    // Display player car and obstacle cars
    display.setColor(WHITE);
    display.drawXbm(playerX, playerY, Car_width, Car_height, Car);
    display.drawXbm(bulletX, bulletY, Bullet_width, Bullet_height, Bullet);

    // Setup variables and flags if button is pressed
    if (digitalRead(LEFT_BUTTON_PIN) == LOW && millis_value - lastLeftButtonPressTime > DEBOUNCE_DELAY && !isExistBullet) {
      keyPressTime = millis();
      isBuzzerOn = true;
      lastLeftButtonPressTime = millis_value;
      if (carRow > 0) {
        carRow -= 1;
      }
      playerY = carRow * 21 + 3;
    }

    if (digitalRead(RIGHT_BUTTON_PIN) == LOW && millis_value - lastRightButtonPressTime > DEBOUNCE_DELAY && !isExistBullet) {
      keyPressTime = millis();
      isBuzzerOn = true;
      lastRightButtonPressTime = millis_value;
      if (carRow < RACING_CAR_ROW_COUNT - 1) {
        carRow += 1;
      }
      playerY = carRow * 21 + 3;
    }

    ClickSound();

    for (int i = 0; i < RACING_CAR_ROW_COUNT; i++) {
      if (obstacleCarType[i] == 0) {
        display.drawXbm(obstacleCarsX[i], obstacleCarsY[i], Car_width, Car_height, CarReverse);
      } else {
        display.drawXbm(obstacleCarsX[i], obstacleCarsY[i], Truck_width, Truck_height, Truck);
      }
    }
    int checkCarType;

    for (int i = 0; i < RACING_CAR_ROW_COUNT; i++) {
      if (obstacleCarType[i] == 0) {
        checkCarType = 0;
      } else {
        checkCarType = 1;
      }
      obstacleCarsX[i] -= speed;
      if (isExistBullet) {
        bulletX += speed;
      }

      if (obstacleCarsX[i] < playerX && !hasScored[i]) {
        score++;
        hasScored[i] = true;

        // Increase speed every 10 cars
        if (score % 10 == 0) {
          speed += 0.01;
        }
      }

      // Xe chướng ngại vật ra khỏi màn chơi sẽ dịch chuyển về phải
      if (obstacleCarType[i] == 0 && obstacleCarsX[i] + Car_width < 0) {
        obstacleCarsX[i] = 128 + ((i + 1) * RACING_CAR_DISTANCE * random(0, 6));
        hasScored[i] = false;
        obstacleCarType[i] = random(0, 2);
      }

      if (obstacleCarType[i] != 0 && obstacleCarsX[i] + Truck_width < 0) {
        obstacleCarsX[i] = 128 + ((i + 1) * RACING_CAR_DISTANCE * random(0, 6));
        hasScored[i] = false;
        obstacleCarType[i] = random(0, 2);
      }

      if (isExistBullet && -10 <= obstacleCarsY[i] - bulletY && obstacleCarsY[i] - bulletY <= 10 && ((obstacleCarsX[i] <= bulletX && bulletX <= obstacleCarsX[i] + obstacleCarWidth[checkCarType]) || (obstacleCarsX[i] <= bulletX + Bullet_width && bulletX + Bullet_width <= obstacleCarsX[i] + obstacleCarWidth[checkCarType]))) {
        obstacleCarsX[i] = 128 + ((i + 1) * RACING_CAR_DISTANCE * random(0, 6));
        bulletX = 128;
        bulletY = -20.0;
        isExistBullet = false;
      }

      if (bulletX >= 128) {
        isExistBullet = false;
      }
    }

    // The buzzer will make sound for 10 milliseconds
    if ((keyPressTime + 50) < millis()) {
      isBuzzerOn = false;
    }

    // Check collision player with other car
    for (int i = 0; i < RACING_CAR_ROW_COUNT; i++) {
      if (obstacleCarType[i] == 0) {
        checkCarType = 0;
      } else {
        checkCarType = 1;
      }
      if (-3 <= obstacleCarsY[i] - playerY && obstacleCarsY[i] - playerY <= 3 && ((obstacleCarsX[i] <= playerX && playerX <= obstacleCarsX[i] + obstacleCarWidth[checkCarType]) || (obstacleCarsX[i] <= playerX + Car_width && playerX + Car_width <= obstacleCarsX[i] + obstacleCarWidth[checkCarType]))) {
        EndingSound();

        if (score > highScore) {
          highScore = score;

          // Write new high score to flash memory
          preferences.begin(gameName, false);
          preferences.putUInt("highScore", highScore);
          preferences.end();
        }

        gameState = 2;

        delay(50);
      }
    }

    // Display boundary
    display.drawRect(0, 0, 128, 64);
  }

  else if (gameState == 2) {
    page = END_GAME_PAGE;
  }
}

// Game hứng hoa quả
void CatchTheFruits() {
  if (gameState == 0) {
    // Reinitialize in-game status
    score = 0;
    speed = 0.015;
    playerX = 10.0;
    playerY = 25.0;

    display.setFont(ArialMT_Plain_16);
    display.drawString(0, 4, "Catch the Fruits");

    // display.drawXbm(75, 40, Rock_width, Rock_height, Rock);
    display.drawXbm(75, 25, Fruit_width, Fruit_height, Banana);
    display.drawXbm(94, 25, Fruit_width, Fruit_height, Apple);
    display.drawXbm(playerX, playerY + 8, Basket_width, Basket_height, Basket);
    display.setColor(WHITE);


    display.setFont(ArialMT_Plain_10);
    display.drawString(10, 50, "Start");
    display.drawString(90, 50, "Exit");

    if (digitalRead(LEFT_BUTTON_PIN) == LOW && millis_value - lastLeftButtonPressTime > DEBOUNCE_DELAY) {
      lastLeftButtonPressTime = millis_value;
      gameState = 1;
      basketColumn = 1;
      lastLeftButtonPressTime = millis();
      lastRightButtonPressTime = millis();
      playerY = 45.00;
      playerX = basketColumn * 32 + 1;
      // Init the fruits
      for (int i = 0; i < FRUIT_COLUMN_COUNT; i++) {
        fruitColumnX[i] = i * 32 + 12;
        fruitColumnY[i] = 0;
        fruitType[i] = random(1, 5);
      }
      delay(100);
    }

    if (digitalRead(RIGHT_BUTTON_PIN) == LOW && millis_value - lastRightButtonPressTime > DEBOUNCE_DELAY) {
      lastRightButtonPressTime = millis_value;
      page = MENU_PAGE;
      gameState = 0;
      delay(100);
    }
  }

  else if (gameState == 1) {

    // Display score
    display.setFont(ArialMT_Plain_10);
    display.drawString(3, 0, String(score));
    // Setup variables and flags if button is pressed
    if (digitalRead(LEFT_BUTTON_PIN) == LOW && millis_value - lastLeftButtonPressTime > DEBOUNCE_DELAY) {
      keyPressTime = millis();
      millis_value = keyPressTime;
      lastLeftButtonPressTime = millis_value;
      isBuzzerOn = true;
      if (basketColumn > 0) {
        basketColumn -= 1;
      }
      playerX = basketColumn * 32 + 3;
    }

    if (digitalRead(RIGHT_BUTTON_PIN) == LOW && millis_value - lastRightButtonPressTime > DEBOUNCE_DELAY) {
      keyPressTime = millis();
      millis_value = keyPressTime;
      lastRightButtonPressTime = millis_value;
      isBuzzerOn = true;
      if (basketColumn < FRUIT_COLUMN_COUNT - 1) {
        basketColumn += 1;
      }
      playerX = basketColumn * 32 + 3;
    }

    // Display player car and obstacle cars
    display.setColor(WHITE);
    display.drawXbm(playerX, playerY, Basket_width, Basket_height, Basket);

    // fruitDataPointer = &Apple;
    for (int i = 0; i < FRUIT_COLUMN_COUNT; i++) {
      if (fruitType[i] == 1) {
        display.drawXbm(fruitColumnX[i], fruitColumnY[i], Fruit_width, Fruit_height, Apple);
        isRockColumn[i] = false;
      } else if (fruitType[i] == 2) {
        display.drawXbm(fruitColumnX[i], fruitColumnY[i], Fruit_width, Fruit_height, Banana);
        isRockColumn[i] = false;
      } else if (fruitType[i] == 3) {
        display.drawXbm(fruitColumnX[i], fruitColumnY[i], Fruit_width, Fruit_height, PineApple);
        isRockColumn[i] = false;
      } else {
        display.drawXbm(fruitColumnX[i], fruitColumnY[i], Rock_width, Rock_height, Rock);
        isRockColumn[i] = true;
      }
    }

    for (int i = 0; i < FRUIT_COLUMN_COUNT; i++) {
      // Move all tube to the left
      fruitColumnY[i] += speed;
      // If a tube pass the bird, add a point
      if (!isRockColumn[i] && ((playerX <= fruitColumnX[i] && fruitColumnX[i] <= playerX + Basket_width) && ((fruitColumnY[i] <= playerY && playerY <= fruitColumnY[i] + Fruit_height) || (playerY <= fruitColumnY[i] && fruitColumnY[i] <= playerY + Basket_height))) && !hasScored[i]) {
        score++;
        hasScored[i] = true;
        fruitColumnY[i] = -16 - random(0, 4) * TUBE_DISTANCE;
        fruitType[i] = random(1, 5);
        if (fruitType[i] < 4) {
          isRockColumn[i] = false;
        } else {
          isRockColumn[i] = true;
        }
        hasScored[i] = false;
        // Increase speed every 10 tubes
        if (score % 10 == 0) {
          speed += 0.005;
        }
      }

      if (fruitColumnY[i] - Fruit_height > 64) {
        fruitColumnY[i] = -16 - (i * RACING_CAR_DISTANCE);
        hasScored[i] = false;
        fruitType[i] = random(1, 5);
        if (fruitType[i] < 4) {
          isRockColumn[i] = false;
        } else {
          isRockColumn[i] = true;
        }
      }
    }

    // The buzzer will make sound for 10 milliseconds
    if ((keyPressTime + 50) < millis()) {
      isBuzzerOn = false;
    }

    ClickSound();

    // Check collision player with other car
    for (int i = 0; i < FRUIT_COLUMN_COUNT; i++) {
      // Kiểm tra va chạm với đá
      if (isRockColumn[i] && ((playerX <= fruitColumnX[i] && fruitColumnX[i] <= playerX + Basket_width) && ((fruitColumnY[i] <= playerY && playerY <= fruitColumnY[i] + Rock_height) || (playerY <= fruitColumnY[i] && fruitColumnY[i] <= playerY + Basket_height)))) {
        EndingSound();

        if (score > highScore) {
          highScore = score;

          // Write new high score to flash memory
          preferences.begin(gameName, false);
          preferences.putUInt("highScore", highScore);
          preferences.end();
        }

        gameState = 2;

        delay(50);
      }
    }

    // Display boundary
    display.drawRect(0, 0, 128, 64);
  }

  else if (gameState == 2) {
    page = END_GAME_PAGE;
  }
}

// Game Khủng long chạy bộ
void DinosaurRun() {
  // Màn hình bắt đầu
  if (gameState == 0) {
    playerX = 20.00;
    playerY = 26.00;
    score = 0;
    speed = 0.02;
    dinoTopDelay = 100;

    display.setFont(ArialMT_Plain_16);
    display.drawString(0, 4, "Dinosaur Run");

    display.drawXbm(playerX, playerY, TRex_width, TRex_height, TRex);

    display.drawXbm(85, playerY, Cactus3Tall_width, Cactus3Tall_height, Cactus3Tall);
    display.drawXbm(0, 44, Ground_width, Ground_height, Ground_2);

    display.setFont(ArialMT_Plain_10);
    display.drawString(10, 50, "Start");
    display.drawString(90, 50, "Exit");

    cactusDisplay[0] = random(0, 4);
    cactusX[0] = 128 + CACTUS_DISTANCE;
    hasScored[0] = false;

    for (int i = 1; i < 3; i++) {
      cactusDisplay[i] = random(0, 4);
      cactusX[i] = cactusX[i - 1] + cactusWidth[cactusDisplay[i - 1]] + CACTUS_DISTANCE * random(1, 3);
      hasScored[i] = false;
    }

    groundX[0] = 0;
    groundX[1] = 200;

    if (digitalRead(LEFT_BUTTON_PIN) == LOW && millis_value - lastLeftButtonPressTime > DEBOUNCE_DELAY) {
      lastLeftButtonPressTime = millis_value;
      gameState = 1;
      playerX = 10;
      playerY = 38;
      speedUp = 0.1;
      speedDown = 0.025;
      delay(100);
    }

    if (digitalRead(RIGHT_BUTTON_PIN) == LOW && millis_value - lastRightButtonPressTime > DEBOUNCE_DELAY) {
      lastRightButtonPressTime = millis_value;
      page = MENU_PAGE;
      gameState = 0;
      delay(100);
    }
  }

  // Màn hình chơi game
  else if (gameState == 1) {
    display.setFont(ArialMT_Plain_10);
    display.drawString(3, 0, String(score));
    display.drawRect(0, 0, 128, 64);

    // display.drawXbm(playerX, playerY, TRex_width, TRex_height, TRex);
    if (!isJumping && !isFalling) {
      if (dinoState) {
        display.drawXbm(playerX, playerY, TRex_width, TRex_height, TRex1);
      } else {
        display.drawXbm(playerX, playerY, TRex_width, TRex_height, TRex2);
      }
      if (stateTime == 0) {
        stateTime = millis();
      }
    } else {
      display.drawXbm(playerX, playerY, TRex_width, TRex_height, TRex);
    }

    if (millis() - 200 > stateTime) {
      dinoState = !dinoState;
      stateTime = 0;
    }

    for (int i = 0; i < 2; i++) {
      if (i == 0) {
        display.drawXbm(groundX[i], 56, Ground_width, Ground_height, Ground_1);
      } else {
        display.drawXbm(groundX[i], 56, Ground_width, Ground_height, Ground_2);
      }

      groundX[i] -= speed;
      if (groundX[i] + 200 < 0) {
        groundX[i] = groundX[(i + 1) % 2] + 200;
      }
    }

    for (int i = 0; i < 3; i++) {
      display.drawXbm(cactusX[i], 58 - cactusHeight[cactusDisplay[i]], cactusWidth[cactusDisplay[i]], cactusHeight[cactusDisplay[i]], CactusArray[cactusDisplay[i]]);
    }

    if (digitalRead(LEFT_BUTTON_PIN) == LOW && !isJumping && !isFalling && millis_value - lastLeftButtonPressTime > DEBOUNCE_DELAY) {
      keyPressTime = millis();
      millis_value = keyPressTime;
      lastLeftButtonPressTime = millis_value;
      isBuzzerOn = true;
      isJumping = true;
    }

    ClickSound();

    if ((keyPressTime + 50) < millis()) {
      isBuzzerOn = false;
    }

    if (isJumping) {
      if (playerY > (6 + speedUp)) {
        playerY -= speedUp;
      } else {
        playerY = 6;
        isJumping = false;
        isFalling = true;
        dinoTopTime = millis();
      }
    } else if (isFalling) {
      if ((dinoTopTime + dinoTopDelay) < millis()) {
        if (playerY < (38 - speedDown)) {
          playerY += speedDown;
        } else {
          playerY = 38;
          isFalling = false;
        }
      }
    }

    if (26 <= playerY && playerY < 38) {
      speedUp = speedUpArray[3];
      speedDown = speedDownArray[3];
    } else if (16 <= playerY && playerY < 26) {
      speedUp = speedUpArray[2];
      speedDown = speedDownArray[2];
    } else if (9 <= playerY && playerY < 16) {
      speedUp = speedUpArray[1];
      speedDown = speedDownArray[1];
    } else {
      speedUp = speedUpArray[0];
      speedDown = speedDownArray[0];
    }

    for (int i = 0; i < 3; i++) {
      cactusX[i] -= speed;
      if (cactusX[i] + cactusWidth[cactusDisplay[i]] < 0) {
        hasScored[i] = false;
        cactusDisplay[i] = random(0, 4);
        cactusX[i] = cactusX[(i + 2) % 3] + cactusWidth[cactusDisplay[(i + 2) % 3]] + CACTUS_DISTANCE * random(1, 3);
      }

      if ((cactusX[i] + cactusWidth[cactusDisplay[i]]) < playerX && !hasScored[i]) {
        score++;
        hasScored[i] = true;

        if (score > highScore) {
          highScore = score;
          preferences.begin(gameName, false);
          preferences.putUInt("highScore", highScore);
          preferences.end();
        }

        if (score % 10 == 0) {
          speed += 0.01;
          dinoTopDelay -= 10;
          for (int i = 0; i < 4; i++) {
            speedUpArray[i] += 0.01;
            speedDownArray[i] += 0.01;
          }
        }
      }

      if ((cactusX[i] <= playerX && playerX <= (cactusX[i] + cactusWidth[cactusDisplay[i]] - 4) && (58 - cactusHeight[cactusDisplay[i]] + 4) < (playerY + TRex_height - 7))
          || (playerX <= cactusX[i] && cactusX[i] <= (playerX + TRex_width - 1) && ((58 - cactusHeight[cactusDisplay[i]] + 4) < (playerY + TRex_height - 14) || (TRex_height > cactusHeight[cactusDisplay[i]] && playerY == 38)))) {
        EndingSound();
        gameState = 2;
      }
    }
  }

  // Màn hình kết thúc
  else if (gameState == 2) {
    page = END_GAME_PAGE;
  }
}

// Âm thanh ấn nút
void ClickSound() {
  if (isBuzzerOn) {
    digitalWrite(BUZZER_PIN, 1);
  } else {
    digitalWrite(BUZZER_PIN, 0);
  }
}

// Âm thanh kết thúc
void EndingSound() {
  digitalWrite(BUZZER_PIN, 1);
  delay(200);
  digitalWrite(BUZZER_PIN, 0);
  delay(50);
  digitalWrite(BUZZER_PIN, 1);
  delay(50);
  digitalWrite(BUZZER_PIN, 0);
  delay(50);
  digitalWrite(BUZZER_PIN, 1);
  delay(50);
  digitalWrite(BUZZER_PIN, 0);
}