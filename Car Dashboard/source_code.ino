#include<SD.h>
#include<SPI.h>
#include <TFT_HX8357.h>   // Hardware-specific library
#include <TinyGPS.h>      //Library for GPS


TFT_HX8357 tft = TFT_HX8357(); // Invoke custom library


#define BU_BMP 1 // Temporarily flip the TFT coords for standard Bottom-Up bit maps
#define TD_BMP 0 // Draw inverted Top-Down bitmaps in standard coord frame

//Colors Defined for TFT LCD
#define LTBLUE    0xB6DF
#define LTTEAL    0xBF5F
#define LTGREEN   0xBFF7
#define LTCYAN    0xC7FF
#define LTRED     0xFD34
#define LTMAGENTA 0xFD5F
#define LTYELLOW  0xFFF8
#define LTORANGE  0xFE73
#define LTPINK    0xFDDF
#define LTPURPLE  0xCCFF
#define LTGREY    0xE71C

#define BLUE      0x001F
#define TEAL      0x0438
#define GREEN     0x07E0
#define CYAN      0x07FF
#define RED       0xF800
#define MAGENTA   0xF81F
#define YELLOW    0xFFE0
#define ORANGE    0xFC00
#define PINK      0xF81F
#define PURPLE    0x8010
#define GREY      0xC618
#define WHITE     0xFFFF
#define BLACK     0x0000

#define DKBLUE    0x000D
#define DKTEAL    0x020C
#define DKGREEN   0x03E0
#define DKCYAN    0x03EF
#define DKRED     0x6000
#define DKMAGENTA 0x8008
#define DKYELLOW  0x8400
#define DKORANGE  0x8200
#define DKPINK    0x9009
#define DKPURPLE  0x4010
#define DKGREY    0x4A49

//WIFI VARIABLES
#define DST_HOST "213.186.33.19"
String devid="vAFCC5109D44ADC6";

//GPS VARIABLES
TinyGPS gps;
float   lat;
float   lon;
float   gps_speed;
float   gps_speedold=0;
String  gps_clock;
int     year;
byte    month, day, hour, minute, second, hundredths;
unsigned long age;

//Lap Counter Variables
int   lap=0;
int   lap_flag=0;
int   lap_initialize=0;
int   lap_first=0;
float lat_initial;
float lon_initial;
float lat_old;
float lon_old;

//Distance Variables

float distance=0;
float lap_distance=0;
float lap_reset=0;
float per_distance=0;
float avg_distance=0;
float avg_speed=0;
float lapspeed_average=0;
unsigned long lap_start=0;
unsigned long time_old=0;;
unsigned long server_update=0;
unsigned long disp=0;

//SD Variables
File Jat;
int sd_check=1;
String sd_data;

//Button Variable
int button=13;
int but_state=0;
int sel=1;

void setup() 

{
    tft.init();               // Initialisation of LCD
    tft.fillScreen(TFT_BLACK);
    pinMode(12,OUTPUT);
    digitalWrite(12,LOW);
    pinMode(12,INPUT);
  while(digitalRead(12)==0) 
  {
    delay(1000);
  }
  SD.begin(53);
  Serial1.begin(9600);                      //Begins GPS Communication
  lcd_setup();
  bootup();
  time_old=millis();
  but_state=digitalRead(12);
 }

void loop() 
{
 gps_function();
 print_date(gps); 
 lap_counter();
 distance_function();
 data_log(); 
 if(digitalRead(12)!=but_state)
  {
     but_state=digitalRead(12);
     if(sel==4)
      { 
       sel=1;
      }
     else
     { 
      sel++;
     } 
  }  
    switch(sel)
     {
    
      case 1 :
             prints();
             break;

      case 2 :
             print_speed();
             break;
           
      case 3 :
             print_time();
             break;
           
      case 4 :
            print_dis();
            break;
     }  
  if(millis()-server_update>14000)
  {
    delay(200);
    google();
    server_update=millis();
    
  }
  delay(600);  
}

void lcd_setup()
{    
  tft.setRotation(1);
  tft.fillScreen(TFT_WHITE);
  tft.setTextSize(1);
  tft.setTextFont(4);
  tft.setTextColor(0x0000); 
  tft.setCursor (190, 60);
  drawBMP("untitled.bmp", 90, 20, BU_BMP); //.bmp file should be in code file directory.
  delay(3000);
  tft.setRotation(1);
  tft.fillScreen(0x07FF);
  tft.print("WELCOME"); 
  tft.setTextSize(2); 
  tft.setCursor (149,135);        
  tft.print("JATAYU");
  tft.setTextSize(1);
  tft.setCursor (135, 235);        
  tft.print("Thrive Till The End !");
  delay(3000);
}

void bootup()
{
  tft.fillScreen(TFT_BLACK);
  tft.setTextSize(1);
  tft.setTextFont(4);
  tft.setTextColor(TFT_WHITE); 
  tft.setCursor(150,20);
  tft.print("BOOTING UP..");
  tft.setCursor(100,95);
  tft.print("SD");
  tft.setCursor(100,180);
  tft.print("WIFI");
  tft.setCursor(100,260);
  tft.print("GPS");

  sd_setup();
  wifi_setup();
  gps_setup();

  delay(2000);
} 
    
void sd_setup()
{
 
 Jat=SD.open("MAIT.csv",FILE_WRITE);
 if(Jat)
   {
    tft.setCursor(250,95);
    Jat.print(" Time, Latitude, Longitude, Lap, Lap Length(Km), Distance(Km), Lap Travel(Km), Speed(Km/hr), Average Lap Speed(Km/hr)");
    tft.print("Okay");
    Jat.close();
   }
 else
   {
    tft.setCursor(250,95);
    tft.print("Error");
   }

}

void wifi_setup()
{
 Serial2.begin(115200);
 String ssid ="MBLAZE-EC315-EFC8"; // Wifi network Name and password
 String password="80C77D93";
 Serial2.println("AT+RST");      //To Reset Module        
 if(Serial2.find("CONNECTED"))
   {
    }  
  else;
 delay(4000);   
 String cmd ="AT+CWJAP=\"" + ssid + "\",\"" + password + "\"";  
 Serial2.println(cmd);     //To Setup wifi   
 if(Serial2.find("CONNECTED"))   
  {  
    tft.setCursor(250,180);
    tft.print("Okay");
  }
 else
  {
    tft.setCursor(250,180);
    tft.print("Error");
  }
 Serial2.println("AT+CIPMUX=0");         //To setup a single link
 delay(3000);
}

void gps_setup()
{
 gps_function();
  if(lat!=1000)
  {
    tft.setCursor(250,260);
    tft.print("Okay");
  }

  else
   {
    tft.setCursor(250,260);
    tft.print("Connecting"); 
   }
}

void gps_function()
{  
 smartdelay(600);
 gps.f_get_position(&lat, &lon, &age);       
}

//To Read GPS Data
static void smartdelay(unsigned long ms)
{
  unsigned long start = millis();
  do 
  {
    while (Serial1.available())
      gps.encode(Serial1.read());
  }while (millis() - start < ms);
}

//Date Funtion Of GPS
void print_date(TinyGPS &gps)
{
  gps.crack_datetime(&year, &month, &day, &hour, &minute, &second, &hundredths, &age);
  if (age == TinyGPS::GPS_INVALID_AGE)
    { 
      gps_clock="Connecting";
    }
   else
  {  
   gps_speed=gps.f_speed_kmph();   
   if(lap_initialize==1)
   {
     if(abs(TinyGPS::distance_between(lat,lon,lat_old,lon_old))<100)
      {
        distance = distance+(abs(TinyGPS::distance_between(lat,lon,lat_old,lon_old)))/1000;
        lat_old = lat;
        lon_old = lon;
      }
   }
   if(lap_initialize==0)
   { 
      lap_initialize=1;
      lat_initial=lat;
      lon_initial=lon;
      lat_old=lat;
      lon_old=lon; 
    }
 
   if(minute<30)
      {
        if(second<10)
        {
          gps_clock=String(hour+5)+":"+String(minute+30)+":"+"0"+String(second);    
        }
        
        else
        {
         gps_clock=String(hour+5)+":"+String(minute+30)+":"+String(second);    
        }
      }
      
    else
      {
        if((minute-30)<10)
        {
          if(second<10)
          {
            gps_clock=String(hour+6)+":"+"0"+String(minute-30)+":"+"0"+String(second);
          }
          else
          {
            gps_clock=String(hour+6)+":"+"0"+String(minute-30)+":"+String(second);
          }
         }
        else
        {
          if(second<10)
         {
           gps_clock=String(hour+6)+":"+String(minute-30)+":"+"0"+String(second);        
         } 
         else
         {
         gps_clock=String(hour+6)+":"+String(minute-30)+":"+String(second);        
         }
        }
        
      }
  }
     
}






//Lap Counter


void lap_counter()
{
  
  if(lap_initialize==1)
  {
    
    if((abs(TinyGPS::distance_between(lat,lon,lat_initial,lon_initial))<10)&&(lap_flag==1))
      {
       
        lap=lap+1;
    
        lapspeed_average=avg_speed;
    
        lap_reset=distance;
    
        avg_distance=distance/lap;
    
        lap_flag=0;
    
        lap_start=millis();
  
       }

      if((abs(TinyGPS::distance_between(lat,lon,lat_initial,lon_initial))>100)&&(lap_flag==0))
        {
         lap_flag=1;
        }

      }

}






//Lap Distance Function

void distance_function()

 {

   lap_distance=distance-lap_reset;
  
   per_distance=100*lap_distance/avg_distance;
  
   avg_speed=3600000*lap_distance/(millis()-lap_start);

 }






void data_log()
{
  Jat=SD.open("MAIT.csv",FILE_WRITE);
 
  if(Jat)
   {
     sd_data =  String(gps_clock) + "," + String(lat,6) + "," + String(lon,6) + "," + String(lap) + "," + String(avg_distance) + "," + String(distance) + "," + String(lap_distance) + "," + String(gps_speed) + "," + String(avg_speed);
    
     Jat.println(sd_data);
    Jat.close();
    sd_check=1;
   }

  else
   {
     sd_check=0; 
   }

}






//Google Data Update


void google()
{
  
  String cmd = "AT+CIPSTART=\"TCP\",\"";
  cmd += DST_HOST;
  cmd += "\",80";

  
  Serial2.println(cmd);

  delay(500);
  
  cmd =  "GET /pushingbox?devid=";
  cmd += devid;
  cmd += "&lat="+String(lat,6)+"&lon="+String(lon,6)+"&lap="+String(lap)+"&lal="+String(avg_distance)+"&dis="+String(distance)+"&lac="+String(lap_distance)+"&spd="+String(gps_speed)+"&avg="+String(avg_speed)+" HTTP/1.1\r\n";     //construct http GET request
  cmd += "Host: api.pushingbox.com\r\n\r\n"; 

  String str= "AT+CIPSEND=";
  str += String(cmd.length());   
  Serial2.println(str);
  
  delay(500);

   Serial2.println(cmd);

}






void prints()
{
  
  tft.fillScreen(0x07FF);
  
    if(lap==0)
  {
    
    tft.setTextSize(1);
    tft.setTextFont(4);
    tft.setTextColor(TFT_BLACK); 
    tft.setCursor (25, 25);
    tft.print("dis  ");
    tft.print(" -");
    tft.setCursor (360, 25);
    tft.print("Lap   ");
    tft.print(lap);
    tft.setCursor (170, 45);
    tft.print(gps_clock);
    tft.setTextSize(4);
    tft.setCursor (110, 100);
    tft.print(gps_speed);
    tft.setTextSize(1);
    tft.setCursor (345, 150);
    tft.print(" Km/Hr");
    tft.setCursor (195, 200);
    tft.print(distance);
    tft.print(" km"); 
    tft.setCursor (28, 260);
    tft.print(" -");
    tft.print("  % ");
    if(sd_check==0)
     {
      tft.setCursor (220, 260);
      tft.print("***");
     }
    tft.setCursor (330, 240);
    tft.print("Avg  ");
    tft.print(avg_speed);
    tft.setCursor (330, 280);
    tft.print("Lav  ");
    tft.print(" -");
     
  }
    else
   {
    tft.setTextSize(1);
    tft.setTextFont(4);
    tft.setTextColor(TFT_BLACK); 
    tft.setCursor (25, 25);
    tft.print("dis  ");
    tft.print(avg_distance);
    tft.setCursor (360, 25);
    tft.print("Lap   ");
    tft.print(lap);
    tft.setCursor (180, 45);
    tft.print(gps_clock);
    tft.setTextSize(4);
    tft.setCursor (110, 100);
    tft.print(gps_speed);
    tft.setTextSize(1);
    tft.setCursor (345, 150);
    tft.print(" Km/Hr");
    tft.setCursor (195, 200);
    tft.print(distance);
    tft.print(" km"); 
    tft.setCursor (28, 260);
    tft.print(per_distance);
    tft.print("  % ");
    if(sd_check==0)
     {
      tft.setCursor (220, 260);
      tft.print("***");
     }
    tft.setCursor (330, 240);
    tft.print("Avg  ");
    tft.print(avg_speed);
    tft.setCursor (330, 280);
    tft.print("Lav  ");
    tft.print(lapspeed_average);
    
}

}






void print_speed()
{
    tft.fillScreen(0x07FF);
    tft.setTextSize(4);
    tft.setTextFont(4);
    tft.setTextColor(TFT_BLACK); 
    tft.setCursor (140, 105);
    tft.print(gps_speed);
    tft.setTextSize(2);
    tft.setCursor (170, 250);
    tft.print("Km/Hr");
    
}






void print_time()
{
    tft.fillScreen(0x07FF);
    tft.setTextSize(4);
    tft.setTextFont(4);
    tft.setTextColor(TFT_BLACK); 
    tft.setCursor (50, 105);
    tft.print(gps_clock);
    
}






void print_dis()
{
    tft.fillScreen(0x07FF);
    tft.setTextSize(4);
    tft.setTextFont(4);
    tft.setTextColor(TFT_BLACK); 
    tft.setCursor (140, 105);
    tft.print(distance);
    tft.setTextSize(2);
    tft.setCursor (200, 250);
    tft.print("Km");
    
}


