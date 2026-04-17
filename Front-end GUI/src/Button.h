#pragma once
#include "ofMain.h"

class Button {

	int x, y;//position variables
	int cornerRadius = 0;//set cornerRadius to 0 by default
	float width, height; //size variables
	float labelOffsetX = 0;//offset x
	float labelOffsetY = 0;	//offset y
	ofColor colour; //variable to store button colour
	ofColor textColour = ofColor(255);    // text colour, defaults to white
	ofColor borderColour = ofColor(255);  // border colour, defaults to white
	string imgPath, label; //string store image file path and button text
	bool visible;//determines if button is visible or not
	ofRectangle btn;//rectangle to wrap button

	ofImage* imgPtr;//image pointer
	ofTrueTypeFont* font;//font pointer
	enum class Image { NONE, CUSTOM, DEFAULT }; //possible image types
	Image imgType;//determines type of image button has

	void mousePressed(ofMouseEventArgs& mouse); //mouse pressed function
	void setCommonValues(string label, int x, int y, ofTrueTypeFont* fnt);//sets common values used on all buttons
	void draw(ofEventArgs&);

public:

	static ofEvent<string> buttonEvent;//static event variable used to notify app if buttons are pressed
	ofRectangle getRect() const { return btn; }

	Button();//constructor
	~Button();//destructor

	void create(string label, int x, int y, ofTrueTypeFont* fnt, ofImage* img);//create default image buttons
	void create(string label, int x, int y, ofTrueTypeFont* fnt, string imgpath);//create custom image buttons
	void create(string label, int x, int y, float width, float height, int hex, ofTrueTypeFont* fnt);//create standard rectangle buttons with width and height
	void setLabel(const string& newLabel);
	void setLabelOffset(int x, int y);
	void setCornerRadius(int radius);
	void setTextColour(ofColor color);
	void setBorderColour(ofColor color);
	void toggle(bool state);
};