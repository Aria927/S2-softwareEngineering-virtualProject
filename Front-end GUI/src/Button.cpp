#include "Button.h"

ofEvent<string> Button::buttonEvent;

Button::Button() {
	imgType = Image::NONE;
	visible = true;
	ofAddListener(ofEvents().mousePressed, this, &Button::mousePressed);
	ofAddListener(ofEvents().draw, this, &Button::draw);
}

//--------------------------------------------------------------

void Button::create(string label, int x, int y, ofTrueTypeFont* fnt, ofImage* img) {
	setCommonValues(label, x, y, fnt); // send label, x and y to setting function

	imgType = Image::DEFAULT;//set image type
	imgPtr = img;// assign img pointer to address passed in 
	height = imgPtr->getHeight();//calculate height based on image height
	width = imgPtr->getWidth();//calculate width based on image width

	btn.set(x, y, width, height);
}

//--------------------------------------------------------------

void Button::create(string label, int x, int y, ofTrueTypeFont* fnt, string imgPath) {
	setCommonValues(label, x, y, fnt); // send label, x and y to setting function

	this->imgPath = imgPath;// set image path
	imgType = Image::CUSTOM;// set image path
	imgPtr = new ofImage;//allocate memory for image object
	imgPtr->load(imgPath);//load image with imgPath

	height = imgPtr->getHeight();//calculate height based on image height
	width = imgPtr->getWidth();//calculate width based on image width

	btn.set(x, y, width, height);
}

//--------------------------------------------------------------

void Button::create(string label, int x, int y, float width, float height, int hex, ofTrueTypeFont* fnt) {
	setCommonValues(label, x, y, fnt); // send label, x and y to setting function

	this->width = width;//set width
	this->height = height;//set height

	colour.setHex(hex);//set colour

	btn.set(x, y, width, height);
}

//--------------------------------------------------------------

void Button::setCommonValues(string label, int x, int y, ofTrueTypeFont* fnt) {
	this->label = label; //set label
	this->x = x; //set x
	this->y = y;//set y 
	font = fnt;//passed font resource
}

//--------------------------------------------------------------

void Button::mousePressed(ofMouseEventArgs& mouse) {
	if (btn.inside(mouse.x, mouse.y)) {//if mouse click in button
		ofNotifyEvent(buttonEvent, label);//notify app button event has occurred and send label
	}
}

//--------------------------------------------------------------

void Button::draw(ofEventArgs&) {
	if (visible) {
		if (imgType != Image::NONE) {//if button is an image button
			ofSetColor(colour);//avoids tints on image
			imgPtr->draw(btn);//draw the button
		}
		else {
			ofSetColor(colour);// set colour
			ofDrawRectRounded(btn, cornerRadius);// draw rectRounded using btn coords, size and cornerRadius

			ofNoFill();
			ofSetLineWidth(1.5);
			ofSetColor(borderColour); // border colour
			ofDrawRectRounded(btn, cornerRadius);
			ofFill();
		}

		ofSetColor(textColour); // text colour
		float textWidth = font->stringWidth(label);
		float textHeight = font->stringHeight(label);
		float textX = btn.x + (btn.width - textWidth) / 2 + labelOffsetX; //set x position of labels
		float textY = btn.y + (btn.height + textHeight) / 2 + labelOffsetY; //set y position of labels
		font->drawString(label, textX, textY);
	}
}

//--------------------------------------------------------------

void Button::toggle(bool state) {
	if (!state) {//if visible make invisible and remove listeners
		visible = false;
		ofRemoveListener(ofEvents().mousePressed, this, &Button::mousePressed);
		ofRemoveListener(ofEvents().draw, this, &Button::draw);
	}
	else if (state) {//if invisible make visible and add listeners
		visible = true;
		ofAddListener(ofEvents().mousePressed, this, &Button::mousePressed);
		ofAddListener(ofEvents().draw, this, &Button::draw);
	}
}

//--------------------------------------------------------------

void Button::setLabel(const string& newLabel) {
	label = newLabel;
}

//--------------------------------------------------------------

void Button::setLabelOffset(int x, int y) { //align labels that don't sit correctly 
	labelOffsetX = x;
	labelOffsetY = y;
}

//--------------------------------------------------------------

void Button::setCornerRadius(int radius) { //set corner radius for buttons
	cornerRadius = radius;
}

//--------------------------------------------------------------

void Button::setTextColour(ofColor color) { // set text colour
	textColour = color;
}

//--------------------------------------------------------------

void Button::setBorderColour(ofColor color) { // set border colour
	borderColour = color;
}

//--------------------------------------------------------------

Button::~Button() //destructor
{
	ofRemoveListener(ofEvents().mousePressed, this, &Button::mousePressed);
	ofRemoveListener(ofEvents().draw, this, &Button::draw);

	if (imgType == Image::CUSTOM) {
		delete imgPtr;
	}
}