#include <Arduino.h>
#include <Adafruit_NeoMatrix.h>
#include <Adafruit_NeoPixel.h>

//TODO refactor to create animation interface
class AnimationController {
private:
    unsigned int _currentFrame = 0;
    unsigned long _lastFrameTime = 0;
public:
    unsigned int frameDuration;
    unsigned int frameCount;
    unsigned int currentFrame() const { return _currentFrame; }
    bool isDone() const { return _currentFrame >= frameCount; }
    unsigned int duration() const { return frameDuration*frameCount; }
    bool shouldRender() const { return !isDone() && millis() - _lastFrameTime >= frameDuration; }
    void setRendered() {
        _lastFrameTime = millis();
        _currentFrame++;
    }
    void reset() {
        _currentFrame = 0;
        _lastFrameTime = 0;
    }
    AnimationController(unsigned int frameDuration, unsigned int frameCount) {
        this->frameDuration = frameDuration;
        this->frameCount = frameCount;
    }
    static AnimationController fromDuration(unsigned int duration, unsigned int frameCount) {
        return {duration/frameCount, frameCount};
    }
};

// Which pin on the Arduino is connected to the NeoPixels?
#define PIN 2 // On Trinket or Gemma, suggest changing this to 1

Adafruit_NeoMatrix matrix = Adafruit_NeoMatrix(8, 8, PIN,
    NEO_MATRIX_TOP     + NEO_MATRIX_RIGHT +
    NEO_MATRIX_COLUMNS + NEO_MATRIX_PROGRESSIVE,
    NEO_GRB            + NEO_KHZ800);

void setup() {
    matrix.begin();
    matrix.setTextWrap(false);
    matrix.setBrightness(255);
    matrix.setTextColor(matrix.Color(255, 255, 255));

    matrix.clear();
    matrix.show();

    Serial.begin(9600);
    Serial.println("Arduino Connected");
}

AnimationController animationController = AnimationController(0, 0);

void circleTransition(uint16_t color) {
    animationController.frameDuration = 40;
    animationController.frameCount = 11;
    if (animationController.shouldRender())
    for (int i=0; i<11; i++) {
        matrix.fillCircle(0, 0, animationController.currentFrame(), color);
        matrix.show();
        delay(40);
    }
}
void snakeTransition(uint16_t color) {
    int x = 3;
    int y = 3;
    int counter = 0;
    int size = 1;
    int direction = 0;
    matrix.drawPixel(x, y, color);
    matrix.show();
    for (int i=0; i<64; i++) {
        delay(10);
        if (direction == 0) {
            x++;
            counter++;
            if (counter >= size) {
                direction++;
                counter = 0;
            }
        } else if (direction == 1) {
            y++;
            counter++;
            if (counter >= size) {
                direction++;
                counter = 0;
                size++;
            }
        } else if (direction == 2) {
            x--;
            counter++;
            if (counter >= size) {
                direction++;
                counter = 0;
            }
        } else if (direction == 3) {
            y--;
            counter++;
            if (counter >= size) {
                direction = 0;
                counter = 0;
                size++;
            }
        }
        matrix.drawPixel(x, y, color);
        matrix.show();
    }
}

void triangleTransition(uint16_t color) {
    for (int i=0; i<4; i++) {
        matrix.drawLine(3, i, 3-i, 0, color);
        matrix.drawLine(4, i, 4+i, 0, color);

        matrix.drawLine(i, 3, 0, 3-i, color);
        matrix.drawLine(i, 4, 0, 4+i, color);

        matrix.drawLine(3, 7-i, 3-i, 7, color);
        matrix.drawLine(4, 7-i, 4+i, 7, color);

        matrix.drawLine(7-i, 3, 7, 3-i, color);
        matrix.drawLine(7-i, 4, 7, 4+i, color);

        matrix.show();
        delay(100);
    }
}

void interlaceTransition(uint16_t color) {
    for (int i=0; i<8; i++) {
        for (int j=0; j<8; j++) {
            int y = (j%2==0) ? i : 7-i;
            matrix.drawPixel(j, y, color);
            matrix.show();
        }
        delay(30);
    }
}

void sparkleTransition(uint16_t color) {
    bool done[8][8];
    for (auto & i : done) {
        for (bool & j : i) {
            j = false;
        }
    }
    for (int i=0; i<64; i++) {
        int value = random(64-i);
        for (int j=0; j<8; j++) {
            for (int k=0; k<8; k++) {
                if (!done[j][k]) {
                    if (value == 0) {
                        done[j][k] = true;
                        matrix.drawPixel(j, k, color);
                        matrix.show();
                        delay(8);
                    }
                    value--;
                }
            }
        }
    }
}

enum MatrixState {
    OFF,
    RED,
    BLUE,
    CONE,
    CUBE,
    MOVING
};

void randomTransition(uint16_t color) {
    int transition = random(5);
    if (transition == 0) {
        circleTransition(color);
    } else if (transition == 1) {
        snakeTransition(color);
    } else if (transition == 2) {
        triangleTransition(color);
    } else if (transition == 3) {
        interlaceTransition(color);
    } else if (transition == 4) {
        sparkleTransition(color);
    }
}

MatrixState state = OFF;

uint16_t coneColor = matrix.Color(255, 210, 0);
uint16_t cubeColor = matrix.Color(150, 25, 255);
uint16_t movingColor = matrix.Color(255, 255, 200);

void loop() {
    MatrixState newState = state;
    if (Serial.available() > 0) {
        switch (Serial.read()) {
            case 0x30:
                newState = OFF;
                break;
            case 0x31:
                newState = RED;
                break;
            case 0x32:
                newState = BLUE;
                break;
            case 0x33:
                newState = CONE;
                break;
            case 0x34:
                newState = CUBE;
                break;
            case 0x35:
                newState = MOVING;
            default:
                Serial.println("Unknown input");
                break;
        }
    }

    if (newState == OFF) {
//        if (newState != state) {
//            randomTransition(0);
//        }
        matrix.clear();
        matrix.show();
    } else if (newState == RED) {
//        randomTransition(matrix.Color(random(150, 255), random(100), random(50)));
        matrix.fillScreen(matrix.Color(random(150, 255), random(100), random(50)));
        matrix.show();
    } else if (newState == BLUE) {
//        randomTransition(matrix.Color(random(50), random(125), random(100, 255)));
        matrix.fillScreen(matrix.Color(random(50), random(125), random(100, 255)));
        matrix.show();
    } else if (newState == CONE) {
//        if (newState != state) {
//            randomTransition(coneColor);
//        }
//        delay(400);
//        matrix.clear();
//        matrix.show();
//        delay(100);
        matrix.fillScreen(coneColor);
        matrix.show();
    } else if (newState == CUBE) {
//        if (newState != state) {
//            randomTransition(cubeColor);
//        }
//        delay(400);
//        matrix.clear();
//        matrix.show();
//        delay(100);
        matrix.fillScreen(cubeColor);
        matrix.show();
    } else if (newState == MOVING) {
//        if (newState != state) {
//            randomTransition(movingColor);
//        }
        matrix.fillScreen(movingColor);
        matrix.show();
    }

    state = newState;
}
