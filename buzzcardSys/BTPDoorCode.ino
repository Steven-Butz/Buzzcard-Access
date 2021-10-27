#define MAX_BITS 100                 // max number of bits 
#define MAX_CARD_CODES 200           // max number of cards in the system
#define MAX_BITCH_CODES 35
#define WEIGAND_WAIT_TIME  3000      // time to wait for another weigand pulse.  
#define DOOR_UNLOCK_TIME 5000        // time to allow the door to be unlocked after successful card read
#define BITCH_UNLOCK_TIME 1000        // time to allow the door to be unlocked after successful card read by a bitch
#define BITCH_SUCCESS_RATE 75        // number between [1,100] that determines how often bitches unlock the door

unsigned char databits[MAX_BITS];    // stores all of the data bits
unsigned char bitCount;              // number of bits currently captured
unsigned char flagDone;              // goes low when data is currently being captured
unsigned int weigand_counter;        // countdown until we assume there are no more bits

unsigned long facilityCode=0;        // decoded facility code
unsigned long cardCode=0;            // decoded card code

unsigned long latestUnlockTime = 0;

bool isBitch = false;

int unlockPin = 5; // HIGH is Locked, LOW is unlocked
int sensorPin = 9; // HIGH Is unlocked
int buttonPin = 12; // HIGH Is unlocked

//Add and remove buzzcards here:
unsigned long cardCodes [MAX_CARD_CODES] = {
  617028,  // Kasper Gammeltoft
  434998,  // Jake Williams
  615517,  // Stephen butts
  426366,  // Gay boy kumpf
  654450,  // Graham Ely (Dandelion)
  611747,  // Sam Keller
  696969,
  420420,
  616800,  //Jack Maginnes
  434583,  //Jacob Carlton
  636920,  //Ben Washburn
  666298,  //William Stallings
  627887,  //Daniel Schwaner
  631949,  //Ajinkya Sawant
  632382,  //Harley Fuller
  434998,  //Jake Williams
  615876,  //Alex Andon
  626753,  //Jack Topping
  428046,  //Stephen Bracher
  635528,  //Tyler Seladi
  628040,  //Ian Cox
  625061,  //Grant Witten
  635584,  //Jimmy Hankish
  645540,  //Graham Ely
  615718,  //Jay Middleton
  622347,  //Jack Gorman
  626377,  //William Groover
  439777,  //Thomas Langstaff
  611177,  //James Rockhill
  623162,  //Patrick Nolen 
  627030,  //Garrett Metropol 
  436640,  //Hale Porter
  612418,  //Harrison Hornung
  620007,  //Trent Davidson
  629538,  //Sully Anderson
  612035,  //Danny Johnsen
  639424,  //Max Engle
  649710,  //Kenny Sidoryk
  627907,  //Devan Knapp
  615517,  //Steven Butz
  612020,  //Matt Pacifico 
  437725,  //Grant Lancaster
  624885,  //Olivia Lawson
  441091,  //Kenta Yasuda
  429449,  //Wes Crane
  428428,  //Adam Schatz
  643124,  //Tyler Meagher
  645267,  //Philip Koster
  620946,  //Mateo Rodriguez
  610498,  //Jonathon Beaver
  623234,  //Liam Bohannon
  438406,  //Jason Floyd
  613927,  //Quint Heaton
  603902,  //Richie Clayton
  615755,  //PJ Capozzi
  615891,  //Colin O’Mara
  426870,  //Anthony Porcelli
  638974,  //Jonathan Clarke
  629363,  //J Clarke's hoe
  654035,  //Nick Sherrard
  618781,  //Chance Hogan
  451396,  // Russ
  428427,  //Ashley Barre
  615501,  //Liam Byrne 
  656173,  //James E Brown 
  657740,  //Nicholas Munce
  657768,  //CJ Sell
  669820,  //Jacob Waddell
  656138,  //Tommy Willman
  651007,  //Sam Pendergast
  648652,  //Noah Weltlich 
  649085,  //Benjamin Shappard
  655707,  //Jack Paris
  656702,  //Jack Maley
  652877,  //Noah Newell
  647076,  //Brian Buckley
  656412,  //Zachary Bellis
  656446,  //Jackson Grant
  657680,  //Jacob Brechbuhl
  646385,  //Matthew Brown
  647059,  //Robert DuPre
  655628,  //Conor Walsh
  650834,  //Gavin Hornung 
  657549,  //Eric Lee
  651016,  //James Kratzberg
  647209,  //David Knight
  647592,  //Griffin Dominguez
  656348,  //Akash Prasad
  656458,  //Luke Simmons
  627678,  //Theresa Devita
  614026,  //Dayna Grigsby
  621058,  //Riley Geran
  1189,    //Young
  1194,    //Bryan
  1177,    //Jason
  438147,  //Mara Hayes
  669319,  //James Brown
  650484,  //Hugh Hogan
  613477,  //Jacob Rankin
  625964,  //Derek Wood
  627590,  //Jack Darrow
  669942,  //Harley Fuller
};

unsigned long bitchCodes [MAX_BITCH_CODES] = {

};

// interrupt that happens when INTO goes low (0 bit)
void ISR_INT0() {
  //Serial.print("0");   // uncomment this line to display raw binary
  bitCount++;
  flagDone = 0;
  weigand_counter = WEIGAND_WAIT_TIME;
 
}

// interrupt that happens when INT1 goes low (1 bit)
void ISR_INT1() {
  //Serial.print("1");   // uncomment this line to display raw binary
  databits[bitCount] = 1;
  bitCount++;
  flagDone = 0;
  weigand_counter = WEIGAND_WAIT_TIME;  
}

void setup() {
  pinMode(2, INPUT);     // DATA0 (INT0)
  pinMode(3, INPUT);     // DATA1 (INT1)
  pinMode(unlockPin, OUTPUT);    // unlockPin (MOSFET)
  pinMode(sensorPin, INPUT);    // sensorPin (SENSOR INPUT)
  pinMode(buttonPin, INPUT);    // butonPin (BUTTON INPUT)

  digitalWrite(unlockPin, HIGH);
  Serial.begin(9600);
  Serial.println("RFID Readers");

  // binds the ISR functions to the falling edge of INTO and INT1
  attachInterrupt( digitalPinToInterrupt(2), ISR_INT0, FALLING);
  attachInterrupt( digitalPinToInterrupt(3), ISR_INT1, FALLING);


  weigand_counter = WEIGAND_WAIT_TIME;
}

void loop()
{
  // Re-locks door if DOOR_UNLOCK_TIME milliseconds have passed since last unlock
  if (millis() > latestUnlockTime + DOOR_UNLOCK_TIME) {
    digitalWrite(unlockPin, HIGH);
//    Serial.println("Locking");
  // Bitches get less time
  } 
  if (isBitch && (millis() > latestUnlockTime + BITCH_UNLOCK_TIME)) {
    digitalWrite(unlockPin, HIGH);
//    Serial.println("Locking out bitch");
  }
  
  // This waits to make sure that there have been no more data pulses before processing data
  if (!flagDone) {
    if (--weigand_counter == 0)
      flagDone = 1;  
  }

  if (digitalRead(sensorPin) == HIGH) {
    Serial.println("Sensor activated, Opening door");
    openDoor();
  }

  if (digitalRead(buttonPin) == HIGH) {
    Serial.println("Button pressed, Opening door");
    openDoor();
  }

  // if we have bits and we the weigand counter went out
  if (bitCount > 0 && flagDone) {
    getCardAndFacilityCode();

    if (cardCode != 0) {
      // Check card
      if (isValidCode(cardCode)) {
        // Enable lock
        openDoor();
      }
      if (isValidBitch(cardCode)) {
        openDoorBitch();
      }
    }
  
    // cleanup and get ready for the next card
    bitCount = 0;
    facilityCode = 0;
    cardCode = 0;
    for (int i=0; i<MAX_BITS; i++) {
      databits[i] = 0;
    }
  }
}
 
void printBits() {
  Serial.print("FC = ");
  Serial.print(facilityCode);
  Serial.print(", CC = ");
  Serial.println(cardCode);
}

void openDoor() {
  latestUnlockTime = millis();
  digitalWrite(unlockPin, LOW);
  isBitch = false;
}

void openDoorBitch() {
  if ((rand() % 100) + 1 < BITCH_SUCCESS_RATE) {
    latestUnlockTime = millis();
    digitalWrite(unlockPin, LOW);
    isBitch = true;
  } else {
    Serial.println("DENIED bitch: https://youtu.be/RD1KqbDdmuE");
  }
}

bool isValidCode(unsigned long code) {
  for (int i=0; i<MAX_CARD_CODES; i++) {
    if (code == cardCodes[i]) {
      return true;
    }
  }
  return false;
}

bool isValidBitch(unsigned long bitchCode) {
  for (int i=0; i<MAX_BITCH_CODES; i++) {
    if (bitchCode == bitchCodes[i]) {
      return true;
    }
  }
  return false;
}

void getCardAndFacilityCode() {
  unsigned char i;
 
  if (bitCount == 35) {
    // 35 bit HID Corporate 1000 format
    // facility code = bits 2 to 14
    for (i=2; i<14; i++) {
       facilityCode <<=1;
       facilityCode |= databits[i];
    }

    // card code = bits 15 to 34
    for (i=14; i<34; i++) {
       cardCode <<=1;
       cardCode |= databits[i];
    }

    printBits();

  } else if (bitCount == 26) {
    // standard 26 bit format
    // facility code = bits 2 to 9
    for (i=1; i<9; i++) {
       facilityCode <<=1;
       facilityCode |= databits[i];
    }

    // card code = bits 10 to 23
    for (i=9; i<25; i++) {
       cardCode <<=1;
       cardCode |= databits[i];
    }

    printBits();
  } else {
    Serial.println("Incorrect code");
  }
}
