#include <SPI.h>
#include <MFRC522.h>

#define RST_PIN         9           // Configurable, see typical pin layout above
#define SS_PIN          10          // Configurable, see typical pin layout above

MFRC522 mfrc522(SS_PIN, RST_PIN);   // Create MFRC522 instance

String inputString = "";         // a String to hold incoming data
bool stringComplete = false;     // whether the string is complete

//*****************************************************************************************//
void setup() {
  Serial.begin(9600);                                           // Initialize serial communications with the PC
  SPI.begin();                                                  // Init SPI bus
  mfrc522.PCD_Init();                                           // Init MFRC522 card
  Serial.println(F("Input read or write:"));                    // shows in serial that it is ready to read or write
  inputString.reserve(200);                                     // reserve 200 bytes for the inputString
}

//*****************************************************************************************//
void loop() {
  
    // Prepare key - all keys are set to FFFFFFFFFFFFh at chip delivery from the factory.
    MFRC522::MIFARE_Key key;
    for (byte i = 0; i < 6; i++) key.keyByte[i] = 0xFF;
  
    //some variables we need
    byte block;
    byte len;
    MFRC522::StatusCode status;
  
    //-------------------------------------------
    if (stringComplete){
      if(inputString.startsWith("read")){
        
        // Reset the loop if no new card present on the sensor/reader. This saves the entire process when idle.
        if ( ! mfrc522.PICC_IsNewCardPresent()) {
          return;
        }
      
        // Select one of the cards
        if ( ! mfrc522.PICC_ReadCardSerial()) {
          return;
        }
      
        Serial.println(F("**Card Detected:**"));
      
        //-------------------------------------------
        
        byte buffer1[18];

        //block = 4;
        len = 18;

        byte buffer2[18];
        block = 1;
      
        status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, 1, &key, &(mfrc522.uid)); //line 834
        if (status != MFRC522::STATUS_OK) {
          Serial.print(F("Authentication failed: "));
          Serial.println(mfrc522.GetStatusCodeName(status));
          return;
        }
      
        status = mfrc522.MIFARE_Read(block, buffer2, &len);
        if (status != MFRC522::STATUS_OK) {
          Serial.print(F("Reading failed: "));
          Serial.println(mfrc522.GetStatusCodeName(status));
          return;
        }
      
        //PRINT LAST NAME
        for (uint8_t i = 0; i < 16; i++) {
          Serial.write(buffer2[i] );
        }

        //----------------------------------------
    
        Serial.println(F("\n**End Reading**\n"));
      
      
        mfrc522.PICC_HaltA();               // Halt PICC
        mfrc522.PCD_StopCrypto1();          // Stop encryption on PCD

        inputString = "";
        stringComplete = false;
      }
      else if(inputString.startsWith("write")){

          // Reset the loop if no new card present on the sensor/reader. This saves the entire process when idle.
          if ( ! mfrc522.PICC_IsNewCardPresent()) {
            return;
          }
        
          // Select one of the cards
          if ( ! mfrc522.PICC_ReadCardSerial()) {
            return;
          }

        //---------------------------------------
        
        byte buffer[34];

        //---------------------------------------Write data
        Serial.setTimeout(20000L) ;     // wait until 20 seconds for input from serial
        // Ask data: 
        Serial.println(F("Enter data, ending with #"));
        len = Serial.readBytesUntil('#', (char *) buffer, 30) ; // read shape from serial
        for (byte i = len; i < 30; i++) buffer[i] = ' ';     // pad with spaces
      
        block = 1;
        //Serial.println(F("Authenticating using key A..."));
        status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, block, &key, &(mfrc522.uid));
        if (status != MFRC522::STATUS_OK) {
          Serial.print(F("PCD_Authenticate() failed: "));
          Serial.println(mfrc522.GetStatusCodeName(status));
          return;
        }
        else Serial.println(F("PCD_Authenticate() success: "));
      
        // Write block
        status = mfrc522.MIFARE_Write(block, buffer, 16);
        if (status != MFRC522::STATUS_OK) {
          Serial.print(F("MIFARE_Write() failed: "));
          Serial.println(mfrc522.GetStatusCodeName(status));
          return;
        }
        else Serial.println(F("MIFARE_Write() success: "));
      
        block = 2;
        //Serial.println(F("Authenticating using key A..."));
        status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, block, &key, &(mfrc522.uid));
        if (status != MFRC522::STATUS_OK) {
          Serial.print(F("PCD_Authenticate() failed: "));
          Serial.println(mfrc522.GetStatusCodeName(status));
          return;
        }
      
        // Write block
        status = mfrc522.MIFARE_Write(block, &buffer[16], 16);
        if (status != MFRC522::STATUS_OK) {
          Serial.print(F("MIFARE_Write() failed: "));
          Serial.println(mfrc522.GetStatusCodeName(status));
          return;
        }
        else Serial.println(F("MIFARE_Write() success: \n"));

        //---------------------------------------------------------------

        mfrc522.PICC_HaltA();               // Halt PICC
        mfrc522.PCD_StopCrypto1();          // Stop encryption on PCD
        
        inputString = "";
        stringComplete = false;
        
      }
      else{
        inputString = "";
        stringComplete = false;
      }
    }
}

void serialEvent() {
  while (Serial.available()) {
    // get the new byte:
    char inChar = (char)Serial.read();
    // add it to the inputString:
    inputString += inChar;
    // if the incoming character is a newline, set a flag so the main loop can
    // do something about it:
    if (inChar == '\n') {
      stringComplete = true;
    }
  }
}
