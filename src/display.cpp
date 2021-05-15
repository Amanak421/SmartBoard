#include "display.h"

ESP32Encoder encoder;
LiquidCrystal_I2C lcd(0x27,20,4); // set the LCD address to 0x3F for a 16 chars and 2 line display

Display::Button encoder_button = {23, 0, false};

const int DEBOUNCE_TIME = 400;
unsigned long debounceTimer;
bool last_state = true;

void IRAM_ATTR isr() {
                                      
    if (last_state == 1 && millis() - debounceTimer > DEBOUNCE_TIME) // neni-li nastaven priznak         
    {                                          // tlacitko stisknuto a uplynul-li vetsi cas  
                                               // od zmeny stavu tlacitka nez je mez (50ms)
      last_state = 0;                             // nastav priznak tlacitko stisknuto
      encoder_button.numberKeyPresses +=1;
      encoder_button.pressed = true;       // zmen hodnotu vystupu LED
    }

    debounceTimer = millis();                       // zapamatuj si posledni cas, kdy bylo nestisknuto
    last_state = 1;                            // nuluj priznak, tlacitko stisknuto

}


Display::Display(){
    
}

void Display::showLoadPage(){
    lcd.setCursor(4, 1);
    lcd.print("SMARTBOARD");
}

void Display::begin(){
    lcd.init(); // initialize the lcd
    lcd.init();
// Print a message to the LCD.
    lcd.backlight();
    lcd.setCursor(0,0);
    lcd.createChar(CURSOR_CHAR, cursor_char);
    lcd.createChar(BACK_CHAR, back_char);
    lcd.createChar(WIFI_CHAR, wifi_char);
    lcd.createChar(TICK_CHAR, tick_char);
    showLoadPage();
    encoderBegin();
    buttonBegin();
    Serial.print("pocet znaku pro heslo");
    Serial.println(wifi_pass_char.length());
}

void Display::encoderBegin(){
    // Enable the weak pull down resistors

	//ESP32Encoder::useInternalWeakPullResistors=DOWN;
	// Enable the weak pull up resistors
	ESP32Encoder::useInternalWeakPullResistors=UP;

	// use pin 19 and 18 for the first encoder
	encoder.attachHalfQuad(5, 17);
	// use pin 17 and 16 for the second encoder
		
	// set starting count value after attaching
	encoder.setCount(1);
}

void Display::buttonBegin(){
    pinMode(encoder_button.PIN, INPUT_PULLUP);
    attachInterrupt(encoder_button.PIN, isr, FALLING);
    Serial.print("TLACITKO PRIPOJENO");
}

bool Display::updateEncoder(){
    if(millis() - last_encoder_read > ENCODER_READ_PAUSE){
        int act_cursor = encoder.getCount();
        /*Serial.print("act_cursor: ");
        Serial.println(act_cursor);*/

        if(current_page == WIFI_CONN_NAME_PAGE && act_cursor != cursor){

            if(act_cursor > max_cursor){
                act_cursor = 4;
                encoder.setCount(4);
                wifi_offset += 1;
                if(wifi_offset > wifi_names.size() - 3){
                    wifi_offset = wifi_names.size() - 3;
                }
                showWifiNames();
            }else if(act_cursor < 2 && wifi_offset > 0){
                act_cursor = 2;
                encoder.setCount(2);
                wifi_offset -= 1;
                if(wifi_offset < 0){
                    wifi_offset = 0;
                }
                showWifiNames();
            }else if(act_cursor < 2 && wifi_offset <= 0){
                if(act_cursor < 1){
                    act_cursor = 1;
                    encoder.setCount(1);
                }
            }


        }else if(current_page == WIFI_CONN_PASS_PAGE && act_cursor != cursor){

            if(password_writing){
                
                wifi_pass_index = act_cursor - 1;

                if(act_cursor > wifi_pass_char.length()){
                    wifi_pass_index = 0;
                    act_cursor = 1;
                    encoder.setCount(1);
                }else if(act_cursor < 1){
                    wifi_pass_index = wifi_pass_char.length() - 1;
                    act_cursor = wifi_pass_char.length() - 1;
                    encoder.setCount(wifi_pass_char.length() - 1);
                }

                editWifiPass();

                Serial.println(wifi_pass_index);

            }


        }else if(act_cursor > max_cursor){
            act_cursor = 1;
            encoder.setCount(1);
        }else if(act_cursor < 1){
            act_cursor = max_cursor;
            encoder.setCount(max_cursor);
        }

        if(act_cursor != cursor){
            Serial.println("Encoder count = " + String(act_cursor));
            cursor = act_cursor;
            return true;
        }

        last_encoder_read = millis();
        return false;
    }
    return false;
}

void Display::showHomePage(){
    current_page = HOME;
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("M: ");
    if(selected_game_mode == INTERNET){
        lcd.print("INT");
    }else if(selected_game_mode == ENGINE){
        lcd.print("ENG");
    }else{
        lcd.print("NON");
    }

    lcd.setCursor(8, 0);
    lcd.print("W: ");
    if(wifi_state == DISCONNECT){
        lcd.print("Neprip.");
    }else if(wifi_state == CONNECT){
        lcd.print(wifi_names[selected_wifi_id]);
    }else{
        lcd.print("NON");
    }

    if(wifi_state == CONNECT && selected_game_mode != UNSELECTED){
        lcd.setCursor(1, 2);
        lcd.print("Cekam na zahajeni");
        lcd.setCursor(8, 3);
        lcd.print("hry");
    }else if(wifi_state == DISCONNECT){
        lcd.setCursor(2, 2);
        lcd.print("Pripojte k wifi!");
    }else if(selected_game_mode == UNSELECTED){
        lcd.setCursor(1, 2);
        lcd.print("Vyberte herni mod");
    }else{
        lcd.setCursor(5, 2);
        lcd.print("Doslo k chybe");
    }

}

void Display::showMainMenuPage(){
    current_page = MENU;
    max_cursor = 4;
    lcd.clear();
    lcd.setCursor(1, 0);
    lcd.write(byte(BACK_CHAR));
    lcd.print(" Domovska stranka");
    lcd.setCursor(1, 1);
    lcd.print("Nastaveni hry");
    lcd.setCursor(1, 2);
    lcd.print("Nastaveni WiFi");
    lcd.setCursor(1, 3);
    lcd.print("ZAHAJIT HRU");
    showCursor();
}

void Display::updateCursor(){
    if(current_page == MENU || current_page == WIFI_MENU || current_page == GAME_MENU || current_page == WIFI_MENU || current_page == WIFI_CONN_NAME_PAGE || current_page == WIFI_CONN_PASS_PAGE){
        if(updateEncoder() && !password_writing){
            lcd.setCursor(0, 0);
            lcd.print(" ");
            lcd.setCursor(0, 1);
            lcd.print(" ");
            lcd.setCursor(0, 2);
            lcd.print(" ");
            lcd.setCursor(0, 3);
            lcd.print(" ");
            lcd.setCursor(0, cursor - 1);
            lcd.write(byte(0));
        }
    }
        
}

void Display::showCursor(){
    lcd.setCursor(0, 0);
    lcd.print(" ");
    lcd.setCursor(0, 1);
    lcd.print(" ");
    lcd.setCursor(0, 2);
    lcd.print(" ");
    lcd.setCursor(0, 3);
    lcd.print(" ");
    lcd.setCursor(0, cursor - 1);
    lcd.write(byte(0));
}

void Display::updateButton(){
    if (encoder_button.pressed) {
      Serial.printf("Button 1 has been pressed %u times\n", encoder_button.numberKeyPresses);
      encoder_button.pressed = false;
      updatePage();
  }
}

void Display::updatePage(){     //po stisknutí tlačítka zobrazí pozadovanou stránku nebo nastavení
    if(current_page == HOME){
        showMainMenuPage();
    }else if(current_page == MENU){
        switch (cursor)
        {
        case 1:
            showHomePage();
            break;

        case 2:
            showGameMenuPage();
            break;

        case 3:
            showWifiMenuPage();
            break;

        case 4:
            game_start = true;
            if(selected_game_mode == ENGINE){
                showGameEnginePage();
            }
        
        default:
            break;
        }
    }
    else if(current_page == GAME_MENU){
        switch (cursor)
        {
        case 1:
            showMainMenuPage();
            break;

        case 2:
            updateSetingRow(6, 1, game_mods_text, selected_game_mode, &selected_game_mode);
            break;

        case 3:
            updateSetingRow(8, 2, colors_text, selected_color, &selected_color);
            break;
        
        case 4:
            updateSetingRow(13, 3, revers_text, selected_revers_state, &selected_revers_state);
            break;
        
        default:
            break;
        }
    }
    else if(current_page == WIFI_MENU){
        switch (cursor){
        case 1:
            showMainMenuPage();
            break;

        case 2:
            showWifiConnectPage();
            break;

        case 3:
            wifi_disconect = true;
            showWifiDissPage();
            break;

        case 4:
            wifi_refresh = true;
            showWifiRefreshPage();
            break;
        
        default:
            break;
        }
    }
    else if(current_page == WIFI_CONN_NAME_PAGE){
        switch (cursor){
            case 1:
                showWifiMenuPage();
                break;
            
            default:
                Serial.println(wifi_offset);
                Serial.println(cursor);
                int _selection = (cursor + wifi_offset) - 2;
                Serial.println(_selection);
                selected_wifi_id = _selection;
                Serial.println(wifi_names[selected_wifi_id]);
                showWifiPassPage();
                break;
        }
    }
    else if(current_page == WIFI_CONN_PASS_PAGE){
        switch (cursor)
        {
        case 1:
            if(!password_writing){
                showWifiMenuPage();
            }else{
                pass_cursor += 1;
                password += wifi_pass_char[wifi_pass_index];
                wifi_pass_index = 0;
                editWifiPass();
            }
            if(pass_cursor >= 19){

                wifi_connect = true;
                password.trim();
                _pass = password;
                wifi_names[selected_wifi_id].trim();
                _ssid = wifi_names[selected_wifi_id];

                Serial.println(_pass);
                Serial.println(_ssid);
                showWifiLoadPage();
                password_writing = false;

            }
            break;

        case 2:
            if(!password_writing){
                password_writing = true;
                max_cursor = wifi_pass_char.length();
            }else if(pass_cursor == 19){

                wifi_connect = true;
                password.trim();
                _pass = password;
                wifi_names[selected_wifi_id].trim();
                _ssid = wifi_names[selected_wifi_id];

                Serial.println(_pass);
                Serial.println(_ssid);
                showWifiLoadPage();
                password_writing = false;

            }else{
                pass_cursor += 1;
                password += wifi_pass_char[wifi_pass_index];
                wifi_pass_index = 0;
                editWifiPass();
            }
            break;
        
        default:
            pass_cursor += 1;
            password += wifi_pass_char[wifi_pass_index];
            wifi_pass_index = 0;
            if(pass_cursor == 19){

                wifi_connect = true;
                password.trim();
                _pass = password;
                wifi_names[selected_wifi_id].trim();
                _ssid = wifi_names[selected_wifi_id];

                Serial.println(_pass);
                Serial.println(_ssid);
                showWifiLoadPage();
                password_writing = false;

            }
            editWifiPass();
            break;
        }
    }
}

const char* Display::ssid(){
    const char* __ssid = _ssid.c_str();
    return __ssid;
}

const char* Display::pass(){
    const char* __pass = _pass.c_str();
    return __pass;
}

void Display::setPage(){
    showHomePage();
}

void Display::showGameMenuPage(){
    current_page = GAME_MENU;
    cursor = 1;
    max_cursor = 4;
    lcd.clear();
    lcd.setCursor(1, 0);
    lcd.write(byte(BACK_CHAR));
    lcd.print(" Menu");
    lcd.setCursor(1, 1);
    lcd.print("Mod: ");
    lcd.print(game_mods_text[selected_game_mode]);
    lcd.setCursor(1, 2);
    lcd.print("Barva: ");
    lcd.print(colors_text[selected_color]);
    lcd.setCursor(1, 3);
    lcd.print("Prevraceni: ");
    lcd.print(revers_text[selected_revers_state]);
    showCursor();
}

void Display::showWifiMenuPage(){
    current_page = WIFI_MENU;
    cursor = 1;
    max_cursor = 4;
    lcd.clear();
    lcd.setCursor(1, 0);
    lcd.write(byte(BACK_CHAR));
    lcd.print(" Menu");
    lcd.setCursor(1, 1);
    lcd.print("Pripojit");
    lcd.setCursor(1, 2);
    lcd.print("Odpojit");
    lcd.setCursor(1, 3);
    lcd.print("Znovu nacist");
    showCursor();
}

void Display::wifiLoadComplete(){
    lcd.noCursor();
    showHomePage();
    wifi_connect = false;
    wifi_state = CONNECT;
}

void Display::wifiDisconnectComplete(){
    showWifiMenuPage();
    wifi_disconect = false;
    wifi_state = DISCONNECT;
}

void Display::wifiRefreshComplete(){
    showWifiMenuPage();
    wifi_refresh = false;
}

void Display::updateSetingRow(int _cursor_char, int _cursor_row, std::vector<String> _items_text, int _act_item_id, int *p_store){
    lcd.setCursor(_cursor_char, _cursor_row);
    int _id = _act_item_id + 1;
    for(int i = _cursor_char; i < 19; i++){
        lcd.print(" ");
    }
    lcd.setCursor(_cursor_char, _cursor_row);
    if(_id >= _items_text.size()){
        _id = 0;
    }
    lcd.print(_items_text[_id]);
    *p_store = _id;
}

void Display::showWifiConnectPage(){
    current_page = WIFI_CONN_NAME_PAGE;
    cursor = 1;
    if(wifi_names.size() < 4){
        max_cursor = wifi_names.size() % 4 + 1;
    }else{
        max_cursor = 4;
    }
    lcd.clear();
    lcd.setCursor(1, 0);
    lcd.write(byte(BACK_CHAR));
    lcd.print(" Menu");

    Serial.print("Wifi count: ");
    Serial.println(wifi_names.size());

    showWifiNames();
    showCursor();
}

void Display::showWifiPassPage(){
    current_page = WIFI_CONN_PASS_PAGE;
    cursor = 1;
    max_cursor = 2;
    lcd.clear();
    lcd.setCursor(1, 0);
    lcd.write(byte(BACK_CHAR));
    lcd.print(" Zpet");
    lcd.setCursor(1, 1);
    lcd.print("Heslo: ");
    lcd.setCursor(19, 2);
    lcd.write(byte(TICK_CHAR));
    showCursor();
}

void Display::showWifiLoadPage(){
    current_page = WIFI_CONN_PASS_PAGE;
    lcd.clear();
    lcd.setCursor(5, 1);
    lcd.print("Pripojuji...");
}

void Display::showWifiRefreshPage(){
    current_page = WIFI_CONN_PASS_PAGE;
    lcd.clear();
    lcd.setCursor(6, 1);
    lcd.print("Nacitam...");
}

void Display::editWifiPass(){
    lcd.setCursor(pass_cursor, 2);
    lcd.cursor();
    Serial.println(wifi_pass_char.substring(wifi_pass_index, wifi_pass_index + 1));
    lcd.print(wifi_pass_char.substring(wifi_pass_index, wifi_pass_index + 1));

}

void Display::showWifiNames(){
    for(int i = 0; i < 3; i++){
       if(wifi_names.size() >= 3 + wifi_offset){
            lcd.setCursor(1, 1 + i);
            lcd.print("                  ");
            lcd.setCursor(1, 1 + i);
            lcd.print(wifi_names[i + wifi_offset]);
        }else if(wifi_names.size() < 3 && i < wifi_names.size()){
            lcd.setCursor(1, 1 + i);
            lcd.setCursor(1, 1 + i);
            lcd.print("                  ");
            lcd.setCursor(1, 1 + i);
            lcd.print(wifi_names[i]);
        }
    }
}

void Display::showWifiDissPage(){
    lcd.clear();
    lcd.setCursor(3, 2);
    lcd.print("Odpojuji...");
}

bool Display::wifiDissconnect(){
    return wifi_disconect;
}

bool Display::wifiConnect(){
    return wifi_connect;
}

bool Display::wifiRefresh(){
    return wifi_refresh;
}
 
void Display::setWifiList(std::vector<String> _ssids){
    wifi_names = _ssids;
}

void Display::showGameEnginePage(){
    current_page = GAME_ENGINE;
    lcd.clear();
    lcd.setCursor(1, 0);
    lcd.print("M: ENGINE");

    switch (engine_error){
    case NONE:
        if(on_move_engine == 0 && player_color_enigne == 0){
            lcd.setCursor(3, 2);
            lcd.print("Jsi na tahu...");
        }else{
            lcd.setCursor(4, 2);
            lcd.print("Hraje souper");
        }
        break;

    case ILLEGAL_MOVE:
        lcd.setCursor(4, 2);
        lcd.print("Neplatny tah");
        lcd.setCursor(3, 3);
        lcd.print("Vratte figurky");
        break;

    case NOT_YOUR_TURN:
        lcd.setCursor(4, 2);
        lcd.print("Nejsi na tahu");
    
    default:
        break;
    }
    

}

bool Display::gameStart(){
    return game_start;
}

void Display::engineInfoError(int _error){
    engine_error = _error;
    showGameEnginePage();
}

int Display::gameMode(){
    return selected_game_mode;
}
