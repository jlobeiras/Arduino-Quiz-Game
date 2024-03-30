#include <hidboot.h> // Parses USB input
#include <usbhub.h>  // In case it is connected to composite USB HUB

// Arduino IDE needs to see the include statment in the ino file.
#ifdef dobogusinclude
    #include <spi4teensy3.h>
#endif
#include <SPI.h>


// Keyboard event processing class
class KeyboardInput : public KeyboardReportParser {

    char _keyAsc;
    int _keyCode;
    boolean _keyPressed;
    unsigned long _timeStamp;

public:

    void OnKeyPressed(uint8_t key) {
        _keyAsc = (char)key;
        _keyCode = (int)key;
        _keyPressed = true;
        _timeStamp = millis();
    }
 
    void OnKeyDown  (uint8_t mod, uint8_t key) {
        uint8_t c = OemToAscii(mod, key);
        if(c) OnKeyPressed(c);
    }

    boolean keyPressed() { return _keyPressed; }

    int getCode() { 
        if(!_keyPressed) return 0;
        _keyPressed = 0;
        return _keyCode;
    }
    
    char getAsc() {
        if(!_keyPressed) return 0;
        _keyPressed = 0;
        return _keyAsc;
    }

    unsigned long lastTime() {
        return millis() - _timeStamp;
    }

};


// Bit population counter (how many bits are 1)
int popcount(unsigned int val) {
    int retVal = 0;
    for(; val; val >>= 1)
        retVal += val & 0x01;
    return retVal;
}


// Class to parse the codes and store current question state
class Question {

    int last;
    int numCorrectas, constante, multiple;
    int col1, col2, numItems, secuencia;

public:

    Question() : last(99) { }

    // Scan the type of the question
    int scan(int data[4]) {
        Serial.println(data[0]*1000 + data[1]*100 + data[2]*10 + data[3]);
        if(last != data[0]) {
            last = data[0];
            reset();
        }
        switch(data[0]) {
            case 1: return pTest(data[1], data[2], data[3]);
            case 2: return pMult(data[1], data[2], data[3]);
            case 3: return pCols(data[1], data[2], data[3]);
            case 4: return pSort(data[1], data[2], data[3]);
        }
        Serial.println("invalid");
        return -1;
    }

    // Single answer question (a/b/c/d/e or true/false)
    // 1ABC with a+b+c odd for correct, even for incorrect
    int pTest(int a, int b, int c) {
        return a + b + c & 0x01;
    }

    // Multiple answer question
    // 2ABC with A=numAnswers, C=constant, B=2^x for correct (1,2,4,8)
    int pMult(int a, int b, int c) {
            if(constante != c || numCorrectas != a) {
                reset();
                constante = c;
                numCorrectas = a;
                multiple = 0;
            }
            if(popcount(b) != 1) return reset(0);
            multiple |= b;
            if(popcount(multiple) != numCorrectas) return -1;
            return reset(1);
    }

    // Pair match question
    // 3A00 with col1=A, 30BC with col2=|B-C|
    int pCols(int a, int b, int c) {
            if(b+c == 0) col1 = a;
            if(b+c != 0) col2 = abs(b-c);
            if(col1 > 9 || col2 > 9) return -1;
            if(col1 != col2) return reset(0);
            return reset(1);
    }

    // Sort question
    // 4ABC with A=numItems, B+C in sequence from 6 to A+5
    int pSort(int a, int b, int c) {
        if(numItems > 9 || secuencia > 19) {
            numItems = a;
            secuencia = 6;
        }
        if(b+c != secuencia) return reset(0);
        secuencia++;
        if(numItems != secuencia - 6) return -1;
        return reset(1);
    }
    
    // Reset state after success or failure
    int reset(int retVal = 0) {
        numCorrectas = numItems = 99;
        constante = secuencia = col1 = col2 = 99;
        multiple = 0;
        return retVal;
    }
};


// Play music on pin (pairs of frequency and duration ending in 0,0)
void musicPlay(int pin, int* melody) {
    for (int i = 0; melody[i+1]; i+=2) {
        if(melody[i] > 31)
            tone(pin, melody[i]);
        delay(melody[i+1]);
        noTone(pin);
    }
}

int nextMelody[] = { 220, 70, 440, 75, 0, 0 };
int sadMelody[] = { 330, 100, 220, 100, 110, 120, 0, 0 };
int happyMelody[] = { 440, 100, 550, 100, 660, 120, 0, 0 };
int readyMelody[] = { 523, 99, 0, 70, 659, 90, 783, 90, 0, 70, 659, 90, 783, 140, 0, 0 };


USB Usb;
HIDBoot<USB_HID_PROTOCOL_KEYBOARD> HidKeyboard(&Usb);
KeyboardInput keyboard;


// Configure serial port for debugging and initialize USB device
void setup() {
    Serial.begin(115200);
    #if !defined(__MIPSEL__)
        while(!Serial); 
    #endif
    Serial.println("Start");

    if(Usb.Init() == -1)
        Serial.println("OSC did not start.");

    delay(200);
    HidKeyboard.SetReportParser(0, &keyboard);
}


// Main exectuion loop, play ready music at startup
void loop() {
    Question qst;
    musicPlay(8, readyMelody);
    int code[4];
    int pos = 0;
    while(1) {
      
        // Read digits from USB scanner in sequence
        Usb.Task();
        if(keyboard.keyPressed())
            code[pos++ & 0x03] = keyboard.getAsc() - '0';
        if(keyboard.lastTime() > 100) pos = 0;
        if(pos < 4) continue;

        // We have received a complete 4 digit barcode
        int retVal = qst.scan(code);
        switch(retVal) {
             case -1: musicPlay(8, nextMelody); break;
             case  0: musicPlay(8, sadMelody); break;
             case  1: musicPlay(8, happyMelody); break;
        }
        Serial.println(retVal);
        pos = 0;
    }
}
