#include "ScrollBar.h"

#include <algorithm>

//--------------------------------------------------------------
void ScrollBar::setup(float x, float y, float width, float height) {
    track.set(x, y, width, height);
    isDragging = false;
    dragStartY = 0;
    dragStartOffset = 0;
}

//--------------------------------------------------------------
void ScrollBar::draw(float contentHeight, float viewHeight, float scrollOffset) {
    // track
    ofSetColor(220, 220, 220);
    ofDrawRectangle(track);

    if (contentHeight <= viewHeight) return; // no scrollbar needed

    float thumbHeight = (viewHeight / contentHeight) * track.height;
    float thumbY = track.y + (scrollOffset / (contentHeight - viewHeight)) * (track.height - thumbHeight);

    thumb.set(track.x, thumbY, track.width, thumbHeight);

    ofSetColor(180, 180, 180);
    ofDrawRectRounded(thumb, 4);
}

//--------------------------------------------------------------
void ScrollBar::mousePressed(int x, int y, float contentHeight, float viewHeight, float scrollOffset) {
    float thumbHeight = (viewHeight / contentHeight) * track.height;
    float thumbY = track.y + (scrollOffset / (contentHeight - viewHeight)) * (track.height - thumbHeight);
    thumb.set(track.x, thumbY, track.width, thumbHeight);

    if (thumb.inside(x, y)) {
        isDragging = true;
        dragStartY = y;
        dragStartOffset = scrollOffset;
    }
}

//--------------------------------------------------------------
void ScrollBar::mouseReleased() {
    isDragging = false;
}

//--------------------------------------------------------------
float ScrollBar::mouseDragged(int x, int y, float contentHeight, float viewHeight) {
    if (!isDragging) return -1;

    float thumbHeight = (viewHeight / contentHeight) * track.height;
    float dragRange = track.height - thumbHeight;
    float delta = y - dragStartY;
    float newOffset = dragStartOffset + (delta / dragRange) * (contentHeight - viewHeight);

    newOffset = std::max(0.0f, std::min(newOffset, contentHeight - viewHeight));
    return newOffset;
}
