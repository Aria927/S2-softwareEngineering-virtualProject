#pragma once
#include "ofMain.h"
#include "Button.h"
#include "PasswordEntry.h"
#include "ScrollBar.h"

class ofApp : public ofBaseApp {
public:
    void setup();
    void update();
    void draw();
    void keyPressed(int key);
    void mousePressed(int x, int y, int button);
    void mouseScrolled(int x, int y, float scrollX, float scrollY);
    void buttonEvent(string& label);

    // login / register screen
    string Email, Password, RePassword;
    bool emailInput, passwordInput, rePasswordInput;
    bool createAccountScreen;

    ofRectangle loginScreenBG, emailBox, passwordBox, rePasswordBox;
    ofTrueTypeFont headerXLFont, headerLargeFont, headerMedFont, smallFont, standardFont;

    Button loginBtn, createBtn, loginAccountBtn, registerBtn;

    // main screen
    bool mainScreen;

    // search
    string searchString;
    bool searchInput;
    ofRectangle searchBox;

    // add new popup
    bool popupOpen;
    string popupName, popupUser, popupPass;
    bool popupNameInput, popupUserInput, popupPassInput;
    ofRectangle popupBG, popupNameBox, popupUserBox, popupPassBox;
    Button popupConfirmBtn, popupCancelBtn;

    // password entries
    vector<PasswordEntry> entries;
    vector<PasswordEntry> filteredEntries;

    // scrolling
    float scrollOffset;
    float rowHeight;
    ScrollBar scrollBar;

    // per-row state (indexed by filteredEntries position)
    vector<bool> passwordVisible;
    int editingRow; // -1 if none
    string editName, editUser, editPass;
    bool editNameInput, editUserInput, editPassInput;
    ofRectangle editNameBox, editUserBox, editPassBox;

    // header / layout rects
    ofRectangle topBar, headerBar;
    ofRectangle addNewBox;

    // helpers
    void drawMainScreen();
    void drawLoginScreen();
    void updateFilter();
};