#include <XBee.h>
#include<math.h>
#include<Servo.h>

XBee xbee = XBee();

//setup for receiving
XBeeResponse response = XBeeResponse(); 
ZBRxResponse rx = ZBRxResponse();
ModemStatusResponse msr = ModemStatusResponse();

#define encoder1 8
#define encoder2 12

//motor master
#define in1 3
#define in2 5

//motor slave
#define in3 11
#define in4 6

volatile int count1 = 0;
volatile int count2 = 0;
volatile byte portBstatus = 0xFF;

void right( float m_power );
void left( float m_power );
void forward( float m_power, float s_power );
void motor_off();
void turn_angle( float angle , float m_power );
void drive_distance( int distance, float m_power );
void grab();
void lift_up();
void drop_down();
void Release();

Servo myservo1,myservo2; //myservo1 is lift-drop action 
                        //myservo2 is  grab-release action

int pos = 0;

void setup() {
  // put your setup code here, to run once:
  PCMSK0 |= bit(PCINT0);
  PCMSK0 |= bit(PCINT4);
  PCIFR |= bit(PCIF0);
  PCICR |= bit(PCIE0);
  
  pinMode(in1,OUTPUT);
  pinMode(in2,OUTPUT);
  pinMode(in3,OUTPUT);
  pinMode(in4,OUTPUT);

  pinMode(encoder1,INPUT);
  digitalWrite(encoder1,HIGH);
  pinMode(encoder2,INPUT);
  digitalWrite(encoder2,HIGH);

  myservo1.attach(9);
  myservo2.attach(10);
  myservo1.write(0);
  myservo2.write(0);
  
  Serial.begin(9600);
  xbee.setSerial(Serial);
}

void loop() {
   xbee.readPacket(500);
  if(xbee.getResponse().isAvailable()){
    Serial.println("got something");
    if((xbee.getResponse().getFrameData()[11]== 1)){ 
       for(int s=12; s< xbee.getResponse().getFrameDataLength(); s+=4){
       int a=xbee.getResponse().getFrameData()[s+1];
       if(xbee.getResponse().getFrameData()[s]==1)
       a=0-a;
       int b=xbee.getResponse().getFrameData()[s+2];
       int c=xbee.getResponse().getFrameData()[s+3];
       Serial.println(a);
       Serial.println(b);
       Serial.println(c);
       turn_angle(a,100);
       delay(500);
       drive_distance(b-16,100);
       delay(500);
       if (c==1){
        grab();
        delay(500);
        lift_up();
       }
       if(c==2){
       drop_down();
       delay(500);
       Release();
     }
     delay(500);
     drive_distance(16,100);
      }
  
   }
   }}

ISR (PCINT0_vect)
{
  uint8_t change;

  change = PINB ^ portBstatus;
  portBstatus = PINB;
  //check if this works
  if( (change & (1<<PB0)) && (change & (1<<PB4)))
  {
    count1+=1;
    count2+=1;
    Serial.print("BOTH");
  }
  if( (change & (1<<PB0)) && !(change & (1<<PB4)))
  count1+=1;
  if( (change & (1<<PB4)) && !(change & (1<<PB0)))
  count2+=1;
}

void turn_angle( float angle , float m_power )
{
  count1 = 0;
  count2 = 0;

  //thin wheel
  //int fullrotation = 118;
  //wide wheel 
  //int fullrotation = 133;
  //micro torque motor 
  int fullrotation = 196;
  
  int total_count = int((angle*fullrotation)/360);
  if(angle>180)
  {
    total_count = (fullrotation-1) - total_count;
    total_count = max(total_count,0);  
  }
  
  while( (count1 <= total_count) && (count2 <= total_count))
  {
    if(angle<=180)
    {
      right(m_power);
    }
    if(angle>180)
    {
      left(m_power);
    }
    delay(20);

    Serial.print(m_power);
    Serial.print(" ");
    //Serial.print(s_power);
    Serial.print(" ");
    Serial.print(count1);
    Serial.print(" ");
    Serial.println(count2);
  }
  motor_off();  
}

void right( float m_power )
{
  analogWrite(in1,m_power);
  analogWrite(in3,0);
  analogWrite(in2,0);
  analogWrite(in4,0);
    
}

void left( float m_power )
{
  analogWrite(in1,0);
  analogWrite(in3,m_power);
  analogWrite(in2,0);
  analogWrite(in4,0);  
}

void motor_off()
{
  analogWrite( in1, 0);
  analogWrite( in3, 0);
  analogWrite( in2, 0);
  analogWrite( in4, 0);  
}
//input parameters: distance(cms) and pwm speed
void drive_distance( int distance, float m_power )
{
  count1 = 0;
  count2 = 0;
  
  int total_count = int((40*distance)/13.95);
  //int totalTicks = 0;
  //wide wheels circumference : 19.16315
  //thin wheels circumference : 25
  //micro torque motor wheels : 13.95
  Serial.println( total_count );
  float s_power = m_power;// + m_power/20.0;
  //Serial.println( s_power);
  
  //grey car = 1.05,1.3, yellow car = 1,1.4
  float Kp = 1.05;
  float Ki = 0.0;
  float Kd = 1.3  ;//0.25;
  int error = 0;
  int last_error = 0;
  int DError = 0;
  int tot_error = 0;
  
  //check this condition
  while( (count1<=total_count) || (count2<=total_count) )
  //while( totalTicks<=total_count )
  {
//    if(count1>total_count)
//    {
//      m_power = 0;
//    }
//    if(count2>total_count)
//    {
//      s_power = 0;
//    }
    forward(m_power,s_power);
    error = count1-count2;
    DError = error - last_error;
    last_error = error;
    tot_error += error; 
    Serial.println(error);
    
    s_power += (Kp*error) + (Ki*tot_error) + (Kd*DError);
    s_power = min(max(0,s_power),255);
    //count1 = 0;
    //count2 = 0;
    delay(100);
    //totalTicks+=count1;

    Serial.print(m_power);
    Serial.print(" ");
    Serial.print(s_power);
    Serial.print(" ");
    Serial.print(count1);
    Serial.print(" ");
    Serial.println(count2);
  }
  motor_off();
  Serial.println(tot_error);
}


void forward ( float m_power , float s_power )
{
  analogWrite( in1, m_power );
  analogWrite( in3, s_power );
  analogWrite( in2, 0 );
  analogWrite( in4, 0 );   
}

void grab()
{
  Serial.println("GRAB");
  for(pos = 0; pos <= 45; pos += 1)  // goes from 0 degrees to 180 degrees 
  {                                  // in steps of 1 degree 
    myservo2.write(pos);              // tell servo to go to position in variable 'pos' 
    delay(30);                       // waits 15ms for the servo to reach the position 
  }  
}

void lift_up()
{
  Serial.println("LIFT UP");
  for(pos = 0; pos <= 50; pos += 1)   
  {                                 
    myservo1.write(pos);            
    delay(20);                      
  }  
}

void drop_down()
{
  Serial.println("DROP DOWN");
  for(pos = 50; pos >= 0; pos -= 1)   
  {                                 
    myservo1.write(pos);            
    delay(20);                      
  }  
}

void Release()
{
  Serial.println("RELEASE");
  for(pos = 30; pos>=0; pos-=1)     // goes from 180 degrees to 0 degrees 
  {                                
    myservo2.write(pos);              // tell servo to go to position in variable 'pos' 
    delay(30);                       // waits 15ms for the servo to reach the position 
  }  
}


