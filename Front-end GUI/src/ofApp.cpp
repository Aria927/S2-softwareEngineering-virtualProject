#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup() {
    emailInput = false;
    passwordInput = false;
    rePasswordInput = false;
    createAccountScreen = false;
    mainScreen = false;
    Email = "Email Address";
    Password = "Password";
    RePassword = "Re-enter Password";
    searchString = "Search";
    popupName = "App/Website Name";
    popupUser = "Username";
    popupPass = "Password";
    popupNotes = "Notes / General Text";

    searchInput = false;
    searchString = "Search";

    popupOpen = false;
    popupName = "App/Website Name";
    popupUser = "Username";
    popupPass = "Password";
    popupNotes = "Notes / General Text";
    popupNameInput = false;
    popupUserInput = false;
    popupPassInput = false;
    popupNotesInput = false;

    scrollOffset = 0;
    rowHeight = 80;
    editingRow = -1;
    editNameInput = false;
    editUserInput = false;
    editPassInput = false;
    editNotesInput = false;
    statusMessage = "";

    headerXLFont.load("Alfarn W05 Regular.otf", 56);
    headerLargeFont.load("Alfarn W05 Regular.otf", 50);
    headerMedFont.load("Alfarn W05 Regular.otf", 40);
    standardFont.load("Antarctican_Headline_Book.otf", 43);
    headerFont.load("Antarctican_Headline_Book.otf", 27);
    smallFont.load("Antarctican_Headline_Book.otf", 23);

    // login screen
    loginScreenBG.set(0, 0, 580, 720);
    emailBox.set(740, 151, 392, 68);
    passwordBox.set(740, 241, 392, 68);
    rePasswordBox.set(740, 331, 392, 68);

    loginBtn.create("Login", 740, 371, 392, 75, 0x1380F0, &standardFont); // blue confirm button
    loginBtn.setCornerRadius(15);

    createBtn.create("Create account", 740, 590, 392, 70, 0xFAFAFA, &standardFont);
    createBtn.setCornerRadius(15);
    createBtn.setTextColour(ofColor(80, 80, 80));
    createBtn.setBorderColour(ofColor(198, 198, 198));

    // create account screen buttons
    registerBtn.create("Register", 740, 424, 392, 70, 0x1380F0, &standardFont); // blue confirm button
    registerBtn.setCornerRadius(15);
    registerBtn.toggle(false); // hidden by default

    loginAccountBtn.create("Login account", 740, 590, 392, 70, 0xfafafa, &standardFont);
    loginAccountBtn.setCornerRadius(15);
    loginAccountBtn.setCornerRadius(15);
    loginAccountBtn.setTextColour(ofColor(80, 80, 80));
    loginAccountBtn.setBorderColour(ofColor(198, 198, 198));
    loginAccountBtn.toggle(false); // hidden by default


    // main screen layout
    topBar.set(0, 0, 1280, 90);
    headerBar.set(0, 90, 1245, 65);
    searchBox.set(20, 18, 300, 54);
    addNewBox.set(348, 18, 140, 54);

    // main screen buttons
    popupConfirmBtn.create("Add", 490, 580, 120, 45, 0x1380F0, &smallFont);
    popupConfirmBtn.setCornerRadius(8);
    popupConfirmBtn.toggle(false);
    popupCancelBtn.create("Cancel", 630, 580, 120, 45, 0xE0E0E0, &smallFont);
    popupCancelBtn.setCornerRadius(8);
    popupCancelBtn.setTextColour(ofColor(80, 80, 80));
    popupCancelBtn.setBorderColour(ofColor(198, 198, 198));
    popupCancelBtn.toggle(false);
    logoutBtn.create("Logout", 1155, 23, 90, 44, 0xFAFAFA, &smallFont);
    logoutBtn.setCornerRadius(10);
    logoutBtn.setTextColour(ofColor(80, 80, 80));
    logoutBtn.setBorderColour(ofColor(198, 198, 198));
    logoutBtn.toggle(false);

    popupBG.set(390, 75, 500, 570);
    popupNameBox.set(430, 145, 420, 50);
    popupUserBox.set(430, 210, 420, 50);
    popupPassBox.set(430, 275, 420, 50);
    popupNotesBox.set(430, 340, 420, 200);

    editPopupOpen = false;
    editingRow = -1;
    editNameInput = false;
    editUserInput = false;
    editPassInput = false;
    editNotesInput = false;

    editPopupBG.set(390, 75, 500, 570);
    editNameBox.set(430, 145, 420, 50);
    editUserBox.set(430, 210, 420, 50);
    editPassBox.set(430, 275, 420, 50);
    editNotesBox.set(430, 340, 420, 200);

    editConfirmBtn.create("Save", 490, 580, 120, 45, 0x1380F0, &smallFont);
    editConfirmBtn.setCornerRadius(8);
    editConfirmBtn.toggle(false);
    editCancelBtn.create("Discard", 630, 580, 120, 45, 0xE0E0E0, &smallFont);
    editCancelBtn.setCornerRadius(8);
    editCancelBtn.setTextColour(ofColor(80, 80, 80));
    editCancelBtn.setBorderColour(ofColor(198, 198, 198));
    editCancelBtn.toggle(false);

    scrollBar.setup(1245, 155, 35, 565);

    ofAddListener(Button::buttonEvent, this, &ofApp::buttonEvent);
}

//--------------------------------------------------------------
void ofApp::update() {
    if (emailInput == false && Email == "") {
        Email = "Email Address";
    }
    if (passwordInput == false && Password == "") {
        Password = "Password";
    }
    if (rePasswordInput == false && RePassword == "") {
        RePassword = "Re-enter Password";
    }
    if (searchInput == false && searchString == "") {
        searchString = "Search";
    }
    if (popupNameInput == false && popupName == "") {
        popupName = "App/Website Name";
    }
    if (popupUserInput == false && popupUser == "") {
        popupUser = "Username";
    }
    if (popupPassInput == false && popupPass == "") {
        popupPass = "Password";
    }
    if (popupNotesInput == false && popupNotes == "") {
        popupNotes = "Notes / General Text";
    }
    if (editNameInput == false && editName == "") {
        editName = "App/Website Name";
    }
    if (editUserInput == false && editUser == "") {
        editUser = "Username";
    }
    if (editPassInput == false && editPass == "") {
        editPass = "Password";
    }
    if (editNotesInput == false && editNotes == "") {
        editNotes = "Notes / General Text";
    }
}

//--------------------------------------------------------------
void ofApp::draw() {
    if (mainScreen == false) {
        drawLoginScreen();
    }
    else {
        drawMainScreen();
    }
}

//--------------------------------------------------------------
void ofApp::drawLoginScreen() {
    ofBackground(250, 250, 250);

    ofSetColor(19, 128, 240);
    ofDrawRectangle(loginScreenBG);

    ofSetColor(255);
    headerXLFont.drawString("Password\n Manager", 75, 290);

    ofSetColor(51, 51, 51);
    if (createAccountScreen == false) {
        headerLargeFont.drawString("Login Account", 660, 70);
    }
    else {
        headerLargeFont.drawString("Create Account", 625, 70);
    }

    // email box
    if (emailInput == false) { 
        ofSetColor(230, 230, 230); 
    }
    else { 
        ofSetColor(250, 250, 250); 
    }
    ofDrawRectRounded(emailBox, 10);

    // password box
    if (passwordInput == false) { 
        ofSetColor(230, 230, 230); 
    }
    else { 
        ofSetColor(250, 250, 250); 
    }
    ofDrawRectRounded(passwordBox, 10);

    if (createAccountScreen == true) {
        if (rePasswordInput == false) { 
            ofSetColor(230, 230, 230); 
        }
        else { 
            ofSetColor(250, 250, 250); 
        }
        ofDrawRectRounded(rePasswordBox, 10);
    }

    // box borders
    ofNoFill();
    ofSetLineWidth(1.5);
    ofSetColor(198, 198, 198);
    ofDrawRectRounded(emailBox, 15);
    ofDrawRectRounded(passwordBox, 15);
    if (createAccountScreen == true) {
        ofDrawRectRounded(rePasswordBox, 15);
    }
    ofFill();

    // input text
    drawFieldText(
        standardFont, Email, "Email Address", 745, 204, emailInput, ofColor(70, 70, 70), ofColor(160, 160, 160));
    drawFieldText(
        standardFont, Password, "Password", 745, 294, passwordInput, ofColor(70, 70, 70), ofColor(160, 160, 160));
    if (createAccountScreen == true) {
        drawFieldText(
            standardFont,
            RePassword,
            "Re-enter Password",
            745,
            384,
            rePasswordInput,
            ofColor(70, 70, 70),
            ofColor(160, 160, 160));
    }

    // login screen extras
    ofSetColor(31, 108, 159);
    if (createAccountScreen == false) {  
        smallFont.drawString("Forgot Password?", 965, 363);
        smallFont.drawString("Don't have an account?", 750, 580);
    }
    else {
        smallFont.drawString("Already have an account?", 745, 580);
    }

    if (!statusMessage.empty()) {
        const bool isLoginError =
            statusMessage.find("username or password") != string::npos ||
            statusMessage.find("Username or password") != string::npos;
        ofSetColor(isLoginError ? ofColor(200, 50, 50) : ofColor(120, 120, 120));
        smallFont.drawString(statusMessage, 740, createAccountScreen ? 535 : 470);
    }
}

//--------------------------------------------------------------
void ofApp::drawMainScreen() {
    ofBackground(250, 250, 250);

    // top bar
    ofSetColor(19, 128, 240);
    ofDrawRectangle(topBar);

    // search box
    if (searchInput == false) { ofSetColor(230, 230, 230); }
    else { ofSetColor(250, 250, 250); }
    ofDrawRectRounded(searchBox, 10);
    ofNoFill();
    ofSetLineWidth(1.5);
    ofSetColor(198, 198, 198);
    ofDrawRectRounded(searchBox, 10);
    ofFill();
    drawFieldText(smallFont, searchString, "Search", 35, 53, searchInput, ofColor(70, 70, 70), ofColor(160, 160, 160));
    // search icon (simple circle + line)
    ofNoFill();
    ofSetLineWidth(2.5);
    ofDrawCircle(288, 44, 12);
    ofSetLineWidth(2);
    ofDrawLine(297, 53, 308, 64);
    ofFill();

    // add new box
    ofSetColor(250, 250, 250);
    ofDrawRectRounded(addNewBox, 10);
    ofNoFill();
    ofSetLineWidth(1.5);
    ofSetColor(198, 198, 198);
    ofDrawRectRounded(addNewBox, 10);
    ofFill();
    ofSetColor(80, 80, 80);
    smallFont.drawString("Add New", 365, 53);
    // + icon
    ofSetLineWidth(2.5);
    ofNoFill();
    ofDrawCircle(463, 44, 12);
    ofFill();
    ofDrawLine(463, 35, 463, 53);
    ofDrawLine(454, 44, 472, 44);

    // title
    ofSetColor(255);
    headerMedFont.drawString("Password Manager", 520, 62);

    // header bar
    ofSetColor(230, 230, 230);
    ofDrawRectangle(headerBar);
    ofDrawRectangle(1245, 90, 35, 65);
    ofSetColor(0, 0, 0);
    ofSetLineWidth(1.0f);
    ofDrawLine(0, 155, 1245, 155);
    ofSetColor(80, 80, 80);
    headerFont.drawString("App/Website Name", 90, 133);
    headerFont.drawString("Username", 490, 133);
    headerFont.drawString("Password", 830, 133);

    // scrollbar track area bg
    ofSetColor(230, 230, 230);
    ofDrawRectangle(1245, 155, 35, 565);

    // clip rows to the vault list area
    glEnable(GL_SCISSOR_TEST);
    glScissor(0, ofGetHeight() - (155 + 565), 1245, 565);
    ofPushMatrix();
    ofTranslate(0, -scrollOffset);

    float contentHeight = filteredEntries.size() * rowHeight;
    float viewStart = 155 + scrollOffset;
    float viewEnd = 155 + scrollOffset + 565;

    for (int i = 0; i < filteredEntries.size(); i++) {
        float rowY = 155 + i * rowHeight;

        if (rowY + rowHeight < viewStart || rowY > viewEnd) continue; // skip off-screen rows

        // alternating row colour
        if (i % 2 == 0) { ofSetColor(235, 235, 235); }
        else { ofSetColor(248, 248, 248); }
        ofDrawRectangle(0, rowY, 1245, rowHeight);

        const float rowCenterY = rowY + rowHeight * 0.5f;
        auto centeredTextBaseline = [&](const string& value) {
            const ofRectangle bounds = smallFont.getStringBoundingBox(value, 0, 0);
            return rowY + (rowHeight - bounds.height) * 0.5f - bounds.y;
        };
        ofSetColor(80, 80, 80);
        smallFont.drawString(filteredEntries[i].appName, 60, centeredTextBaseline(filteredEntries[i].appName));
        smallFont.drawString(filteredEntries[i].username, 460, centeredTextBaseline(filteredEntries[i].username));

            // password — masked or visible
            if (i < passwordVisible.size() && passwordVisible[i] == true) {
                smallFont.drawString(filteredEntries[i].password, 820, centeredTextBaseline(filteredEntries[i].password));
            }
            else {
                string masked = string(filteredEntries[i].password.size(), '*');
                smallFont.drawString(masked, 820, centeredTextBaseline("Password"));
            }

        // pencil icon (left)
        ofSetColor(120, 120, 120);
        ofSetLineWidth(1.5);
        ofNoFill();
        // pencil body
        ofDrawLine(18, rowCenterY + 1, 30, rowCenterY - 11);
        ofDrawLine(30, rowCenterY - 11, 36, rowCenterY - 5);
        ofDrawLine(36, rowCenterY - 5, 24, rowCenterY + 7);
        ofDrawLine(24, rowCenterY + 7, 18, rowCenterY + 1);
        // pencil tip
        ofDrawLine(18, rowCenterY + 1, 15, rowCenterY + 11);
        ofDrawLine(15, rowCenterY + 11, 24, rowCenterY + 7);
        ofFill();

        // bin icon
        ofSetColor(120, 120, 120);
        ofNoFill();
        ofSetLineWidth(1.5);
        ofDrawRectangle(1088, rowCenterY - 10, 18, 20);
        ofDrawLine(1085, rowCenterY - 10, 1109, rowCenterY - 10);
        ofDrawLine(1094, rowCenterY - 14, 1100, rowCenterY - 14);
        ofDrawLine(1094, rowCenterY - 6, 1094, rowCenterY + 6);
        ofDrawLine(1097, rowCenterY - 6, 1097, rowCenterY + 6);
        ofDrawLine(1100, rowCenterY - 6, 1100, rowCenterY + 6);
        ofFill();

        // eye icon (right)
        ofSetColor(120, 120, 120);
        ofNoFill();
        ofSetLineWidth(1.5);
        // eye outline arc approximation
        ofDrawLine(1160, rowCenterY, 1190, rowCenterY);
        ofDrawEllipse(1175, rowCenterY, 36, 20);
        ofDrawCircle(1175, rowCenterY, 6);
        ofFill();
    }

    ofPopMatrix();
    glDisable(GL_SCISSOR_TEST);

    // scrollbar thumb
    float contentH = max((float)filteredEntries.size() * rowHeight, 1.0f);
    scrollBar.draw(contentH, 565, scrollOffset);

    // popup overlay
    if (popupOpen == true) {
        ofSetColor(0, 0, 0, 120);
        ofDrawRectangle(0, 0, 1280, 720);

        ofSetColor(245, 245, 245);
        ofDrawRectRounded(popupBG, 12);

        ofSetColor(51, 51, 51);
        smallFont.drawString("Add New Entry", 575, 135);

        if (popupNameInput == false) { ofSetColor(230, 230, 230); }
        else { ofSetColor(255, 255, 255); }
        ofDrawRectRounded(popupNameBox, 8);

        if (popupUserInput == false) { ofSetColor(230, 230, 230); }
        else { ofSetColor(255, 255, 255); }
        ofDrawRectRounded(popupUserBox, 8);

        if (popupPassInput == false) { ofSetColor(230, 230, 230); }
        else { ofSetColor(255, 255, 255); }
        ofDrawRectRounded(popupPassBox, 8);

        if (popupNotesInput == false) { ofSetColor(230, 230, 230); }
        else { ofSetColor(255, 255, 255); }
        ofDrawRectRounded(popupNotesBox, 8);

        ofNoFill();
        ofSetLineWidth(1);
        ofSetColor(198, 198, 198);
        ofDrawRectRounded(popupNameBox, 8);
        ofDrawRectRounded(popupUserBox, 8);
        ofDrawRectRounded(popupPassBox, 8);
        ofDrawRectRounded(popupNotesBox, 8);
        ofFill();

        drawFieldText(
            smallFont, popupName, "App/Website Name", 440, 178, popupNameInput, ofColor(50, 50, 50), ofColor(160, 160, 160));
        drawFieldText(
            smallFont, popupUser, "Username", 440, 243, popupUserInput, ofColor(50, 50, 50), ofColor(160, 160, 160));
        drawFieldText(
            smallFont, popupPass, "Password", 440, 308, popupPassInput, ofColor(50, 50, 50), ofColor(160, 160, 160));
        drawFieldText(
            smallFont,
            popupNotes,
            "Notes / General Text",
            440,
            373,
            popupNotesInput,
            ofColor(50, 50, 50),
            ofColor(160, 160, 160));
    }

    // edit popup overlay
    if (editPopupOpen == true) {
        ofSetColor(0, 0, 0, 120);
        ofDrawRectangle(0, 0, 1280, 720);

        ofSetColor(245, 245, 245);
        ofDrawRectRounded(editPopupBG, 12);

        ofSetColor(51, 51, 51);
        smallFont.drawString("Edit Entry", 575, 135);

        if (editNameInput == false) { ofSetColor(230, 230, 230); }
        else { ofSetColor(255, 255, 255); }
        ofDrawRectRounded(editNameBox, 8);

        if (editUserInput == false) { ofSetColor(230, 230, 230); }
        else { ofSetColor(255, 255, 255); }
        ofDrawRectRounded(editUserBox, 8);

        if (editPassInput == false) { ofSetColor(230, 230, 230); }
        else { ofSetColor(255, 255, 255); }
        ofDrawRectRounded(editPassBox, 8);

        if (editNotesInput == false) { ofSetColor(230, 230, 230); }
        else { ofSetColor(255, 255, 255); }
        ofDrawRectRounded(editNotesBox, 8);

        ofNoFill();
        ofSetLineWidth(1);
        ofSetColor(198, 198, 198);
        ofDrawRectRounded(editNameBox, 8);
        ofDrawRectRounded(editUserBox, 8);
        ofDrawRectRounded(editPassBox, 8);
        ofDrawRectRounded(editNotesBox, 8);
        ofFill();

        drawFieldText(
            smallFont, editName, "App/Website Name", 440, 178, editNameInput, ofColor(50, 50, 50), ofColor(160, 160, 160));
        drawFieldText(
            smallFont, editUser, "Username", 440, 243, editUserInput, ofColor(50, 50, 50), ofColor(160, 160, 160));
        drawFieldText(
            smallFont, editPass, "Password", 440, 308, editPassInput, ofColor(50, 50, 50), ofColor(160, 160, 160));
        drawFieldText(
            smallFont,
            editNotes,
            "Notes / General Text",
            440,
            373,
            editNotesInput,
            ofColor(50, 50, 50),
            ofColor(160, 160, 160));
    }

}

//--------------------------------------------------------------
bool ofApp::shouldDrawCaret() const {
    return static_cast<int>(ofGetElapsedTimef() * 2.0f) % 2 == 0;
}

//--------------------------------------------------------------
void ofApp::drawFieldText(
    const ofTrueTypeFont& font,
    const string& value,
    const string& placeholder,
    float x,
    float y,
    bool active,
    const ofColor& textColor,
    const ofColor& placeholderColor) const {
    const bool showingPlaceholder = !placeholder.empty() && value == placeholder;

    ofPushStyle();
    ofSetColor(showingPlaceholder ? placeholderColor : textColor);
    if (!value.empty()) {
        font.drawString(value, x, y);
    }
    ofPopStyle();

    drawCaretForText(font, value, placeholder, x, y, active);
}

//--------------------------------------------------------------
void ofApp::drawCaretForText(
    const ofTrueTypeFont& font,
    const string& value,
    const string& placeholder,
    float x,
    float y,
    bool active) const {
    if (!active || !shouldDrawCaret() || value == placeholder) {
        return;
    }

    const float caretX = x + font.stringWidth(value) + 2.0f;
    const float caretTop = y - font.stringHeight("Ay");
    const float caretBottom = y + 4.0f;

    ofPushStyle();
    ofSetColor(80, 80, 80);
    ofSetLineWidth(1.5f);
    ofDrawLine(caretX, caretTop, caretX, caretBottom);
    ofPopStyle();
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key) {
    auto activateLoginField = [&](int index) {
        emailInput = index == 0;
        passwordInput = index == 1;
        rePasswordInput = createAccountScreen && index == 2;

        if (emailInput && Email == "Email Address") Email = "";
        if (passwordInput && Password == "Password") Password = "";
        if (rePasswordInput && RePassword == "Re-enter Password") RePassword = "";
    };

    auto activatePopupField = [&](int index) {
        popupNameInput = index == 0;
        popupUserInput = index == 1;
        popupPassInput = index == 2;
        popupNotesInput = index == 3;

        if (popupNameInput && popupName == "App/Website Name") popupName = "";
        if (popupUserInput && popupUser == "Username") popupUser = "";
        if (popupPassInput && popupPass == "Password") popupPass = "";
        if (popupNotesInput && popupNotes == "Notes / General Text") popupNotes = "";
    };

    auto activateEditField = [&](int index) {
        editNameInput = index == 0;
        editUserInput = index == 1;
        editPassInput = index == 2;
        editNotesInput = index == 3;

        if (editNameInput && editName == "App/Website Name") editName = "";
        if (editUserInput && editUser == "Username") editUser = "";
        if (editPassInput && editPass == "Password") editPass = "";
        if (editNotesInput && editNotes == "Notes / General Text") editNotes = "";
    };

    if (mainScreen == false) {
        if (key == OF_KEY_TAB) {
            const int fieldCount = createAccountScreen ? 3 : 2;
            int activeIndex = emailInput ? 0 : (passwordInput ? 1 : (rePasswordInput ? 2 : -1));
            const int nextIndex = (activeIndex + 1 + fieldCount) % fieldCount;
            activateLoginField(nextIndex);
            return;
        }

        if (key == OF_KEY_RETURN || key == '\r') {
            string action = createAccountScreen ? "Register" : "Login";
            buttonEvent(action);
            return;
        }

        // login / register screen input
        if (key == OF_KEY_BACKSPACE) {
            if (emailInput && Email.size() > 0) Email.pop_back();
            else if (passwordInput && Password.size() > 0) Password.pop_back();
            else if (rePasswordInput && RePassword.size() > 0) RePassword.pop_back();
        }
        else if (key >= 32 && key < 128) {
            if (emailInput) Email += (char)key;
            if (passwordInput) Password += (char)key;
            if (rePasswordInput) RePassword += (char)key;
        }
    }
    else {
        if (editPopupOpen) {
            if (key == OF_KEY_TAB) {
                int activeIndex = editNameInput ? 0 : (editUserInput ? 1 : (editPassInput ? 2 : (editNotesInput ? 3 : -1)));
                const int nextIndex = (activeIndex + 1 + 4) % 4;
                activateEditField(nextIndex);
                return;
            }

            if (key == OF_KEY_RETURN || key == '\r') {
                string action = "Save";
                buttonEvent(action);
                return;
            }
        }

        if (popupOpen) {
            if (key == OF_KEY_TAB) {
                int activeIndex = popupNameInput ? 0 : (popupUserInput ? 1 : (popupPassInput ? 2 : (popupNotesInput ? 3 : -1)));
                const int nextIndex = (activeIndex + 1 + 4) % 4;
                activatePopupField(nextIndex);
                return;
            }

            if (key == OF_KEY_RETURN || key == '\r') {
                string action = "Add";
                buttonEvent(action);
                return;
            }
        }

        if (key == OF_KEY_TAB && !popupOpen && !editPopupOpen) {
            searchInput = true;
            if (searchString == "Search") {
                searchString = "";
            }
            return;
        }

        // main screen input
        if (key == OF_KEY_BACKSPACE) {
            if (searchInput && searchString.size() > 0) {
                searchString.pop_back();
                updateFilter();
            }
            else if (popupNameInput && popupName.size() > 0) popupName.pop_back();
            else if (popupUserInput && popupUser.size() > 0) popupUser.pop_back();
            else if (popupPassInput && popupPass.size() > 0) popupPass.pop_back();
            else if (popupNotesInput && popupNotes.size() > 0) popupNotes.pop_back();
            else if (editNameInput && editName.size() > 0) editName.pop_back();
            else if (editUserInput && editUser.size() > 0) editUser.pop_back();
            else if (editPassInput && editPass.size() > 0) editPass.pop_back();
            else if (editNotesInput && editNotes.size() > 0) editNotes.pop_back();
        }
        else if (key >= 32 && key < 128) {
            if (searchInput) { searchString += (char)key; updateFilter(); }
            if (popupNameInput) popupName += (char)key;
            if (popupUserInput) popupUser += (char)key;
            if (popupPassInput) popupPass += (char)key;
            if (popupNotesInput) popupNotes += (char)key;
            if (editNameInput) editName += (char)key;
            if (editUserInput) editUser += (char)key;
            if (editPassInput) editPass += (char)key;
            if (editNotesInput) editNotes += (char)key;
        }
    }
}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button) {
    if (mainScreen == false) {
        if (emailBox.inside(x, y) && emailInput == false) {
            emailInput = true;
            passwordInput = false;
            rePasswordInput = false;
            
            if (Email == "Email Address") {
                Email = "";
            }
        }
        else if (passwordBox.inside(x, y) && passwordInput == false) {
            passwordInput = true;
            emailInput = false;
            rePasswordInput = false;
            
            if (Password == "Password") {
                Password = "";
            }
        }
        else if (rePasswordBox.inside(x, y) && rePasswordInput == false && createAccountScreen == true) {
            rePasswordInput = true;
            emailInput = false;
            passwordInput = false;
            
            if(RePassword == "Re-enter Password") {
                RePassword = "";
            }
        }
        else {
            emailInput = false;
            passwordInput = false;
            rePasswordInput = false;
        }
        return;
    }

    // main screen clicks
    // edit popup clicks
    if (editPopupOpen == true) {
        if (editNameBox.inside(x, y) && editNameInput == false) {
            editNameInput = true;
            editUserInput = false;
            editPassInput = false;
            editNotesInput = false;
            if (editName == "App/Website Name") {
                editName = "";
            }
        }
        else if (editUserBox.inside(x, y) && editUserInput == false) {
            editUserInput = true;
            editNameInput = false;
            editPassInput = false;
            editNotesInput = false;
            if (editUser == "Username") {
                editUser = "";
            }
        }
        else if (editPassBox.inside(x, y) && editPassInput == false) {
            editPassInput = true;
            editNameInput = false;
            editUserInput = false;
            editNotesInput = false;
            if (editPass == "Password") {
                editPass = "";
            }
        }
        else if (editNotesBox.inside(x, y) && editNotesInput == false) {
            editNotesInput = true;
            editNameInput = false;
            editUserInput = false;
            editPassInput = false;
            if (editNotes == "Notes / General Text") {
                editNotes = "";
            }
        }
        else if (!editNameBox.inside(x, y) && !editUserBox.inside(x, y) && !editPassBox.inside(x, y) &&
                 !editNotesBox.inside(x, y)) {
            editNameInput = false;
            editUserInput = false;
            editPassInput = false;
            editNotesInput = false;
        }
        return;
    }

    // add popup clicks
    if (popupOpen == true) {
        if (popupNameBox.inside(x, y) && popupNameInput == false) {
            popupNameInput = true;
            popupUserInput = false;
            popupPassInput = false;
            popupNotesInput = false;

            if (popupName == "App/Website Name") {
                popupName = "";
            }
        }
        else if (popupUserBox.inside(x, y) && popupUserInput == false) {
            popupUserInput = true;
            popupNameInput = false;
            popupPassInput = false;
            popupNotesInput = false;

            if (popupUser == "Username") {
                popupUser = "";
            }
        }
        else if (popupPassBox.inside(x, y) && popupPassInput == false) {
            popupPassInput = true;
            popupNameInput = false;
            popupUserInput = false;
            popupNotesInput = false;

            if (popupPass == "Password") {
                popupPass = "";
            }
        }
        else if (popupNotesBox.inside(x, y) && popupNotesInput == false) {
            popupNotesInput = true;
            popupNameInput = false;
            popupUserInput = false;
            popupPassInput = false;

            if (popupNotes == "Notes / General Text") {
                popupNotes = "";
            }
        }
        else if (!popupNameBox.inside(x, y) && !popupUserBox.inside(x, y) && !popupPassBox.inside(x, y) &&
                 !popupNotesBox.inside(x, y)) {
            popupNameInput = false;
            popupUserInput = false;
            popupPassInput = false;
            popupNotesInput = false;
        }
        return;
    }

    // search box
    if (searchBox.inside(x, y) && searchInput == false) {
        searchInput = true;
        searchString = "";
    }
    else if (!searchBox.inside(x, y)) {
        searchInput = false;
    }

    // add new box
    if (addNewBox.inside(x, y)) {
        popupOpen = true;
        popupNameInput = false; popupUserInput = false; popupPassInput = false; popupNotesInput = false;
        popupConfirmBtn.toggle(true);
        popupCancelBtn.toggle(true);
        return;
    }

    // scrollbar
    float contentH = filteredEntries.size() * rowHeight;
    scrollBar.mousePressed(x, y, contentH, 565, scrollOffset);

    // row interactions
    float adjustedY = y + scrollOffset - 155;
    int rowIndex = (int)(adjustedY / rowHeight);

    if (rowIndex >= 0 && rowIndex < filteredEntries.size() && x < 1245 && y > 155) {
        ofRectangle pencilRect(5, 155 + rowIndex * rowHeight - scrollOffset + 10, 45, 45);
        ofRectangle binRect(1080, 155 + rowIndex * rowHeight - scrollOffset + 10, 35, 35);
        ofRectangle eyeRect(1160, 155 + rowIndex * rowHeight - scrollOffset + 10, 55, 40);

        if (pencilRect.inside(x, y)) {
            // open edit popup pre-filled with this row's data
            editingRow = rowIndex;
            editName = filteredEntries[rowIndex].appName;
            editUser = filteredEntries[rowIndex].username;
            editPass = filteredEntries[rowIndex].password;
            editNotes = filteredEntries[rowIndex].notes;
            editNameInput = false;
            editUserInput = false;
            editPassInput = false;
            editNotesInput = false;
            editPopupOpen = true;
            editConfirmBtn.toggle(true);
            editCancelBtn.toggle(true);
        }
        else if (binRect.inside(x, y)) {
            if (backendBridge.deleteEntry(filteredEntries[rowIndex].id, statusMessage)) {
                entries = backendBridge.getAllEntries(statusMessage);
                updateFilter();
            }
        }
        else if (eyeRect.inside(x, y)) {
            while (passwordVisible.size() <= rowIndex) passwordVisible.push_back(false);
            passwordVisible[rowIndex] = !passwordVisible[rowIndex];
        }
    }
}

//--------------------------------------------------------------
void ofApp::mouseScrolled(int x, int y, float scrollX, float scrollY) {
    if (mainScreen == false) return;

    float contentH = filteredEntries.size() * rowHeight;
    scrollOffset -= scrollY * 20;
    scrollOffset = max(0.0f, min(scrollOffset, max(0.0f, contentH - 565)));
}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button) {
    if (!mainScreen) return;

    float contentH = filteredEntries.size() * rowHeight;
    if (contentH <= 565) return;

    const float newOffset = scrollBar.mouseDragged(x, y, contentH, 565);
    if (newOffset >= 0.0f) {
        scrollOffset = newOffset;
    }
}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button) {
    scrollBar.mouseReleased();
}

//--------------------------------------------------------------
void ofApp::buttonEvent(string& label) {
    if (label == "Create account") {
        createAccountScreen = true;
        emailInput = false; passwordInput = false; rePasswordInput = false;
        loginBtn.toggle(false); createBtn.toggle(false);
        registerBtn.toggle(true); loginAccountBtn.toggle(true);
    }
    else if (label == "Login account") {
        createAccountScreen = false;
        emailInput = false; passwordInput = false; rePasswordInput = false;
        loginBtn.toggle(true); createBtn.toggle(true);
        registerBtn.toggle(false); loginAccountBtn.toggle(false);
    }
    else if (label == "Login") {
        const string email = sanitiseField(Email, "Email Address");
        const string password = sanitiseField(Password, "Password");

        if (backendBridge.login(email, password, statusMessage)) {
            mainScreen = true;
            loginBtn.toggle(false); createBtn.toggle(false);
            registerBtn.toggle(false); loginAccountBtn.toggle(false);
            logoutBtn.toggle(true);
            entries = backendBridge.getAllEntries(statusMessage);
            updateFilter();
        }
    }
    else if (label == "Register") {
        const string email = sanitiseField(Email, "Email Address");
        const string password = sanitiseField(Password, "Password");
        const string confirmation = sanitiseField(RePassword, "Re-enter Password");

        if (backendBridge.registerAccount(email, password, confirmation, statusMessage)) {
            createAccountScreen = false;
            emailInput = false; passwordInput = false; rePasswordInput = false;
            loginBtn.toggle(true); createBtn.toggle(true);
            registerBtn.toggle(false); loginAccountBtn.toggle(false);
            Password = "Password";
            RePassword = "Re-enter Password";
        }
    }
    else if (label == "Add") {
        const string appName = sanitiseField(popupName, "App/Website Name");
        const string username = sanitiseField(popupUser, "Username");
        const string password = sanitiseField(popupPass, "Password");
        const string notes = sanitiseField(popupNotes, "Notes / General Text");

        if (backendBridge.addEntry(appName, username, password, notes, statusMessage)) {
            entries = backendBridge.getAllEntries(statusMessage);
            updateFilter();
        }
        popupOpen = false;
        resetPopupFields();
        popupConfirmBtn.toggle(false);
        popupCancelBtn.toggle(false);
    }
    else if (label == "Cancel") {
        popupOpen = false;
        resetPopupFields();
        popupConfirmBtn.toggle(false);
        popupCancelBtn.toggle(false);
    }
    else if (label == "Logout") {
        if (backendBridge.logout(statusMessage)) {
            resetToLoginScreen();
        }
    }
    else if (label == "Save") {
        if (editingRow >= 0 && editingRow < filteredEntries.size()) {
            const PasswordEntry currentEntry = filteredEntries[editingRow];
            const string appName = sanitiseField(editName, "App/Website Name");
            const string username = sanitiseField(editUser, "Username");
            const string password = sanitiseField(editPass, "Password");
            const string notes = sanitiseField(editNotes, "Notes / General Text");
            if (backendBridge.updateEntry(currentEntry.id, appName, username, password, notes, statusMessage)) {
                entries = backendBridge.getAllEntries(statusMessage);
                updateFilter();
            }
        }
        editPopupOpen = false;
        editingRow = -1;
        editNameInput = false; editUserInput = false; editPassInput = false; editNotesInput = false;
        editConfirmBtn.toggle(false);
        editCancelBtn.toggle(false);
    }
    else if (label == "Discard") {
        editPopupOpen = false;
        editingRow = -1;
        editNameInput = false; editUserInput = false; editPassInput = false; editNotesInput = false;
        editConfirmBtn.toggle(false);
        editCancelBtn.toggle(false);
    }
}

//--------------------------------------------------------------
void ofApp::updateFilter() {
    filteredEntries.clear();
    for (int i = 0; i < entries.size(); i++) {
        if (searchString == "Search" || searchString.size() == 0) {
            filteredEntries.push_back(entries[i]);
        }
        else {
            string entryLower = entries[i].appName + " " + entries[i].username + " " + entries[i].notes;
            string searchLower = searchString;
            for (int j = 0; j < entryLower.size(); j++) entryLower[j] = tolower(entryLower[j]);
            for (int j = 0; j < searchLower.size(); j++) searchLower[j] = tolower(searchLower[j]);
            if (entryLower.find(searchLower) != string::npos) {
                filteredEntries.push_back(entries[i]);
            }
        }
    }
    // sync passwordVisible to entries size, preserving existing values
    while (passwordVisible.size() < entries.size()) passwordVisible.push_back(false);
    scrollOffset = 0;
    editingRow = -1;
}

//--------------------------------------------------------------
string ofApp::sanitiseField(const string& value, const string& placeholder) {
    if (value == placeholder) {
        return "";
    }

    return value;
}

//--------------------------------------------------------------
void ofApp::resetToLoginScreen() {
    // Returning to a clean login state avoids leftover vault data remaining on screen after logout.
    mainScreen = false;
    createAccountScreen = false;
    popupOpen = false;
    searchInput = false;
    emailInput = false;
    passwordInput = false;
    rePasswordInput = false;
    editNameInput = false;
    editUserInput = false;
    editPassInput = false;
    editNotesInput = false;
    editingRow = -1;
    scrollOffset = 0;

    Email = "Email Address";
    Password = "Password";
    RePassword = "Re-enter Password";
    searchString = "Search";

    entries.clear();
    filteredEntries.clear();
    passwordVisible.clear();

    loginBtn.toggle(true);
    createBtn.toggle(true);
    registerBtn.toggle(false);
    loginAccountBtn.toggle(false);
    logoutBtn.toggle(false);

    resetPopupFields();
}

//--------------------------------------------------------------
void ofApp::resetPopupFields() {
    popupNameInput = false;
    popupUserInput = false;
    popupPassInput = false;
    popupNotesInput = false;
    popupName = "App/Website Name";
    popupUser = "Username";
    popupPass = "Password";
    popupNotes = "Notes / General Text";
}
