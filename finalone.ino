#include <SoftwareSerial.h>
#define RX 10
#define TX 11

#define         MQ_PIN                       (0)     
#define         RL_VALUE                     (4.9)     
#define         RO_CLEAN_AIR_FACTOR          (9.83)  
#define         CALIBARAION_SAMPLE_TIMES     (50) 
#define         CALIBRATION_SAMPLE_INTERVAL  (500)
#define         READ_SAMPLE_INTERVAL         (50) 
#define         READ_SAMPLE_TIMES            (5)   
                                                  
#define         GAS_LPG                      (0)
#define         GAS_CO                       (1)
#define         GAS_SMOKE                    (2)

float           LPGCurve[3]  =  {2.3,0.21,-0.47};   
                                                   
                                                    
                                                    
float           COCurve[3]  =  {2.3,0.72,-0.34};    
                                                    
                                                   
                                                    
float           SmokeCurve[3] ={2.3,0.53,-0.44};    
                                                    
                                                   
                                                                                                        
float           Ro           =  10;    

String AP = "harshit";       
String PASS = "123456789"; 
String API = "8M7DHM7DO7SYE5YS";
String HOST = "api.thingspeak.com";
String PORT = "80";
int countTrueCommand;
int countTimeCommand; 
boolean found = false; 
SoftwareSerial esp8266(RX,TX); 
void setup() {
  Serial.begin(9600);
  esp8266.begin(115200);
  Serial.print("Calibrating...\n");                
  Ro = MQCalibration(MQ_PIN);                                           
  Serial.print("Calibration is done...\n"); 
  Serial.print("Ro=");
  Serial.print(Ro);
  Serial.print("kohm");
  Serial.print("\n");
  sendCommand("AT",5,"OK");
  sendCommand("AT+CWMODE=1",5,"OK");
  sendCommand("AT+CWJAP=\""+ AP +"\",\""+ PASS +"\"",20,"OK");
  
  
}
int count=0;
void loop()
{
  if(count>10)
  {
    sendCommand("AT",5,"OK");
    sendCommand("AT+CWMODE=1",5,"OK");
    sendCommand("AT+CWJAP=\""+ AP +"\",\""+ PASS +"\"",20,"OK");
    count=0;
  }
   Serial.print("LPG:");
   float lpg=MQGetGasPercentage(MQRead(MQ_PIN)/Ro,GAS_LPG);
   Serial.print(lpg );
   Serial.print( "ppm" );
   Serial.print("    ");   
   Serial.print("CO:"); 
   float co=MQGetGasPercentage(MQRead(MQ_PIN)/Ro,GAS_CO) ;
   Serial.print(co);
   Serial.print( "ppm" );
   Serial.print("    ");   
   Serial.print("SMOKE:");
   float smoke= MQGetGasPercentage(MQRead(MQ_PIN)/Ro,GAS_SMOKE) ;
   Serial.print(smoke);
   Serial.print( "ppm" );
   Serial.print("\n");
String getData1 = "GET /update?api_key="+ API +"&"+ "field1" +"="+String(lpg);
 sendCommand("AT+CIPMUX=1",5,"OK");
 sendCommand("AT+CIPSTART=0,\"TCP\",\""+ HOST +"\","+ PORT,15,"OK");
 sendCommand("AT+CIPSEND=0," +String(getData1.length()+4),8,">");
 esp8266.println(getData1);
 delay(20000);
 countTrueCommand++;
 sendCommand("AT+CIPCLOSE=0",5,"OK");
 
 String getData2 = "GET /update?api_key="+ API +"&"+ "field2" +"="+String(co);
 sendCommand("AT+CIPMUX=1",5,"OK");
 sendCommand("AT+CIPSTART=0,\"TCP\",\""+ HOST +"\","+ PORT,15,"OK");
 sendCommand("AT+CIPSEND=0," +String(getData1.length()+4),8,">");
 esp8266.println(getData2);
 delay(20000);
 countTrueCommand++;
 sendCommand("AT+CIPCLOSE=0",5,"OK");

 String getData3 = "GET /update?api_key="+ API +"&"+ "field3" +"="+String(smoke);
 sendCommand("AT+CIPMUX=1",5,"OK");
 sendCommand("AT+CIPSTART=0,\"TCP\",\""+ HOST +"\","+ PORT,15,"OK");
 sendCommand("AT+CIPSEND=0," +String(getData1.length()+4),8,">");
 esp8266.println(getData3);
 delay(3000);
 countTrueCommand++;
 sendCommand("AT+CIPCLOSE=0",5,"OK");
}
void sendCommand(String command, int maxTime, char readReplay[]) {
  Serial.print(countTrueCommand);
  Serial.print(". at command => ");
  Serial.print(command);
  Serial.print(" ");
  while(countTimeCommand < (maxTime*1))
  {
    esp8266.println(command);
    if(esp8266.find(readReplay))
    {
      found = true;
      break;
    }
  
    countTimeCommand++;
  }
  
  if(found == true)
  {
    Serial.println("OYI");
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

 float MQResistanceCalculation(int raw_adc)
{
  return ( ((float)RL_VALUE*(1023-raw_adc)/raw_adc));
}

float MQCalibration(int mq_pin)
{
  int i;
  float val=0;

  for (i=0;i<CALIBARAION_SAMPLE_TIMES;i++) {            
    val += MQResistanceCalculation(analogRead(mq_pin));
    delay(CALIBRATION_SAMPLE_INTERVAL);
  }
  val = val/CALIBARAION_SAMPLE_TIMES;                   

  val = val/RO_CLEAN_AIR_FACTOR;                         
                                                         

  return val; 
}

float MQRead(int mq_pin)
{
  int i;
  float rs=0;

  for (i=0;i<READ_SAMPLE_TIMES;i++) {
    rs += MQResistanceCalculation(analogRead(mq_pin));
    delay(READ_SAMPLE_INTERVAL);
  }

  rs = rs/READ_SAMPLE_TIMES;

  return rs;  
}

int  MQGetPercentage(float rs_ro_ratio, float *pcurve)
{
  return (pow(10,( ((log(rs_ro_ratio)-pcurve[1])/pcurve[2]) + pcurve[0])));
}

int MQGetGasPercentage(float rs_ro_ratio, int gas_id)
{
  if ( gas_id == GAS_LPG ) {
     return MQGetPercentage(rs_ro_ratio,LPGCurve);
  } else if ( gas_id == GAS_CO ) {
     return MQGetPercentage(rs_ro_ratio,COCurve);
  } else if ( gas_id == GAS_SMOKE ) {
     return MQGetPercentage(rs_ro_ratio,SmokeCurve);
  }    

  return 0;
}


 
