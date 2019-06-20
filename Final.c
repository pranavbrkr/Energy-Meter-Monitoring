#include <EEPROM.h>
#include <SoftwareSerial.h>
#include <LiquidCrystal.h>

#define LDRpin 7  // pin where we connect LDR and resistor
#define RX 10
#define TX 11

String AP = "hqoaneh";       // Wifi Name
String PASS = "asdfghjk"; // Password
String API = "LG9VRE9A47WP6795";   // Write API Key
String HOST = "api.thingspeak.com";
String PORT = "80";
String field = "field1";  //cost
String field2 = "field2"; //unit
int countTrueCommand;
int countTimeCommand; 
boolean found = false; 
double valSensor = 0.0;
int LDRValue = 0;
double unit = 0.0;
double cost = 0.0;
unsigned long PulseCounter = 1;

SoftwareSerial esp8266(RX,TX);
LiquidCrystal lcd(12,9,5,4,3,2);

void setup() {
  Serial.begin(9600); // sets serial port for communication with baud rate
  lcd.begin(16,2);
  lcd.setCursor(3,0);
  lcd.print("WELCOME!!");
  delay(2000);
  lcd.clear();
  lcd.setCursor(1,0);
  lcd.print("CONNECTING TO");
  lcd.setCursor(4,1);
  lcd.print("WiFi!!");
  esp8266.begin(9600); // ESP8266 baud rate
  sendCommand("AT",5,"OK");
  sendCommand("AT+CWMODE=1",5,"OK");
  sendCommand("AT+CWJAP=\""+ AP +"\",\""+ PASS +"\"",20,"OK");
  PulseCounter = EEPROM.read(0);  // Read Previous state units
  lcd.clear();
  lcd.setCursor(3,0);
  lcd.print("CONNECTED");

  delay(3000);
  lcd.clear();
}

void loop() {
  
  valSensor = getSensorData();  // returns unit, when cal blinks
  cost = (double)(valSensor*10.0);  //calculate cost as per current unit
  
  //Send data to thinksboard
  String getData = "GET /update?api_key="+ API +"&"+ field +"="+String(cost,10) +"&"+ field2 +"="+ String(valSensor,10);
  sendCommand("AT+CIPMUX=1",5,"OK");
  sendCommand("AT+CIPSTART=0,\"TCP\",\""+ HOST +"\","+ PORT,15,"OK");
  sendCommand("AT+CIPSEND=0," +String(getData.length()+4),4,">");
  esp8266.println(getData);
  countTrueCommand++;
  sendCommand("AT+CIPCLOSE=0",5,"OK");
  lcd.clear();
  //Display on lcd
  lcd.setCursor(0,0);
  lcd.print("Units:"+String(valSensor,6));
  lcd.setCursor(0,1);
  lcd.print("Cost:"+String(cost,6));

  //Write data to EEPROM in case of failure
  EEPROM.write(0, PulseCounter);
}

float getSensorData(){
  while(1){
    LDRValue = digitalRead(LDRpin);// read the value from the LDR
    if(LDRValue == 0){ //if led blinks
      unit = (double)(0.3125 * PulseCounter++) / 1000.0;
      delay(2000);
      return unit; 
    }
  }
}


void sendCommand(String command, int maxTime, char readReplay[]) {
  Serial.print(countTrueCommand);
  Serial.print(". at command => ");
  Serial.print(command);
  Serial.print(" ");
  while(countTimeCommand < (maxTime*1))
  {
    esp8266.println(command);//at+cipsend
    if(esp8266.find(readReplay))//ok
    {
      found = true;
      break;
    }
  
    countTimeCommand++;
  }
  
  if(found == true)
  {
    Serial.println("SENT");
    countTrueCommand++;
    countTimeCommand = 0;
  }
  
  if(found == false)
  {
    Serial.println("Fail");
    countTrueCommand = 0;
    countTimeCommand = 0;
  }
  
  found = false;
 }