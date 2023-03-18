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
    void setRenderComplete() {
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
    void setTimings(unsigned int newFrameDuration, unsigned int newFrameCount) {
        this->frameDuration = newFrameDuration;
        this->frameCount = newFrameCount;
    }
};

struct RGBColor {
    unsigned char red;
    unsigned char green;
    unsigned char blue;
    uint16_t to565() const {
        return ((uint16_t)(red & 0xF8) << 8) | ((uint16_t)(green & 0xFC) << 3) | (blue >> 3);
    }
};
RGBColor operator* (const RGBColor lhs, const float rhs)  {
    return RGBColor{
            (unsigned char) (lhs.red*rhs),
            (unsigned char) (lhs.green*rhs),
            (unsigned char) (lhs.blue*rhs)
    };
}
RGBColor operator* (const float lhs, const RGBColor rhs)  {
    return RGBColor{
            (unsigned char) (rhs.red*lhs),
            (unsigned char) (rhs.green*lhs),
            (unsigned char) (rhs.blue*lhs)
    };
}

// Which pin on the Arduino is connected to the NeoPixels?

Adafruit_NeoMatrix matrix = Adafruit_NeoMatrix(8, 8, 2,
    NEO_MATRIX_TOP + NEO_MATRIX_RIGHT +
    NEO_MATRIX_COLUMNS + NEO_MATRIX_PROGRESSIVE,
    NEO_GRB + NEO_KHZ800);

#define STRIP_LENGTH 60

Adafruit_NeoPixel strip = Adafruit_NeoPixel(STRIP_LENGTH, 3, NEO_GRB + NEO_KHZ800);

void setup() {
    matrix.begin();
    matrix.setTextWrap(false);
    matrix.setBrightness(255);
    matrix.setTextColor(matrix.Color(255, 255, 255));

    matrix.clear();
    matrix.show();

    strip.begin();
    strip.setBrightness(255);

    strip.clear();
    strip.show();

    Serial.begin(9600);
    Serial.println("Arduino Connected");
}

AnimationController matrixController = AnimationController(0, 0);
AnimationController stripController = AnimationController(0, 0);

void circleTransition(RGBColor color) {
    matrixController.setTimings(40, 11);
    if (matrixController.shouldRender()) {
        matrix.fillCircle(0, 0, matrixController.currentFrame(), color.to565());
        matrix.show();
        matrixController.setRenderComplete();
    }
}
void snakeTransition(RGBColor color) {
    matrixController.setTimings(10, 64);
    if (matrixController.shouldRender()) {
        uint16_t color565 = color.to565();
        int x = 3;
        int y = 3;
        int counter = 0;
        int size = 1;
        int direction = 0;
        matrix.drawPixel(x, y, color565);
        matrix.show();
        for (unsigned int i=0; i < matrixController.currentFrame(); i++) {
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
            matrix.drawPixel(x, y, color565);
        }
        matrix.show();
        matrixController.setRenderComplete();
    }
}

void triangleTransition(RGBColor color) {
    matrixController.setTimings(100, 4);
    if (matrixController.shouldRender()) {
        int i = matrixController.currentFrame();
        uint16_t color565 = color.to565();

        matrix.drawLine(3, i, 3-i, 0, color565);
        matrix.drawLine(4, i, 4+i, 0, color565);

        matrix.drawLine(i, 3, 0, 3-i, color565);
        matrix.drawLine(i, 4, 0, 4+i, color565);

        matrix.drawLine(3, 7-i, 3-i, 7, color565);
        matrix.drawLine(4, 7-i, 4+i, 7, color565);

        matrix.drawLine(7-i, 3, 7, 3-i, color565);
        matrix.drawLine(7-i, 4, 7, 4+i, color565);

        matrix.show();
        matrixController.setRenderComplete();
    }
}

void interlaceTransition(RGBColor color) {
    matrixController.setTimings(30, 8);
    if (matrixController.shouldRender()) {
        int i = matrixController.currentFrame();

        for (int j=0; j<8; j++) {
            int y = (j%2==0) ? i : 7-i;
            matrix.drawPixel(j, y, color.to565());
        }

        matrix.show();
        matrixController.setRenderComplete();
    }
}

bool matrixDone[8][8];
void matrixSparkleTransition(RGBColor color) {
    matrixController.setTimings(8, 64);
    if (matrixController.shouldRender()) {
        int i = matrixController.currentFrame();
        if (i == 0) {
            for (auto &j : matrixDone) {
                for (bool &k : j) {
                    k = false;
                }
            }
        }
        int value = random(64-i);
        for (int j=0; j<8; j++) {
            for (int k=0; k<8; k++) {
                if (!matrixDone[j][k]) {
                    if (value == 0) {
                        matrixDone[j][k] = true;
                        matrix.drawPixel(j, k, color.to565());
                    }
                    value--;
                }
            }
        }
        matrix.show();
        matrixController.setRenderComplete();
    }
}

bool stripDone[STRIP_LENGTH];
void stripSparkleTransition(RGBColor color) {
    stripController.setTimings(8, 60);
    if (stripController.shouldRender()) {
        unsigned int i = stripController.currentFrame();
        if (i==0) {
            for (auto &j : stripDone) {
                j = false;
            }
        }
        int value = random(STRIP_LENGTH-i);
        for (int j=0; j<STRIP_LENGTH; j++) {
            if (!stripDone[j]) {
                if (value == 0) {
                    stripDone[j] = true;
                    strip.setPixelColor(j, color.red, color.green, color.blue);
                }
                value--;
            }
        }
        strip.show();
        stripController.setRenderComplete();
    }
}

void stripFillTransition(RGBColor color) {
    stripController.setTimings(8, 60);
    if (stripController.shouldRender()) {
        unsigned int i = stripController.currentFrame();
        strip.setPixelColor(i, color.red, color.green, color.blue);
        strip.show();
        stripController.setRenderComplete();
    }
}

void fadeTransition(RGBColor color) {
    matrixController.setTimings(10, 25);
    float progress = (float) matrixController.currentFrame() / matrixController.frameCount;
    matrix.fillScreen((progress * color).to565());
    matrix.show();
}

void blockTransition(unsigned long duration) {
    matrixController.setTimings(duration, 1);
    if (matrixController.shouldRender()) {
        matrixController.setRenderComplete();
    }
}

int matrixTransition = 0;
RGBColor matrixColor = RGBColor{};
void matrixRandomTransition(RGBColor newColor) {
    if (matrixController.shouldRender()) {
        if (matrixController.currentFrame() == 0) {
            matrixTransition = (int) random(5);
            matrixColor = newColor;
        }
    }

    if (matrixTransition == 0) {
        circleTransition(matrixColor);
    } else if (matrixTransition == 1) {
        snakeTransition(matrixColor);
    } else if (matrixTransition == 2) {
        triangleTransition(matrixColor);
    } else if (matrixTransition == 3) {
        interlaceTransition(matrixColor);
    } else if (matrixTransition == 4) {
        matrixSparkleTransition(matrixColor);
    }
}

int stripTransition = 0;
RGBColor stripColor = RGBColor{};
void stripRandomTransition(RGBColor newColor) {
    if (stripController.shouldRender()) {
        if (stripController.currentFrame() == 0) {
            stripTransition = (int) random(2);
            stripColor = newColor;
        }
    }

    if (stripTransition == 0) {
        stripSparkleTransition(stripColor);
    } else if (stripTransition == 1) {
        stripFillTransition(stripColor);
    }
}

enum class MatrixState {
    OFF,
    RED,
    BLUE,
    CONE,
    CUBE,
    MOVING
};

enum class BlinkState {
    TRANSITION,
    ON,
    FADE_OFF,
    OFF,
    FADE_ON
};

MatrixState state = MatrixState::OFF;
MatrixState newState = state;

BlinkState blinkState = BlinkState::TRANSITION;

RGBColor coneColor = RGBColor{255, 210, 0};
RGBColor cubeColor = RGBColor{150, 25, 255};
RGBColor movingColor = RGBColor{255, 255, 200};

void loop() {
    if (Serial.available() > 0) {
        switch (Serial.read()) {
            case 0x30:
                newState = MatrixState::OFF;
                Serial.println("Got new state: OFF");
                break;
            case 0x31:
                newState = MatrixState::RED;
                Serial.println("Got new state: RED");
                break;
            case 0x32:
                newState = MatrixState::BLUE;
                Serial.println("Got new state: BLUE");
                break;
            case 0x33:
                newState = MatrixState::CONE;
                Serial.println("Got new state: CONE");
                break;
            case 0x34:
                newState = MatrixState::CUBE;
                Serial.println("Got new state: CUBE");
                break;
            case 0x35:
                newState = MatrixState::MOVING;
                Serial.println("Got new state: MOVING");
            default:
                Serial.println("Unknown input");
                break;
        }
    }

    if (matrixController.isDone() && stripController.isDone() && state != newState) {
        Serial.println("Starting animation");
        matrixController.reset();
        stripController.reset();
        state = newState;
    }

    if (state == MatrixState::OFF) {
        stripRandomTransition(RGBColor{0, 0, 0});
        matrixRandomTransition(RGBColor{0, 0, 0});
    } else if (state == MatrixState::RED) {
        if (matrixController.isDone() && state == newState) {
            matrixController.reset();
        }
        if (stripController.isDone() && state == newState) {
            stripController.reset();
        }
        RGBColor redColor = RGBColor{
                (unsigned char) random(150, 255),
                (unsigned char) random(100),
                (unsigned char) random(50)
        };
        stripRandomTransition(redColor);
        matrixRandomTransition(redColor);
    } else if (state == MatrixState::BLUE) {
        if (matrixController.isDone() && state == newState) {
            matrixController.reset();
        }
        if (stripController.isDone() && state == newState) {
            stripController.reset();
        }
        RGBColor blueColor = RGBColor{
                (unsigned char) random(50),
                (unsigned char) random(125),
                (unsigned char) random(100, 255)
        };
        stripRandomTransition(blueColor);
        matrixRandomTransition(blueColor);
    } else if (state == MatrixState::CONE) {
        stripRandomTransition(coneColor);
        matrixRandomTransition(coneColor);
//        delay(400);
//        matrix.clear();
//        matrix.show();
//        delay(100);
//        matrix.fillScreen(coneColor);
//        matrix.show();
    } else if (state == MatrixState::CUBE) {
        stripRandomTransition(cubeColor);
        matrixRandomTransition(cubeColor);
//        delay(400);
//        matrix.clear();
//        matrix.show();
//        delay(100);
//        matrix.fillScreen(cubeColor);
//        matrix.show();
    } else if (state == MatrixState::MOVING) {
        stripRandomTransition(movingColor);
        matrixRandomTransition(movingColor);
    }
}
