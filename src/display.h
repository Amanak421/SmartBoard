#ifndef __DISPLAY_H__
#define __DISPLAY_H__

#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <ESP32Encoder.h>
#include <vector>


#define UNSELECTED 0x00

#define LOADING 0x01        //výběr stránek
#define HOME 0x02
#define MENU 0x03
#define WIFI_MENU 0x04
#define GAME_MENU 0x05
#define WIFI_CONN_NAME_PAGE 0x06
#define WIFI_CONN_PASS_PAGE 0x07
#define WIFI_LOAD_PAGE 0x08
#define GAME_ENGINE 0x09
#define GAME_INTERNET 0x0a

#define CONNECT 0x01        //stavy wifi
#define DISCONNECT 0x02

#define ENGINE 0x00     //nastaveni hry
#define INTERNET 0x01       

#define ROTARY_ENCODER_A_PIN 5      //připojení enkoderu
#define ROTARY_ENCODER_B_PIN 18
#define ROTARY_ENCODER_BUTTON_PIN 19
#define ROTARY_ENCODER_STEPS 2

#define CURSOR_SELECT 0x00      //jak pracuje kurzor - NEVYUŽITO
#define CURSOR_INTER_SELECT 0x01

#define WHITE 0x00      //výběr id barvy
#define BLACK 0x01

#define R_OFF 0x00        //výber zapnutí na vypnutí reverse
#define R_ON 0x01

#define NONE 0x00
#define ILLEGAL_MOVE 0x01
#define NOT_YOUR_TURN 0x02

class Display{

private:

    /*  speciální znaky */

    byte cursor_char[8] = {
        B00000,
        B01000,
        B01100,
        B01110,
        B01110,
        B01100,
        B01000,
        B00000
    };
    const int CURSOR_CHAR = 0;

    byte back_char[8] = {
        B00100,
        B01110,
        B11111,
        B00100,
        B00100,
        B00111,
        B00000,
        B00000
    };
    const int BACK_CHAR = 1;

    byte wifi_char[8] = {
        B00000,
        B01110,
        B10001,
        B00100,
        B01010,
        B00000,
        B00100,
        B00000
    };
    const int WIFI_CHAR = 2;

    byte tick_char[8] = { 
        B00000, 
        B00000, 
        B00001,
        B10010, 
        B10100, 
        B11000, 
        B10000 };
    const int TICK_CHAR = 3;

    /******************************/

    const int ENCODER_READ_PAUSE = 100;     //pauza mezi změnami enkoderu
    unsigned long last_encoder_read;


    int wifi_state = DISCONNECT;    //pocatecni nastaveni wifi a herniho modu
    int selected_game_mode = ENGINE;
    std::vector<String> game_mods_text = {"TY vs PC", "Online"};

    int selected_color = WHITE;
    std::vector<String> colors_text = {"Bila", "Cerna"};

    int selected_revers_state = R_OFF;
    std::vector<String> revers_text = {"ON", "OFF"};

    int current_page = HOME;

    bool wifi_disconect = false;        //indikace odpojení wifi
    bool wifi_connect = false;          //indikace připojení k wifi
    bool wifi_refresh = false;          //indikace znovu načtení seznamu sítí

    bool updateEncoder();       //updatuje enkoder -> zjistí zda se pohl a tuto změnu uloží

    void showCursor();  //ukáže kurzor na aktuálním místě

    int cursor = 1;
    int max_cursor = 4;

    /*zobrazení jednotlivých stránek*/

    void showLoadPage();
    void showHomePage();
    void showMainMenuPage();
    void showGameMenuPage();
    void showWifiMenuPage();
        void showWifiConnectPage();
        void showWifiPassPage();
        void showWifiLoadPage();
        void showWifiDissPage();
        void showWifiRefreshPage();
    void showGameEnginePage();

    /****************************************/

    void updateSetingRow(int _cursor_char, int _cursor_row, std::vector<String> _items_text, int _act_item_id, int *p_store);       //updatuje nastaveni na určité pozici

    std::vector<String> wifi_names;
    String sel_wifi_name = "nejake_jmeno";      //jméno wifi, ke které je deska připojena
    int selected_wifi_id = -1;
    int wifi_offset = 0;

    String _ssid = "";
    String _pass = "";

    void showWifiNames();
    void editWifiPass();

    String wifi_pass_char = " aAbBcCdDeEfFhHiIjJkKlLmMoOpPqQsStTuUvVwWxXyYzZ0123456789!@#$%^&*()~`_-=[]{}|;:<>?,./";
    String password = "";
    int wifi_pass_index = 0;
    int pass_cursor = 0;
    bool password_writing = false;

    bool game_start = false;

    /* hra engine */
    int on_move_engine = 0;
    int player_color_enigne = 0;
    int engine_error = NONE;

    
    /* init fce */
    void encoderBegin();
    void buttonBegin();
    /********************/

    void updatePage();      //updatuje stránku, které má být právě načtená, volá ji updateButton


public:

    Display();

    void begin();       // nastaví všechny prvky, které jsou potřeba ke správnému fungování
    void setPage();     //zobrazí první stránku
    void updateCursor();    //updatuje pozici kurzoru
    void updateButton();    //zkontroluje zda nebylo stisknuto tlačítko

    bool wifiDissconnect(); //vrátí true pokud se má odpojit wifi
    bool wifiConnect();
    bool wifiRefresh();
    void setWifiList(std::vector<String> _ssids);

    void wifiLoadComplete();
    void wifiDisconnectComplete();
    void wifiRefreshComplete();

    const char* ssid();
    const char* pass();

    bool gameStart();
    int gameMode();

    void engineInfoError(int _error);

    struct Button {     //struktura talačítka
        const uint8_t PIN;
        uint32_t numberKeyPresses;
        bool pressed;
    };
};

#endif