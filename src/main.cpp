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

void circleTransition(RGBColor color) {
    animationController.setTimings(40, 11);
    if (animationController.shouldRender()) {
        matrix.fillCircle(0, 0, animationController.currentFrame(), color.to565());
        matrix.show();
        animationController.setRenderComplete();
    }
}
void snakeTransition(RGBColor color) {
    animationController.setTimings(10, 64);
    if (animationController.shouldRender()) {
        uint16_t color565 = color.to565();
        int x = 3;
        int y = 3;
        int counter = 0;
        int size = 1;
        int direction = 0;
        matrix.drawPixel(x, y, color565);
        matrix.show();
        for (unsigned int i=0; i<animationController.currentFrame(); i++) {
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
        animationController.setRenderComplete();
    }
}

void triangleTransition(RGBColor color) {
    animationController.setTimings(100, 4);
    if (animationController.shouldRender()) {
        int i = animationController.currentFrame();
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
        animationController.setRenderComplete();
    }
}

void interlaceTransition(RGBColor color) {
    animationController.setTimings(30, 8);
    if (animationController.shouldRender()) {
        int i = animationController.currentFrame();

        for (int j=0; j<8; j++) {
            int y = (j%2==0) ? i : 7-i;
            matrix.drawPixel(j, y, color.to565());
        }

        matrix.show();
        animationController.setRenderComplete();
    }
}

bool done[8][8];
void sparkleTransition(RGBColor color) {
    animationController.setTimings(8, 64);
    if (animationController.shouldRender()) {
        int i = animationController.currentFrame();
        if (i == 0) {
            for (auto &j : done) {
                for (bool &k : j) {
                    k = false;
                }
            }
        }
        int value = random(64-i);
        for (int j=0; j<8; j++) {
            for (int k=0; k<8; k++) {
                if (!done[j][k]) {
                    if (value == 0) {
                        done[j][k] = true;
                        matrix.drawPixel(j, k, color.to565());
                    }
                    value--;
                }
            }
        }
        matrix.show();
        animationController.setRenderComplete();
    }
}

void fadeTransition(RGBColor color) {
    animationController.setTimings(10, 25);
    float progress = (float) animationController.currentFrame() / animationController.frameCount;
    matrix.fillScreen((progress * color).to565());
    matrix.show();
}

void blockTransition(unsigned long duration) {
    animationController.setTimings(duration, 1);
    if (animationController.shouldRender()) {
        animationController.setRenderComplete();
    }
}

int transition = 0;
void randomTransition(RGBColor color) {
    if (animationController.shouldRender()) {
        if (animationController.currentFrame() == 0) {
            transition = (int) random(5);
        }

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
}

enum MatrixState {
    OFF,
    RED,
    BLUE,
    CONE,
    CUBE,
    MOVING
};

MatrixState state = OFF;
MatrixState newState = state;

RGBColor coneColor = RGBColor{255, 210, 0};
RGBColor cubeColor = RGBColor{150, 25, 255};
RGBColor movingColor = RGBColor{255, 255, 200};

void loop() {
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

    if (animationController.isDone() && state != newState) {
        animationController.reset();
        state = newState;
    }

    if (state == OFF) {
        randomTransition(RGBColor{0, 0, 0});
//        matrix.clear();
//        matrix.show();
    } else if (state == RED) {
        randomTransition(RGBColor{
            (unsigned char) random(150, 255),
            (unsigned char) random(100),
            (unsigned char) random(50)
        });
//        matrix.fillScreen(matrix.Color(random(150, 255), random(100), random(50)));
//        matrix.show();
    } else if (state == BLUE) {
        randomTransition(RGBColor{
            (unsigned char) random(50),
            (unsigned char) random(125),
            (unsigned char) random(100, 255)
        });
//        matrix.fillScreen(matrix.Color(random(50), random(125), random(100, 255)));
//        matrix.show();
    } else if (state == CONE) {
        randomTransition(coneColor);
//        delay(400);
//        matrix.clear();
//        matrix.show();
//        delay(100);
//        matrix.fillScreen(coneColor);
//        matrix.show();
    } else if (state == CUBE) {
        randomTransition(cubeColor);
//        delay(400);
//        matrix.clear();
//        matrix.show();
//        delay(100);
//        matrix.fillScreen(cubeColor);
//        matrix.show();
    } else if (state == MOVING) {
        randomTransition(movingColor);
//        matrix.fillScreen(movingColor);
//        matrix.show();
    }
}
