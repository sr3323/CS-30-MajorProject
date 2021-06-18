/*
   Computer Science 30 Major Project
   Benjamin Jackson-Reynolds

   Foot Controller
   May-June 2021

   This is designed to send keyboard(like keyboard shortcuts) or MIDI CC (Musical instrument digital interface Control Changes) to a computer

   This is for controlling various aspects in a computer such as playing or pausing a media player or starting a recording without the use of your hands
   The reason why this is useful mainly to me is so I can control things on my computer while I am playing an instrument.
   I mainly am going to use this with this program called Mobius by Circular Labs which is a software loop pedal (which i will demonstrate in the video).

   MIDI CC has 2 main signals that are used
   Velocity - a value from 0-127 which is basically how strong the message is. Think of this like how far a knob is rotated
   CC - What number the control change is. This is basically like what knob you are turning.
   These values can be mapped to control various functions in a DAW (Digital audio workstation) or any other music software
   
   This device is reprogrammed using serial (old-school kind of way of communicating with the computer both ways).
*/

#include <Bounce.h>
#include <EEPROM.h>

//Sets values for the pin number for each of the buttons and the reset pin
int button_1_pin = 23;
int button_2_pin = 22;
int button_3_pin = 2;
int button_4_pin = 1;
int button_5_pin = 0;
int power_mosfet_pin = 3; //Have a N channel MOSFET that intterupts the gound signal when pulled low. This is to act as a reset pin because the teensy doesn not have one.

// Debounces the buttons so that it doesn't register multiple presses when it shouldn't
Bounce button_1 = Bounce(button_1_pin, 50);
Bounce button_2 = Bounce(button_2_pin, 50);
Bounce button_3 = Bounce(button_3_pin, 50);
Bounce button_4 = Bounce(button_4_pin, 50);
Bounce button_5 = Bounce(button_5_pin, 50);

//Reads values stored in EEPROM and saves them to lists.
int mode = EEPROM.read(0);
int button_CC[]       = {EEPROM.read(1), EEPROM.read(3), EEPROM.read(5), EEPROM.read(7), EEPROM.read(9) };
int button_velocity[] = {EEPROM.read(2), EEPROM.read(4), EEPROM.read(6), EEPROM.read(8), EEPROM.read(10)};

bool hasRan = false; //This bool makes it so that the keys aren't pressed as soon as you plug in the device.
int readButtons() {
  //detects what button is pressed. Returns the number of the button if one is pressed and 0 if none is pressed.

  //Updates the states of the buttons
  button_1.update();
  button_2.update();
  button_3.update();
  button_4.update();
  button_5.update();

  //returns a number based on what button is pressed
  if        (button_1.risingEdge() == true) {
    return 1;
  } else if (button_2.risingEdge() == true) {
    return 2;
  } else if (button_3.risingEdge() == true) {
    return 3;
  } else if (button_4.risingEdge() == true) {
    return 4;
  } else if (button_5.risingEdge() == true) {
    return 5;
  } else {
    return (0); //if no button is pressed return 0.
  }

}
int serialInput() {
  //Reads the input from serial and returns an integer from what is typed.

  while (Serial.available() == 0) {} //Waits for there to be incoming serial data.
  int readInfo = Serial.parseInt(); //Takes the numbers incoming from the serial stream and saves them as an int
  while (Serial.available()) {
    int clearCache = Serial.read(); // Clears the incoming serial data in the cache.
  }
  return (readInfo);
}

void waitForRelease(int buttonNumber) {
  //Waits for the specified button to be released and returns when it is.
  if (buttonNumber == 0) {
    while (digitalRead(button_1_pin) == LOW && !button_1.risingEdge()) {
      button_1.update();
    }
    return;
  }
  if (buttonNumber == 1) {
    while (digitalRead(button_2_pin) == LOW && !button_2.risingEdge()) {
      button_2.update();
    }
    return;
  }
  if (buttonNumber == 2) {
    while (digitalRead(button_3_pin) == LOW && !button_3.risingEdge()) {
      button_3.update();
    }
    return;
  }
  if (buttonNumber == 3) {
    while (digitalRead(button_4_pin) == LOW && !button_4.risingEdge()) {
      button_4.update();
    }
    return;
  }
  if (buttonNumber == 4) {
    while (digitalRead(button_5_pin) == LOW && !button_5.risingEdge()) {
      button_5.update();
    }
    return;
  }
}
void pressButton(int buttonNumber) {
  //Completes the action required for each button.

  buttonNumber = buttonNumber - 1; //Subtracts one to be comptatible with lists that start at 0.
  if (!mode) {
    //midi mode is selected
    usbMIDI.sendControlChange(button_CC[buttonNumber], button_velocity[buttonNumber], 1);
    waitForRelease(buttonNumber);
    usbMIDI.sendControlChange(button_CC[buttonNumber], 0, 1);
    //Sends a control change on channel 1 based on the values stored in EEPROM.
  } else {
    //Keyboard mode
    if (mode == 1) { //If it is in keyboard mode, run this code.
      int key = button_velocity[buttonNumber];
      if(key==32){//if the key is 32 (ascii for space) change the keycode to be the space.
        key = KEY_SPACE;
      }else{
        if (!isUpperCase(key)) {
          //If the key inputted is lowercase, make it uppercase. This is because the keyboard won't press the right key if it's not uppercase
          key = key - 32; //the ascii code for lowercase characters is 32 values higher then highercase characters
        }
        key = key + 61379; //For some reason I need to add 61379 for it to work. I can't explain why lol
      }
      //sets the modifier key based on the value stored in EEPROM
      if     (button_CC[buttonNumber] == 1) {
        Keyboard.set_modifier(MODIFIERKEY_CTRL);
      }
      else if (button_CC[buttonNumber] == 2) {
        Keyboard.set_modifier(MODIFIERKEY_SHIFT);
      }
      else if (button_CC[buttonNumber] == 3) {
        Keyboard.set_modifier(MODIFIERKEY_ALT);
      }
      else if (button_CC[buttonNumber] == 4) {
        Keyboard.set_modifier(MODIFIERKEY_GUI);
      }
      else if (button_CC[buttonNumber] == 5) {
        Keyboard.set_modifier(MODIFIERKEY_CTRL | MODIFIERKEY_SHIFT);
      }
      else {
        Keyboard.set_modifier(0); // If there's no value stored send 0(no modifier key)
      }
      Keyboard.send_now();

      delay(10);

      Keyboard.set_key1(key); //Sends the key stored in EEPROM
      Keyboard.send_now();

      waitForRelease(buttonNumber); //Wait for the button to release before the key is released.
      //Clears the keys and sends a event (aka releaseses all keys)
      Keyboard.set_modifier(0);
      Keyboard.set_key1(0);
      Keyboard.send_now();
    }
  }
}
void remapButton(int buttonNumber, bool isMidi) {
  //Remapps the button. The buttonNumber input is a value from 1-5 and the bool isMidi when false is keyboard.
  Serial.print("Remapping button ");
  Serial.println(buttonNumber);
  if (isMidi) {
    remapMidi(buttonNumber);
  } else {
    remapKeyboard(buttonNumber);
  }
}
void remapMidi(int buttonNumber) {
  //Writes values to EEPROM based on incoming serial data.
  Serial.println("MIDI Mode");
  buttonNumber = buttonNumber - 1;

  Serial.println("What CC would you like the button to control?");
  int readValue = serialInput();
  int writeAddress = (2 * buttonNumber) + 1;
  EEPROM.write(writeAddress, readValue);
  Serial.println(readValue);

  Serial.println("What value do you want the control change to be? (0-127)");
  readValue = serialInput();
  writeAddress = (2 * buttonNumber) + 2;
  EEPROM.write(writeAddress, readValue);
  Serial.println(readValue);
}
void remapKeyboard(int buttonNumber) {
  //Writes values to EEPROM based on incoming serial data.
  Serial.println("Keyboard Mode");
  buttonNumber = buttonNumber - 1;

  Serial.println("What Modifier keys do you want? type 0 for no modifier, 1 for CTRL, 2 for Shift, 3 for ALT, 4 for Windows key (on Windows) or Command on mac and 5 for CTRL + Shift");
  while (Serial.available() <= 0) {
    delayMicroseconds(1);
  }
  int readValue = Serial.parseInt();
  int writeAddress = (2 * buttonNumber) + 1;
  EEPROM.write(writeAddress, readValue);
  Serial.println(readValue);

  Serial.println("Type the letter you want pressed along with the modifier key");
  while (Serial.available() <= 0) {
    delayMicroseconds(1);
  }
  readValue = Serial.read();
  writeAddress = (2 * buttonNumber) + 2;
  if(readValue == ' '){ //if the key is space, set it to 32 (ascii code for space). If I don't do this, it will write 0 to the address.
    readValue = 32;
  }
  EEPROM.write(writeAddress, readValue);
  Serial.println(char(readValue));
}
void EEPROM_test() {
  //Prints the entire contents of the EEPROM.
  Serial.println("EEPROM Test Mode\n\nAddress\t\tValue\n");
  for (int i = 0; i < 128; i++) {
    int EEPROM_value = EEPROM.read(i);
    String message = String(i) + "\t\t" + String(EEPROM_value);
    Serial.println(message);
    delay(10); //I need this delay here or else it doesn't work for some reason (probably it sends too much data)
  }
}
void programmingMode() {

  //mode to change the mapping of the buttons without having to change the code
  while (Serial.available()) {
    int clearCache = Serial.read(); // Clears the incoming serial data in the cache.
  }
  int mode = EEPROM.read(0);
  if (!mode) {  //this means it's in midi mode right now
    Serial.println("Currently in MIDI mode");
  } else if (mode) {
    //Currently in Keyboard Mode
    Serial.println("Currently in Keyboard mode");
  } else {
    // Either the EEPROM is corrupted or this is the first time running the program.
    Serial.println("There is currently no mode selected. What mode would you like?");
  }
  Serial.println("Type 1 for Keyboard Mode or 0 for MIDI CC mode");
  int newMode;
  while (newMode != 1 && newMode != 0) {
    newMode = serialInput();
  }
  bool doneMapping = false;

  if (mode != newMode) {
    EEPROM.write(0, newMode);
    Serial.print("Now in ");
    if (newMode == 0) {
      Serial.println("MIDI Mode");
    } else {
      Serial.println("Keyboard Mode");
    }
    Serial.println("\nRemapping all the buttons.");
    for (int i = 1; i <= 5; i++) {
      remapButton(i, newMode == 0);
      doneMapping = true;
    }
  }

  mode = EEPROM.read(0);

  if (!doneMapping) {
    Serial.println("Press the button you want to remap");
  }

  while (doneMapping == false) {
    int buttonPressed = 0;
    while (!buttonPressed && !Serial.available()) {
      buttonPressed = readButtons();
    }
    if (Serial.available() && !buttonPressed) {
      if (Serial.read() != 13) {
        Serial.println("Programming Done");
        doneMapping = true;
      }

    } else {
      remapButton(buttonPressed, !mode);
      Serial.println("Press the button you want to remap or type 'e' to exit programming mode");
    }

  }
  //Connect a relay to a pin and have it dissconnect the power
  Serial.println("Programming complete.\nRebooting...");
  delay(1000); //So that you can see the message and it seems like it's doing something lol.
  Serial.end(); //ends the serial stream.
  delay(100);
  digitalWrite(power_mosfet_pin, LOW); //Reset the teensy.
}

void setup() {
  //This runs once when the device is plugged in or it's reset after programming.

  //Starts the serial and the keyboard.
  Serial.begin(9600);
  Keyboard.begin();

  //Initalizes the pins for the button. Sets a pullup resistor because when the button is pressed it connects the pin to ground.
  //without the pullup it would read random values from that pin since it's not connected to anything.
  pinMode(button_1_pin, INPUT_PULLUP);
  pinMode(button_2_pin, INPUT_PULLUP);
  pinMode(button_3_pin, INPUT_PULLUP);
  pinMode(button_4_pin, INPUT_PULLUP);
  pinMode(button_5_pin, INPUT_PULLUP);
  pinMode(power_mosfet_pin, OUTPUT);
  digitalWrite(power_mosfet_pin, HIGH); //Write high to the mosfet as to not accidentally reset the teensy and cause a bootloop.
  //If the middle button is pressed during startup, print the contents of the EEPROM for debugging.
  if (digitalRead(button_3_pin) == LOW) {
    EEPROM_test();
  }
}
void loop() {
  //This runs continously as long as the device is plugged in.

  //Updates the states of all the buttons.
  button_1.update();
  button_2.update();
  button_3.update();
  button_4.update();
  button_5.update();

  if (button_1.fallingEdge() || button_2.fallingEdge() || button_3.fallingEdge() || button_4.fallingEdge() || button_5.fallingEdge()) {
    //this code makes sure that the buttons are checked if they are released only after one is pressed.
    //This prevents all the buttons being pressed as soon as the device is plugged in
    hasRan = true;
  }

  //does the actions required when each button is pressed.
  if (hasRan == true) {
    if (button_1.fallingEdge()) {
      pressButton(1);
    }
    if (button_2.fallingEdge()) {
      pressButton(2);
    }
    if (button_3.fallingEdge()) {
      pressButton(3);
    }
    if (button_4.fallingEdge()) {
      pressButton(4);
    }
    if (button_5.fallingEdge()) {
      pressButton(5);
    }
  }
  while (usbMIDI.read()) {} //All usb midi devices should ignore incoming messages.

  if (Serial.available()) {
    programmingMode(); //When serial data is recieved, go into programming mode.
  }
}
