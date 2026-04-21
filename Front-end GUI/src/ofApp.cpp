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

    searchInput = false;
    searchString = "Search";

    popupOpen = false;
    popupName = "App/Website Name";
    popupUser = "Username";
    popupPass = "Password";
    popupNameInput = false;
    popupUserInput = false;
    popupPassInput = false;

    scrollOffset = 0;
    rowHeight = 80;
    editingRow = -1;
    editNameInput = false;
    editUserInput = false;
    editPassInput = false;
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
    popupConfirmBtn.create("Add", 490, 470, 120, 45, 0x1380F0, &smallFont);
    popupConfirmBtn.setCornerRadius(8);
    popupConfirmBtn.toggle(false);
    popupCancelBtn.create("Cancel", 630, 470, 120, 45, 0xE0E0E0, &smallFont);
    popupCancelBtn.setCornerRadius(8);
    popupCancelBtn.toggle(false);
    logoutBtn.create("Logout", 1155, 23, 90, 44, 0xFAFAFA, &smallFont);
    logoutBtn.setCornerRadius(10);
    logoutBtn.setTextColour(ofColor(80, 80, 80));
    logoutBtn.setBorderColour(ofColor(198, 198, 198));
    logoutBtn.toggle(false);

    popupBG.set(390, 240, 500, 290);
    popupNameBox.set(430, 280, 420, 50);
    popupUserBox.set(430, 345, 420, 50);
    popupPassBox.set(430, 410, 420, 50);

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
    if (popupPassUser == false && popupUser == "") {
        popupUser = "Username";
    }
    if (popupPassInput == false && popupPass == "") {
        popupPass = "Password";
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
    ofSetColor(160, 160, 160);
    standardFont.drawString(Email, 745, 204);
    standardFont.drawString(Password, 745, 294);
    if (createAccountScreen == true) {
        standardFont.drawString(RePassword, 745, 384);
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
        ofSetColor(120, 120, 120);
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
    ofSetColor(160, 160, 160);
    smallFont.drawString(searchString, 35, 53);
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

    if (!statusMessage.empty()) {
        ofSetColor(225, 240, 255);
        smallFont.drawString(statusMessage, 640, 62);
    }

    // header bar
    ofSetColor(230, 230, 230);
    ofDrawRectangle(headerBar);
    ofSetColor(80, 80, 80);
    headerFont.drawString("App/Website Name", 160, 133);
    headerFont.drawString("Username", 560, 133);
    headerFont.drawString("Password", 900, 133);

    // scrollbar track area bg
    ofSetColor(230, 230, 230);
    ofDrawRectangle(1245, 155, 35, 565);

    // clip rows to view area
    ofPushMatrix();
    ofTranslate(0, -scrollOffset);

    float contentHeight = filteredEntries.size() * rowHeight;
    float viewStart = scrollOffset;
    float viewEnd = scrollOffset + 565;

    for (int i = 0; i < filteredEntries.size(); i++) {
        float rowY = 155 + i * rowHeight;

        if (rowY + rowHeight < viewStart || rowY > viewEnd) continue; // skip off-screen rows

        // alternating row colour
        if (i % 2 == 0) { ofSetColor(235, 235, 235); }
        else { ofSetColor(248, 248, 248); }
        ofDrawRectangle(0, rowY, 1245, rowHeight);

        if (editingRow == i) {
            // draw edit boxes inline
            ofSetColor(255, 255, 255);
            ofDrawRectRounded(editNameBox, 6);
            ofDrawRectRounded(editUserBox, 6);
            ofDrawRectRounded(editPassBox, 6);
            ofNoFill();
            ofSetLineWidth(1);
            ofSetColor(180, 180, 180);
            ofDrawRectRounded(editNameBox, 6);
            ofDrawRectRounded(editUserBox, 6);
            ofDrawRectRounded(editPassBox, 6);
            ofFill();
            ofSetColor(50, 50, 50);
            smallFont.drawString(editName, editNameBox.x + 8, rowY + 50);
            smallFont.drawString(editUser, editUserBox.x + 8, rowY + 50);
            smallFont.drawString(editPass, editPassBox.x + 8, rowY + 50);
        }
        else {
            ofSetColor(80, 80, 80);
            smallFont.drawString(filteredEntries[i].appName, 60, rowY + 50);
            smallFont.drawString(filteredEntries[i].username, 460, rowY + 50);

            // password — masked or visible
            if (i < passwordVisible.size() && passwordVisible[i] == true) {
                smallFont.drawString(filteredEntries[i].password, 820, rowY + 50);
            }
            else {
                string masked = string(filteredEntries[i].password.size(), '*');
                smallFont.drawString(masked, 820, rowY + 50);
            }
        }

        // pencil icon (left)
        ofSetColor(120, 120, 120);
        ofSetLineWidth(1.5);
        ofNoFill();
        // pencil body
        ofDrawLine(18, rowY + 28, 30, rowY + 16);
        ofDrawLine(30, rowY + 16, 36, rowY + 22);
        ofDrawLine(36, rowY + 22, 24, rowY + 34);
        ofDrawLine(24, rowY + 34, 18, rowY + 28);
        // pencil tip
        ofDrawLine(18, rowY + 28, 15, rowY + 38);
        ofDrawLine(15, rowY + 38, 24, rowY + 34);
        ofFill();

        // bin icon
        ofSetColor(120, 120, 120);
        ofNoFill();
        ofSetLineWidth(1.5);
        ofDrawRectangle(1088, rowY + 17, 18, 20);
        ofDrawLine(1085, rowY + 17, 1109, rowY + 17);
        ofDrawLine(1094, rowY + 13, 1100, rowY + 13);
        ofDrawLine(1094, rowY + 21, 1094, rowY + 33);
        ofDrawLine(1097, rowY + 21, 1097, rowY + 33);
        ofDrawLine(1100, rowY + 21, 1100, rowY + 33);
        ofFill();

        // eye icon (right)
        ofSetColor(120, 120, 120);
        ofNoFill();
        ofSetLineWidth(1.5);
        // eye outline arc approximation
        ofDrawLine(1160, rowY + 27, 1190, rowY + 27);
        ofDrawEllipse(1175, rowY + 27, 36, 20);
        ofDrawCircle(1175, rowY + 27, 6);
        ofFill();
    }

    ofPopMatrix();

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
        smallFont.drawString("Add New Entry", 575, 270);

        if (popupNameInput == false) { ofSetColor(230, 230, 230); }
        else { ofSetColor(255, 255, 255); }
        ofDrawRectRounded(popupNameBox, 8);

        if (popupUserInput == false) { ofSetColor(230, 230, 230); }
        else { ofSetColor(255, 255, 255); }
        ofDrawRectRounded(popupUserBox, 8);

        if (popupPassInput == false) { ofSetColor(230, 230, 230); }
        else { ofSetColor(255, 255, 255); }
        ofDrawRectRounded(popupPassBox, 8);

        ofNoFill();
        ofSetLineWidth(1);
        ofSetColor(198, 198, 198);
        ofDrawRectRounded(popupNameBox, 8);
        ofDrawRectRounded(popupUserBox, 8);
        ofDrawRectRounded(popupPassBox, 8);
        ofFill();

        ofSetColor(160, 160, 160);
        smallFont.drawString(popupName, 440, 313);
        smallFont.drawString(popupUser, 440, 378);
        smallFont.drawString(popupPass, 440, 443);
    }
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key) {
    if (mainScreen == false) {
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
        // main screen input
        if (key == OF_KEY_BACKSPACE) {
            if (searchInput && searchString.size() > 0) {
                searchString.pop_back();
                updateFilter();
            }
            else if (popupNameInput && popupName.size() > 0) popupName.pop_back();
            else if (popupUserInput && popupUser.size() > 0) popupUser.pop_back();
            else if (popupPassInput && popupPass.size() > 0) popupPass.pop_back();
            else if (editNameInput && editName.size() > 0) editName.pop_back();
            else if (editUserInput && editUser.size() > 0) editUser.pop_back();
            else if (editPassInput && editPass.size() > 0) editPass.pop_back();
        }
        else if (key == OF_KEY_RETURN) {
            // confirm inline edit on enter
            if (editingRow >= 0 && editingRow < filteredEntries.size()) {
                if (backendBridge.updateEntry(filteredEntries[editingRow].id, editName, editUser, editPass, statusMessage)) {
                    updateFilter();
                }
                editingRow = -1;
                editNameInput = false;
                editUserInput = false;
                editPassInput = false;
            }
        }
        else if (key >= 32 && key < 128) {
            if (searchInput) { searchString += (char)key; updateFilter(); }
            if (popupNameInput) popupName += (char)key;
            if (popupUserInput) popupUser += (char)key;
            if (popupPassInput) popupPass += (char)key;
            if (editNameInput) editName += (char)key;
            if (editUserInput) editUser += (char)key;
            if (editPassInput) editPass += (char)key;
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
    if (popupOpen == true) {
        if (popupNameBox.inside(x, y) && popupNameInput == false) {
            popupNameInput = true;
            popupUserInput = false;
            popupPassInput = false; 

            if (popupName == "App/Website Name") {
                popupName = "";
            }
            
        }
        else if (popupUserBox.inside(x, y) && popupUserInput == false) {
            popupUserInput = true;
            popupNameInput = false;
            popupPassInput = false;
            
            if (popupUser == "Username") {
                popupUser = "";
            }
           
        }
        else if (popupPassBox.inside(x, y) && popupPassInput == false) {
            popupPassInput = true;
            popupNameInput = false;
            popupUserInput = false; 
            
            if (popupPass == "Password") {
                popupPass = "";
            }
            
        }
        else if (!popupNameBox.inside(x, y) && !popupUserBox.inside(x, y) && !popupPassBox.inside(x, y)) {
            popupNameInput = false;
            popupUserInput = false;
            popupPassInput = false;
        }
        return; // block clicks behind popup
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
        popupNameInput = false; popupUserInput = false; popupPassInput = false;
        popupConfirmBtn.toggle(true);
        popupCancelBtn.toggle(true);
        return;
    }

    // scrollbar
    float contentH = filteredEntries.size() * rowHeight;
    scrollBar.mousePressed(x, y, contentH, 565, scrollOffset);

    // row interactions — adjust y for scroll offset
    float adjustedY = y + scrollOffset - 155;
    int rowIndex = (int)(adjustedY / rowHeight);

    if (rowIndex >= 0 && rowIndex < filteredEntries.size() && x < 1245 && y > 155) {
        // Pencil opens inline editing, bin deletes, and eye reveals the password.
        // Keeping each action in a fixed hit area makes the row easier to use.
        // pencil icon hit area (left side)
        ofRectangle pencilRect(5, 155 + rowIndex * rowHeight - scrollOffset + 10, 45, 45);
        ofRectangle deleteRect(1078, 155 + rowIndex * rowHeight - scrollOffset + 10, 40, 40);
        // eye icon hit area (right side)
        ofRectangle eyeRect(1150, 155 + rowIndex * rowHeight - scrollOffset + 10, 55, 40);

        if (pencilRect.inside(x, y)) {
            // start editing this row
            editingRow = rowIndex;
            editName = filteredEntries[rowIndex].appName;
            editUser = filteredEntries[rowIndex].username;
            editPass = filteredEntries[rowIndex].password;
            editNameInput = true;
            editUserInput = false;
            editPassInput = false;
            float rowY = 155 + rowIndex * rowHeight - scrollOffset;
            editNameBox.set(50, rowY + 20, 350, 40);
            editUserBox.set(450, rowY + 20, 350, 40);
            editPassBox.set(845, rowY + 20, 350, 40);
        }
        else if (deleteRect.inside(x, y)) {
            if (backendBridge.deleteEntry(filteredEntries[rowIndex].id, statusMessage)) {
                updateFilter();
            }
        }
        else if (eyeRect.inside(x, y)) {
            // toggle password visibility
            while (passwordVisible.size() <= rowIndex) passwordVisible.push_back(false);
            passwordVisible[rowIndex] = !passwordVisible[rowIndex];
        }
        else if (editingRow == rowIndex) {
            // switch focus between edit boxes
            if (editNameBox.inside(x, y + scrollOffset - 155 + 155)) { // re-check with scroll
                editNameInput = true; editUserInput = false; editPassInput = false;
            }
            else if (editUserBox.inside(x, y + scrollOffset - 155 + 155)) {
                editNameInput = false; editUserInput = true; editPassInput = false;
            }
            else if (editPassBox.inside(x, y + scrollOffset - 155 + 155)) {
                editNameInput = false; editUserInput = false; editPassInput = true;
            }
        }
        else {
            editingRow = -1;
            editNameInput = false; editUserInput = false; editPassInput = false;
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

        if (backendBridge.addEntry(appName, username, password, statusMessage)) {
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
}

//--------------------------------------------------------------
void ofApp::updateFilter() {
    const string query = (searchString == "Search") ? "" : searchString;
    filteredEntries = backendBridge.searchEntries(query, statusMessage);
    entries = backendBridge.getAllEntries(statusMessage);

    // reset passwordVisible to match new filtered list size
    passwordVisible.assign(filteredEntries.size(), false);
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
    popupName = "App/Website Name";
    popupUser = "Username";
    popupPass = "Password";
}
