#include <math.h>
#include <XBee.h>

XBee xbee = XBee();
uint8_t payload[]={2,0,0,100,0,0,110,100,0} ;

uint8_t payload_1[]={1,0,45,97,1,0,45,70,2} ;              

// SH + SL Address of receiving XBee
XBeeAddress64 addr64 = XBeeAddress64(0x00000000, 0x0000ffff);
ZBTxRequest zbTx = ZBTxRequest(addr64, payload, sizeof(payload));
ZBTxRequest zbTx_1 = ZBTxRequest(addr64, payload_1, sizeof(payload_1));
ZBTxStatusResponse txStatus = ZBTxStatusResponse();

void setup() 
{
  Serial.begin(9600);
  xbee.setSerial(Serial);
  //delay(1000);
  }
int q = sizeof(payload);
int r = sizeof(payload_1);
//uint8_t copy[100];
float a,b,d,p,t,a_n,d_n;
float x = 5.25;

void correction()
{
  for(int i=1; i<q; i+=4)  {
    float a=payload[i+1];                                               //*PI/180;      
    //float a = 90.0;
    float d=payload[i+2];
    float b=(atan2(((d*sin(a*PI/180))-x),(d*cos(a*PI/180))))*180/PI;
   // Serial.println(b);
    float p=pow((((pow((d*sin(a*PI/180)-x),2))+pow((d*cos(a*PI/180)),2))),0.5);
    //Serial.print(p);
    float t=asin(x/p)*180/PI;
   // Serial.println(t);
    int a_n=t+b;
   // Serial.println(a_n);
    int d_n=p*cos(t*PI/180);
    //Serial.print(d_n);
    payload[i+1]=a_n;
    payload[i+2]=d_n;
   // Serial.println(payload[i+1]);
}
 for(int i=1; i<q; i+=4)  {
    float a=payload_1[i+1];                                               //*PI/180;      
    //float a = 90.0;
    float d=payload_1[i+2];
    float b=(atan2(((d*sin(a*PI/180))-x),(d*cos(a*PI/180))))*180/PI;
   // Serial.println(b);
    float p=pow((((pow((d*sin(a*PI/180)-x),2))+pow((d*cos(a*PI/180)),2))),0.5);
    //Serial.print(p);
    float t=asin(x/p)*180/PI;
   // Serial.println(t);
    int a_n=t+b;
   // Serial.println(a_n);
    int d_n=p*cos(t*PI/180);
    //Serial.print(d_n);
    payload_1[i+1]=a_n;
    payload_1[i+2]=d_n;
   // Serial.println(payload[i+1]);
}
  }
  
void loop() 
{  
correction();
//Serial.println(q);
//delay(1500);
for(int s=1; s<q; s++)
   Serial.println(payload[s]);
   Serial.println(".......");
   for(int s=1; s<r; s++)
   Serial.println(payload_1[s]);
   
xbee.send(zbTx);
delay(1500);
xbee.send(zbTx_1);
while(1);
}
