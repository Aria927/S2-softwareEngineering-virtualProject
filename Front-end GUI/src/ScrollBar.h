#pragma once
#include "ofMain.h"

class ScrollBar {
public:
    void setup(float x, float y, float width, float height);
    void draw(float contentHeight, float viewHeight, float scrollOffset);
    float getDragDelta(int mouseX, int mouseY, float contentHeight, float viewHeight);

    bool isDragging;
    float dragStartY;
    float dragStartOffset;
    ofRectangle track;
    ofRectangle thumb;

    void mousePressed(int x, int y, float contentHeight, float viewHeight, float scrollOffset);
    void mouseReleased();
    float mouseDragged(int x, int y, float contentHeight, float viewHeight);
};
