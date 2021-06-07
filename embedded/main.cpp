#include <Arduino.h>
#include <SoftwareSerial.h>
#include <util/delay.h>
#include <Wire.h>
#define timeout 10000
#define TEMP_IN A7
#define RX 10
#define TX 11
SoftwareSerial Serial3 (RX,TX); 

String ssid = "MonaConnect";
String password = "";
String host = "10.22.60.87";
String MAC = "";
String PORT = "5000";
String Cmmnd  = "";
String Post = "";
String Body = "";
int count1;
int count2; 
int item = 0;
unsigned long time_s = 0;
long timer_l;
int gyro_x, gyro_y, gyro_z;
long gyro_x_cal, gyro_y_cal, gyro_z_cal;
bool set_gyro_angles;
long acc_x, acc_y, acc_z, acc_total_vector;
float angle_roll_acc, angle_pitch_acc;
float angle_pitch, angle_roll;
int angle_pitch_buffer, angle_roll_buffer;
float angle_pitch_output, angle_roll_output;
int Temp;
unsigned char wifi = 0; 


void setup() {
  Serial.begin(115200);
   
  LM35();
  gyroSetup();                                        
  setup_esp();
  for(int i = 0; i < 4; i++)
  MAC = getMAC();   

}

void loop(){
  time_s = millis();
  while ((millis() - time_s) < timeout)
  {
  read_mpu_6050_data();   
 //Subtract the offset values from the raw gyro values
  gyro_x -= gyro_x_cal;                                                
  gyro_y -= gyro_y_cal;                                                
  gyro_z -= gyro_z_cal;                                                
         
  //Gyro angle calculations . Note 0.0000611 = 1 / (250Hz x 65.5)
  angle_pitch += gyro_x * 0.0000611;                                   //Calculate the traveled pitch angle and add this to the angle_pitch variable
  angle_roll += gyro_y * 0.0000611;                                    //Calculate the traveled roll angle and add this to the angle_roll variable
  //0.000001066 = 0.0000611 * (3.142(PI) / 180degr) The Arduino sin function is in radians
  angle_pitch += angle_roll * sin(gyro_z * 0.000001066);               //If the IMU has yawed transfer the roll angle to the pitch angel
  angle_roll -= angle_pitch * sin(gyro_z * 0.000001066);               //If the IMU has yawed transfer the pitch angle to the roll angel
  
  //Accelerometer angle calculations
  acc_total_vector = sqrt((acc_x*acc_x)+(acc_y*acc_y)+(acc_z*acc_z));  //Calculate the total accelerometer vector
  //57.296 = 1 / (3.142 / 180) The Arduino asin function is in radians
  angle_pitch_acc = asin((float)acc_y/acc_total_vector)* 57.296;       //Calculate the pitch angle
  angle_roll_acc = asin((float)acc_x/acc_total_vector)* -57.296;       //Calculate the roll angle
  
  angle_pitch_acc -= 0.0;                                              //Accelerometer calibration value for pitch
  angle_roll_acc -= 0.0;                                               //Accelerometer calibration value for roll

  if(set_gyro_angles){                                                 //If the IMU is already started
    angle_pitch = angle_pitch * 0.9996 + angle_pitch_acc * 0.0004;     //Correct the drift of the gyro pitch angle with the accelerometer pitch angle
    angle_roll = angle_roll * 0.9996 + angle_roll_acc * 0.0004;        //Correct the drift of the gyro roll angle with the accelerometer roll angle
  }
  else{                                                                //At first start
    angle_pitch = angle_pitch_acc;                                     //Set the gyro pitch angle equal to the accelerometer pitch angle 
    angle_roll = angle_roll_acc;                                       //Set the gyro roll angle equal to the accelerometer roll angle 
    set_gyro_angles = true;                                            //Set the IMU started flag
  }
  
  //To dampen the pitch and roll angles a complementary filter is used
  angle_pitch_output = angle_pitch_output * 0.9 + angle_pitch * 0.1;   //Take 90% of the output pitch value and add 10% of the raw pitch value
  angle_roll_output = angle_roll_output * 0.9 + angle_roll * 0.1;      //Take 90% of the output roll value and add 10% of the raw roll value

 _delay_us(4000);
  }
  
  sendPost();
}

void setup_mpu_6050_registers()
{
  //Activate the MPU-6050
  Wire.beginTransmission(0x68);                                        //Start communicating with the MPU-6050
  Wire.write(0x6B);                                                    //Send the requested starting register
  Wire.write(0x00);                                                    //Set the requested starting register
  Wire.endTransmission();                                             
  //Configure the accelerometer (+/-8g)
  Wire.beginTransmission(0x68);                                        //Start communicating with the MPU-6050
  Wire.write(0x1C);                                                    //Send the requested starting register
  Wire.write(0x10);                                                    //Set the requested starting register
  Wire.endTransmission();                                             
  //Configure the gyro (500dps full scale)
  Wire.beginTransmission(0x68);                                        //Start communicating with the MPU-6050
  Wire.write(0x1B);                                                    //Send the requested starting register
  Wire.write(0x08);                                                    //Set the requested starting register
  Wire.endTransmission();                                             
}

void read_mpu_6050_data()
{                                             // subroutine for reading the raw gyro and accelerometer data
  Wire.beginTransmission(0x68);                                        //Start communicating with the MPU-6050
  Wire.write(0x3B);                                                    //Send the requested starting register
  Wire.endTransmission();                                              //End the transmission
  Wire.requestFrom(0x68,14);                                           //Request 14 bytes from the MPU-6050
  while(Wire.available() < 14);                                        //Wait until all the bytes are received
  acc_x = Wire.read()<<8|Wire.read();                                  
  acc_y = Wire.read()<<8|Wire.read();                                  
  acc_z = Wire.read()<<8|Wire.read();                                  
  Temp = Wire.read()<<8|Wire.read();                                   
  gyro_x = Wire.read()<<8|Wire.read();                                 
  gyro_y = Wire.read()<<8|Wire.read();                                 
  gyro_z = Wire.read()<<8|Wire.read();                                 
}

int roundoff (float Temp)
{
  boolean neg;
  if (Temp < 0)
  {
     Temp = abs(Temp);
     neg = true;
  }
  else
  {
    neg = false;
  }
  float dp = Temp - (int)Temp;
  if (dp >= 0.5) 
  {
    if (neg) return ((int)Temp+1)*(-1);
    else return (int)Temp+1;
  }
  else 
  {
    if(neg) return ((int) Temp)*(-1);
    else return (int) Temp;
  }
}

float getTemp()
{
  int ADCin = analogRead(TEMP_IN);
  return ((ADCin/1024.0)*5)/0.01;
}

String getMAC()
 {
    
    Serial3.println("AT+CIPSTAMAC?");
    int sizee =  Serial3.available();
    char response1[sizee];
    String response = "";
    String MAC = "";
    for (int x = 0; x <sizee; x++)
    {
      response1[x] = Serial3.read();
      response+= response1[x];
    }
    
    int x = response.indexOf('"');
    int from = x+1;
    
    for(int i = x; i < (response.indexOf('"', from))+1; i++)
    {
    MAC += response1[i];
    }
    delay(400);
    return MAC;
 }

void setup_esp()
{
  Serial3.begin(115200);
  sendCommand("AT",5,"OK"); 
  sendCommand("AT+CWMODE=1",5,"OK"); 
  if(sendCommand("AT+CWJAP=\""+ ssid +"\",\""+ password +"\"",20,"OK")) wifi = 1;
  else wifi = 0;

}

void gyroSetup()
{
    Wire.begin();                                                       
  setup_mpu_6050_registers();                                          
  for (int cal_int = 0; cal_int < 1000 ; cal_int ++){                 
    read_mpu_6050_data();                                             
    gyro_x_cal += gyro_x;                                              
    gyro_y_cal += gyro_y;                                              
    gyro_z_cal += gyro_z;                                              //Add the gyro z offset to the gyro_z_cal variable
    delay(3);                                                          //Delay 3us to have 250Hz for-loop
  }

  
  gyro_x_cal /= 1000;                                                 
  gyro_y_cal /= 1000;                                                 
  gyro_z_cal /= 1000;   
}

void LM35()
{

  pinMode(TEMP_IN, INPUT);
}

void sendPost() 
{
    sendCommand("AT+CIPSTART=\"TCP\",\""+ host +"\"," + PORT,15,"OK");
    Body="";
    Body+= "{";
    Body += "\"patient_id\":"+MAC+",";
    Body+= "\"position\":"+ String(roundoff(angle_pitch_output)) +",";
    Body+= "\"temperature\":"+String(roundoff(getTemp()));
    Body+= "}";
    Post="";
    Post = "Post /api/record HTTP/1.1\r\nHost: ";
    Post += host;
    Post += "\r\nContent-Type: application/json\r\nContent-Length:";
    Post += Body.length();
    Post += "\r\n\r\n";
    Post += Body;
    Post += "\r\n";
    Cmmnd = "AT+CIPSEND=";
    Cmmnd+= String(Post.length());
    sendCommand(Cmmnd, 10, "OK");
    sendCommand(Post, 15,"OK");
    sendCommand("AT+CIPCLOSE=0", 10, "OK");
}

int sendCommand(String Cmmnd, int maxTime, char readReply[]) 
{
  Serial.flush();
  Serial.print(count1);
  
  Serial.print(". at Cmmnd => ");
  Serial.print(Cmmnd);
  Serial.print(" ");
  item = 0;
  while(count2 < (maxTime*1))
  {
    Serial3.println(Cmmnd); 
    if(Serial3.find(readReply))
    {
      item = 1;
      break;
    }
  
    count2++;
  }
  
  if(item == 1)
  {
    Serial.println("OK");
    count1++;
    count2 = 0;
  }
  
  if(item == 0)
  {
    Serial.println("Fail");
    count1 = 0;
    count2 = 0;
  }

  return item;
 }

