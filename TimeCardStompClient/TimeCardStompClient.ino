#include <Wire.h> // Enable this line if using Arduino Uno, Mega, etc.
#include <SPI.h>

#include "Adafruit_LEDBackpack.h"
#include "Adafruit_GFX.h"
#include "Adafruit_SSD1306.h"
#include "math.h"


int STOMP_START = A5;
int STOMP_SWITCH = A4;
unsigned long currentMilliseconds = 0;
unsigned long switchMillisecondCounter = 0;
unsigned long oledDisplayMillisecondCounter = 0;
bool clockedIn = false;

//communication variables
bool inputComplete = false;
String inputString = "";

//project variables
#define MAX_PROJECT_NUMBER 5
int currentTimeArray [MAX_PROJECT_NUMBER];
String projectArray [MAX_PROJECT_NUMBER];
int projectIdArray [MAX_PROJECT_NUMBER];
int currentProject = 0;

//display variables
Adafruit_7segment matrix = Adafruit_7segment();
#define OLED_MOSI   9
#define OLED_CLK   10
#define OLED_DC    11
#define OLED_CS    12
#define OLED_RESET 13
Adafruit_SSD1306 display(OLED_MOSI, OLED_CLK, OLED_DC, OLED_RESET, OLED_CS);


void setup() {
  pinMode(STOMP_SWITCH, INPUT);
  pinMode(STOMP_START, INPUT);
  digitalWrite(STOMP_SWITCH, HIGH);
  digitalWrite(STOMP_START, HIGH);
  Serial.begin(9600);
  Serial.println("starting up....");
  //7 segment
  matrix.begin(0x70);
  matrix.setBrightness(6);
/*  matrix.writeDigitNum(0,0,false);
  matrix.writeDigitNum(1,0,false);
  matrix.writeDigitRaw(2,0x02);
  matrix.writeDigitNum(3,0,false);
  matrix.writeDigitNum(4,0,false);
*/ 
  matrix.clear();
  matrix.writeDisplay();
  
  //oled
  display.begin(SSD1306_SWITCHCAPVCC);
  display.clearDisplay();
  display.setCursor(0,0);
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.println("");
  display.println("Hello,");
  display.println("SLACKER!");
  display.display();
  
  inputString;
  
  currentMilliseconds = millis();
  for(int x = 0;x<MAX_PROJECT_NUMBER;x++)
  {
    projectArray[x] = "";
  }
  
  //debug! debug! debug!
  //populateDummyData();
}

void handleStompStart()
{
    String oledDisplayString = "";
    if(projectIdArray[currentProject] == 0)
    { //no project loaded
      oledDisplayString = "No projects loaded!";
      oledDisplayMillisecondCounter = millis();
      
    } else
    {
      clockedIn = !clockedIn;
//      currentTimeArray[currentProject] = 0;
      currentMilliseconds = millis();
      oledDisplayString = "Project: " + String(currentProject+1) + "\n\n" + projectArray[currentProject];
      oledDisplayString += (clockedIn)? "\n\nClocked In": "\n\nClocked Out";
      if(!clockedIn)
        oledDisplayMillisecondCounter = millis();
    }
    update7SegmentDisplay();
    updateOLEDDisplay(oledDisplayString, 1);
}

void handleStompSwitch()
{
  currentProject++;

  if(currentProject >= MAX_PROJECT_NUMBER || projectIdArray[currentProject] == 0)
    currentProject=0;
  oledDisplayMillisecondCounter = millis();
  clockedIn = !clockedIn; //it'll get switched back below....
  handleStompStart();
}

void handleInputString()
{
  if(inputString.indexOf('@') < 1)
    return;

  String command = inputString.substring(0,inputString.indexOf('@'));
  Serial.print(inputString.indexOf('@'));
  Serial.print("<--->");
  Serial.print("parsed command:");
  Serial.println(command);
  if(command.equals("syncProject"))
    handleSyncProject(inputString.substring(inputString.indexOf('@')+1, inputString.length() -1));
  else if (command.equals("syncHours"))
    handleSyncHours();
  else if (command.equals("timeWipe"))
    handleTimeWipe();
  else
  {
    resetStatus("Invalid input!"); //need to put in some more sophisticated logic

  }
}

void resetStatus(String message)
{
  currentProject = 0;
  clockedIn = false;
  
  updateOLEDDisplay(message,2);
  update7SegmentDisplay();
  oledDisplayMillisecondCounter = millis();

}

void handleSyncProject(String projectString)
{
  Serial.print("handleSyncProject: ");
  Serial.println(projectString);
  int counter = 0;
  int rollingIndex = 0;
  String projectChunk = projectString.substring(0,projectString.indexOf('@'));
  rollingIndex += projectChunk.length();
  while(projectChunk.length() > 2 && counter < MAX_PROJECT_NUMBER)
  {
    Serial.print("project String:");
    Serial.println(projectString.length());
    Serial.println(projectString);
    Serial.print("projectChunk:");
    Serial.println(projectChunk);
    char projectIdCharArray[projectChunk.indexOf('|') + 1];
    String projectIdChunk = projectChunk.substring(0,projectChunk.indexOf('|'));
    projectIdChunk.replace("@","");
    projectIdChunk.toCharArray(projectIdCharArray, sizeof(projectIdCharArray));
    int projectId = atoi(projectIdCharArray);
    projectIdArray[counter] = projectId;
    String projectName = projectChunk.substring(projectChunk.indexOf('|') + 1, projectChunk.length());
    Serial.print("chunked values: ");
    Serial.print(projectId);
    Serial.print("<--->");
    Serial.println(projectName);
    
    projectIdArray[counter] = projectId;
    projectArray[counter] = projectName;
    currentTimeArray[counter] = 0;
    
    Serial.print("rollingIndex:");
    Serial.println(rollingIndex);
    Serial.println(projectString.indexOf('@',rollingIndex+1));
    projectChunk = projectString.substring(rollingIndex+1,projectString.indexOf('@',rollingIndex+2));
    rollingIndex+=projectChunk.length();
    counter++;
  }
  
  for(int x = counter; x < MAX_PROJECT_NUMBER; x++)
  {
    projectIdArray[x] = 0;
    projectArray[x] = "";
    currentTimeArray[x] = 0;
  }
  resetStatus("Project sync completed!");
}
void handleTimeWipe()
{
  for(int x = 0; x < MAX_PROJECT_NUMBER && projectIdArray[x] != 0;x++)
  {
    currentTimeArray[x] = 0;
  }
  resetStatus("Time wipe completed!");
}

void handleSyncHours()
{
  Serial.println("handleSyncHours");
  for(int x = 0; x < MAX_PROJECT_NUMBER;x++)
  {
    Serial.print(projectIdArray[x]);
    Serial.print("<--->");
    Serial.println(currentTimeArray[x]);
  }
  Serial.println("endHourOutput");
}

void loop() {

  //delay(500);
  
  if (inputComplete) {
    Serial.println("input string received:");
    Serial.println(inputString); 
    // clear the string:
    handleInputString();
    inputString = "";
    inputComplete = false;
  }
  
  if(digitalRead(STOMP_START) == HIGH && ((millis() - currentMilliseconds) > 500))  //give the button 1.5 second delay
  { //detected buttonpress
    Serial.println("button 1 press detected");
    handleStompStart();
  }
  
  if(digitalRead(STOMP_SWITCH) == HIGH && ((millis() - switchMillisecondCounter) > 500))  //give the button 1.5 second delay
  {
    Serial.println("button 2 press detected");
    switchMillisecondCounter = millis();
    handleStompSwitch();
  }

  if(millis() - oledDisplayMillisecondCounter > 15000 && !clockedIn) //drop the display on clock out after 30 seconds
  {
    updateOLEDDisplay("",2);
  }
  if(((millis() - currentMilliseconds) > 60000) && clockedIn)
//  if((millis() - currentMilliseconds) > 1000 && clockedIn) //debug: tick every second
  {
    currentTimeArray[currentProject]++;
    update7SegmentDisplay();   
    currentMilliseconds = millis();
  }
  while (Serial.available()) {
    // get the new byte:
    char inChar = (char)Serial.read(); 
    // add it to the inputString:
    inputString += inChar;
    // if the incoming character is a newline, set a flag
    // so the main loop can do something about it:
    if (inChar == '\n') {
      inputComplete = true;
    } 
  }

  delay(100);
}

void updateOLEDDisplay(String displayString, int textSize)
{

    display.clearDisplay();
    display.setCursor(0,0);
    display.setTextSize(textSize);
    display.println(displayString);
    display.display();
/*  } else 
  {
    Serial.println("oled clockin");
      display.clearDisplay();
      display.setCursor(0,0);
//      display.println("");
      display.println("Clocked");
     display.print("in!");
      display.display();
  }
*/
}

void populateDummyData()
{
 for(int x = 0; x < MAX_PROJECT_NUMBER; x++)
 {
   projectIdArray[x] = x+10;
   projectArray[x] = "Project: " + String(x+1);
 } 
}

void update7SegmentDisplay()
{
  if(!clockedIn)
  {
    matrix.clear();
    matrix.writeDisplay();
    return;
  }
  
  int minutes = currentTimeArray[currentProject] % 60;
  int hours = floor(currentTimeArray[currentProject] / 60);
  //digit 0
  int digit0 = minutes % 10;
  //digit 1
  int digit1 = floor(minutes/10);
  //digit 3 
  int digit3 = hours % 10;
  //digit 4
  int digit4 = floor(hours/10);
  
/*  Serial.print("digit0: ");
  Serial.println(digit0);
  Serial.print("digit1: ");
  Serial.println(digit1);
  */
  matrix.writeDigitRaw(2,0x02);
  matrix.writeDigitNum(4,digit0,false);
  matrix.writeDigitNum(3,digit1,false);
  matrix.writeDigitNum(1,digit3,false);
  matrix.writeDigitNum(0,digit4,false);
  matrix.writeDisplay();

}
