//INCLUIMOS LAS LIBRERIAS PERTINENTES
#include <Wire.h>//Incluimos libreria de comunicacion SCL, SDA
#include <RTC.h>//Incluimos libreria del modulo RTC
#include <Adafruit_GFX.h>    // Core graphics library
#include <MCUFRIEND_kbv.h>   // Hardware-specific library
MCUFRIEND_kbv lcd;
0 2
#include <Fonts/FreeSans9pt7b.h>
#include <Fonts/FreeSans12pt7b.h>
#include <Fonts/FreeSerif12pt7b.h>

#include <FreeDefaultFonts.h>

#include "avr/io.h"
#include "avr/interrupt.h"

#include "Arduino.h"
#include "DFRobotDFPlayerMini.h"
#define com_mp3_ardu Serial2//VERIFICAR SIEMPRE HAY ARDUS QUE ESTAN POR EL TX1 RX1 Y UNO QUE ESTÁ POR EL TX2 Y RX2

DFRobotDFPlayerMini audios_casilla;

//FOR INTERRUPT BY BILL

volatile boolean first_bill;
volatile boolean success_bill;
volatile unsigned long conteo_overflow_bill;
volatile unsigned long finishTime_bill=0;
volatile unsigned long startTime_bill=0;
volatile unsigned long valor_bill =0;
volatile float show_bill=0;
volatile int contador = 0;
volatile unsigned long tiempo_bill = 0;
volatile boolean lapso_activated = false;



//FOR LCD
#define BLACK   0x0000
#define RED     0xF800
#define GREEN   0x07E0
#define WHITE   0xFFFF
#define GREY    0x8410

//Creamos el objeto rtc como reloj
RTC rtc(DST_ON);

//Definimos los pines involucrados con el control de los módulos RELE, para las luces LED(siempre son rojos mientras no este un cel dentro)
#define led_rgb_1 23
#define led_rgb_2 25
#define led_rgb_3 27
#define led_rgb_4 29
#define led_rgb_5 31
//#define led_rgb_6 33

//definimos los pines que se utilizarán para los pulsadores NC.
#define pulsador_1 30
#define pulsador_2 32
#define pulsador_3 34
#define pulsador_4 36
#define pulsador_5 38 
/*#define pulsador_6 39*/

//Definimos los pines involucrados con el control de los módulos RELE, para los HUBs.
#define usb_1 35
#define usb_2 37
#define usb_3 22
#define usb_4 24
#define usb_5 26
/*#define usb_6 28*/

//Referente al coin acceptor
#define coin_data 48
#define bill_data 49
#define inhi_bill 51

 //FOR RESET
#define reset_pin 14//ANALOG PIN A ELECCION
#define pin_prueba 15//PARA PRUEBAS

//RELOJ...Creamos las variables donde almacenar los datos del momento de despegue de los reles. Uno para la hora actual y los otros para el momento de pegado de reles y las "alarmas"
Data d, actual,d1, d2, d3 ,d4, d5, d6 ,alarm1, alarm2, alarm3, alarm4, alarm5, alarm6;
int horas=0;
int minutos=0;

//PARA MUESTREO EN PANTALLA DE TIEMPO DISPONIBLE
int alarm_1=0;
int alarm_2=0;
int alarm_3=0;
int alarm_4=0;
int alarm_5=0;
/*int alarm_6=0;*/
int dif1=0;
int dif2=0;
int dif3=0;
int dif4=0;
int dif5=0;
/*int dif6=0;*/

int a=0;
int b=0;

//BANDERAS DE INDICACION DE INTERACCION CON PANTALLA LCD
int act1=0;
int act2=0;
int act3=0;
int act4=0;
int act5=0;
/*int act6=0;*/

//Referente al tiempo cronometrizado para tener control del ingreso de monedas
unsigned long tiempo_actual=0;
unsigned long tiempo1=0;
unsigned long previo=0;

//Definimos los créditos para cada uno de los casilleros, cada uno tendrá su propio valor asignado
volatile unsigned long dinero=0;//SE USA PARA MOSTRAR EN PANTALLA.
volatile unsigned long var_int = 0;//SE USA PARA LOS CALCULOS DE CREDITO
volatile unsigned long var_int_aux = 0;//SE USA PARA LOS CALCULOS DE CREDITO
volatile unsigned long intro= 0;
volatile unsigned long credito=0;//donde guardar la cantidad de tiempo asignado.

//Definimos banderas para indicar el estado de los casilleros (0 es libre, 1 es ocupado)
int ban_1=0;
int ban_2=0;
int ban_3=0;
int ban_4=0;
int ban_5=0;
/*int ban_6=0;*/

//VARIABLE AUXILIAR
int aux=0;
int i=0;
int res=1;//para poder resetear todo al liberarse las casillas...

//PARA LA INTERRUPCION
int mon=0;//BANDERA INGRESO DE MONEDAAAA
int nino=1;//ni noticias de cliente

//PARA EL MODO PRUEBA
int prueba=0;




void setup(){
  //FOR RESET
  digitalWrite(reset_pin,HIGH);

  
  com_mp3_ardu.begin(9600);
  audios_casilla.begin(com_mp3_ardu);
  audios_casilla.reset();
  audios_casilla.EQ(DFPLAYER_EQ_ROCK);
  audios_casilla.volume(30);  //Set volume value (0~30).
  audios_casilla.outputDevice(DFPLAYER_DEVICE_SD);
  audios_casilla.disableLoopAll();

  
//INICIO RELoj
    d = rtc.getData();
     //ponemos la fecha actual
    alarm1.year    = d.year;
    alarm1.month   = d.month;
    alarm1.day     = d.day;
    alarm2.year    = d.year;
    alarm2.month   = d.month;
    alarm2.day     = d.day;
    alarm3.year    = d.year;
    alarm3.month   = d.month;
    alarm3.day     = d.day;
    alarm4.year    = d.year;
    alarm4.month   = d.month;
    alarm4.day     = d.day;
    alarm5.year    = d.year;
    alarm5.month   = d.month;
    alarm5.day     = d.day;
   /* alarm6.year    = d.year;
    alarm6.month   = d.month;
    alarm6.day     = d.day;*/
    

//LCD
    uint16_t ID = lcd.readID();
    if (ID == 0xD3) ID = 0x9481;
    //ID = 0x9326;//comentar cuando la pantalla NO ES de 2.4"
    lcd.begin(ID);
    lcd.setRotation(1);
    lcd.fillScreen(BLACK); 

 
//Entradas PULL-UP para los pulsadores NC
  pinMode(pulsador_1, INPUT);
  pinMode(pulsador_2, INPUT);
  pinMode(pulsador_3, INPUT);
  pinMode(pulsador_4, INPUT);
  pinMode(pulsador_5, INPUT);
  /*pinMode(pulsador_6, INPUT);*/

  //HUBs de USB
  pinMode(usb_1,OUTPUT);
  pinMode(usb_2,OUTPUT);
  pinMode(usb_3,OUTPUT);
  pinMode(usb_4,OUTPUT);
  pinMode(usb_5,OUTPUT);
  /*pinMode(usb_6,OUTPUT);*/
  digitalWrite(usb_1,HIGH); 
  digitalWrite(usb_2,HIGH);
  digitalWrite(usb_3,HIGH);
  digitalWrite(usb_4,HIGH);
  digitalWrite(usb_5,HIGH);
 /* digitalWrite(usb_6,HIGH);*/

  //LEDs RGB
  pinMode(led_rgb_1,OUTPUT);
  pinMode(led_rgb_2,OUTPUT);
  pinMode(led_rgb_3,OUTPUT);
  pinMode(led_rgb_4,OUTPUT);
  pinMode(led_rgb_5,OUTPUT);
  /*pinMode(led_rgb_6,OUTPUT);*/
  digitalWrite(led_rgb_1,HIGH);
  digitalWrite(led_rgb_2,HIGH);
  digitalWrite(led_rgb_3,HIGH);
  digitalWrite(led_rgb_4,HIGH);
  digitalWrite(led_rgb_5,HIGH);
  /*digitalWrite(led_rgb_6,HIGH);*/

  //delay(3000);//esperamos 3 segundos para recibir un pulso, en ello las conexiones ya estan debidamente conectadas
  pinMode(bill_data,INPUT_PULLUP);
  pinMode(inhi_bill, OUTPUT);
  digitalWrite(inhi_bill, LOW);//acepta billetes
  
  //NOS PREPARAMOS PARA LAS INTERRUPCIONES...
 
  prepareForInterrupts_bill();

  
  pinMode(reset_pin,OUTPUT);//AL MOMENTO DE CARGAR, DESCONECTAR PIN RESET DE PIN 14!! O SINO NO CARGA
  pinMode(pin_prueba,INPUT_PULLUP);//DESDE EL INICIO ESTA EN HIGH, ESPERANDO A QUE VAYA A LOW
  
  
  //audios_casilla.play(9);
  
}
         
void loop(){
            aceptar_bill();//acepta billetes
            check_bill_acceptor();
            while(nino==1){
                if(res==1 or prueba==1){
                  resetear_valores();                
                  lcd.fillScreen(BLACK);
                  res=0;
                  d=rtc.getData();
                  previo=millis();
                  prueba=0;
                  lcd.setTextColor(WHITE);
                  lcd.setTextSize(2);
                  lcd.setCursor ( 60, 50 );
                  lcd.print ("RECARGA TU CELULAR");
                  lcd.setCursor ( 60, 150 );
                  lcd.print ("DESDE SOLO 2 MIL GS!");
                  
                  }
                if(millis()-previo>600000){//en 10 min se resetea si no hay clientes...
                    digitalWrite(reset_pin,LOW);
                }
                mon=0;//bandera de ingreso de moneda desactivada, esperando interrrupcion,...
                while(digitalRead(pin_prueba)==LOW){
                  if(prueba==0){
                    lcd.fillScreen(RED);
                    lcd.setTextColor(BLACK);
                    lcd.setTextSize(3);
                    lcd.setCursor ( 100, 100 );
                    lcd.print ("MODO PRUEBA");
                    prueba=1;
                    }
                  if(digitalRead(pulsador_1)==LOW  and digitalRead(pulsador_2)==HIGH and digitalRead(pulsador_3)==HIGH  and digitalRead(pulsador_4)==HIGH and digitalRead(pulsador_5)==HIGH/*  and digitalRead(pulsador_6)==HIGH*/){
                      digitalWrite(led_rgb_1,LOW);
                      digitalWrite(usb_1,LOW);  
                       audios_casilla.play(11);
                    }
                    if(digitalRead(pulsador_1)==HIGH  and digitalRead(pulsador_2)==LOW and digitalRead(pulsador_3)==HIGH  and digitalRead(pulsador_4)==HIGH and digitalRead(pulsador_5)==HIGH /* and digitalRead(pulsador_6)==HIGH*/){
                      digitalWrite(led_rgb_2,LOW);
                      digitalWrite(usb_2,LOW);
                       audios_casilla.play(12);
                    }
                    if(digitalRead(pulsador_1)==HIGH  and digitalRead(pulsador_2)==HIGH and digitalRead(pulsador_3)==LOW  and digitalRead(pulsador_4)==HIGH and digitalRead(pulsador_5)==HIGH/*  and digitalRead(pulsador_6)==HIGH*/){
                      digitalWrite(led_rgb_3,LOW);
                      digitalWrite(usb_3,LOW);
                       audios_casilla.play(13);
                    }
                    if(digitalRead(pulsador_1)==HIGH  and digitalRead(pulsador_2)==HIGH and digitalRead(pulsador_3)==HIGH  and digitalRead(pulsador_4)==LOW and digitalRead(pulsador_5)==HIGH/*  and digitalRead(pulsador_6)==HIGH*/){
                      digitalWrite(led_rgb_4,LOW);
                      digitalWrite(usb_4,LOW);
                       audios_casilla.play(14);
                    }
                    if(digitalRead(pulsador_1)==HIGH  and digitalRead(pulsador_2)==HIGH and digitalRead(pulsador_3)==HIGH  and digitalRead(pulsador_4)==HIGH and digitalRead(pulsador_5)==LOW/*  and digitalRead(pulsador_6)==HIGH*/){ 
                      digitalWrite(led_rgb_5,LOW);
                      digitalWrite(usb_5,LOW);
                       audios_casilla.play(15);
                      }
                    /*if(digitalRead(pulsador_1)==HIGH  and digitalRead(pulsador_2)==HIGH and digitalRead(pulsador_3)==HIGH  and digitalRead(pulsador_4)==HIGH and digitalRead(pulsador_5)==HIGH  and digitalRead(pulsador_6)==LOW){
                       digitalWrite(led_rgb_6,LOW);
                       digitalWrite(usb_6,LOW);
                        audios_casilla.play(16);
                       }
                       */
                  }
                  check_bill_acceptor();
                
             }
             
            
            if(mon==1){//COLOCACION DE UNA MONEDA...
                 rechazar_bill();
                 audios_casilla.play(19);
                 dinero = intro;//Solamente para mostrar en pantalla
                 var_int = var_int_aux;//se asigna para hacer los calculos tras bambalinas
                 horas=0;
                 minutos=0;
                 credito=0;
                 tiempo_actual=0;//para volver a entrar
                 lcd.fillScreen(BLACK);
                 delay(500);//esperamos medio seg para entrar a observar lo preparado en pantalla
                 lcd.setTextSize(2);
                 lcd.setTextColor(WHITE,BLACK);
                 lcd.setCursor ( 80, 30 );
                 lcd.print ("Billete aceptado");
                 lcd.setCursor ( 30,80 );
                 lcd.print ("Ingrese su siguiente");
                 lcd.setCursor(50,130);
                 lcd.print("billete o espere...");
                 
                 
                 lcd.setCursor(180,180);
                 lcd.print(" segundos");
                 
                 lcd.setCursor(200,200);
                 lcd.print("GS");
                 aceptar_bill();
                 audios_casilla.play(17);
                 tiempo1=millis();   
                 
              while(tiempo_actual<9200){//El tiempo de espera de carga entre un billete y otro es de 3 segundos.
                      lcd.setCursor(130,180);
                      lcd.print(String((9200-tiempo_actual)/1000));

                      lcd.setCursor(120,200);
                      lcd.print(String(dinero));
                      if (success_bill == true){
                        audios_casilla.play(20);
                        }
                      /////////
                      check_bill_acceptor_plus();
                      if(tiempo_actual >= 5000){//PARA RECHAZAR BILLETES EN CASO DE ESTAR EN TIEMPO LIMITE
                        rechazar_bill();
                        }
                      tiempo_actual=millis()-tiempo1;//tomamos registro del tiempo disponible para colocar las monedas          
               }
               
              credito=(var_int/1000) * 15;//CONSIDERAMOS EL TIEMPO CARGADO
             // noInterrupts();
              lcd.fillScreen(BLACK);
              lcd.setTextColor(WHITE,BLACK);
              lcd.setTextSize(2);
              lcd.setCursor ( 10, 0 );
              lcd.print("Tiempo de carga:");
              lcd.setCursor ( 10, 100 );
              lcd.print (String(credito));
              lcd.setCursor(80,100);
              lcd.print("minutos");
              lcd.setCursor ( 10, 200 );
              lcd.print("Escoja su casillero");
              audios_casilla.loop(10);
                        
              ///////////////////////////////////////////////////////
              if(credito>60){
                horas=credito/60;//realizamos el calculo de la cantidad de horas y minutos. La parte entera queda almacenada. Dividimos por 60 para saber la cantidad de horas
                minutos=credito % 60;//tomamos el resto de la division para saber el resto, los minutos a sumar
              }
              else{
                minutos=credito;
              }

              ///////////////////////////////////////////////////
              tiempo1=millis();//tomamos el tiempo...
              tiempo_actual=0;//aseguramos que siempre va a entrar...
              aux=0;//para que vuelva a entrar al menu despues...
              while(aux==0 and tiempo_actual<50000){//este menu debe durar 50 segundos.Preguntamos si se pulsa una casilla sin haberla tocado antes (ban_x==0)
                    if(digitalRead(pulsador_1)==LOW  and digitalRead(pulsador_2)==HIGH and digitalRead(pulsador_3)==HIGH  and digitalRead(pulsador_4)==HIGH and digitalRead(pulsador_5)==HIGH  /*and digitalRead(pulsador_6)==HIGH*/ and aux==0){
                      lcd.fillScreen(BLACK);
                      lcd.setTextSize(2);
                      lcd.setTextColor(WHITE,BLACK);
                      lcd.setCursor ( 120, 40 );
                      lcd.print ("Casilla 1");
                      lcd.setCursor ( 30, 120 );
                      lcd.print ("Conecte y espere...");
                      audios_casilla.play(11);
                      delay(3000);
                      if(ban_1==0){//Si es la primera vez...
                          d1=rtc.getData();
                          alarm_1=((d1.hour24h*3600)+(d1.minutes*60)+d1.seconds)+credito*60;//para tener la cantidad de segundos que se ha introducido para luego quitar el celular de su casilla...
                          alarm1.hour24h = d1.hour24h + horas;//Cargaremos la hora de la alarma de las casillas
                        if(alarm1.hour24h > 23){//por si se pase 23...
                          alarm1.hour24h=alarm1.hour24h - (23+1);
                        }
                        alarm1.minutes = d1.minutes + minutos;//sumamos estas cantidades, el minutero actual mas los minutos cargados.
                        if(alarm1.minutes >60){//Si la suma supera a 60...
                            alarm1.hour24h=alarm1.hour24h + alarm1.minutes/60;//le sumamos la parte entera de la division
                            alarm1.minutes=alarm1.minutes % 60;//queda el resto de la division, osea lo que le resta para alcanzar 1 hora...
                        }
                        alarm1.seconds = d1.seconds;
                        
                        }
                        else{//PARA AUMENTAR TIEMPO
                            alarm_1=alarm_1+credito*60;
                            alarm1.hour24h = alarm1.hour24h + horas;//Cargaremos la hora de la alarma de las casillas
                            if(alarm1.hour24h > 23){//por si se pase 23...
                              alarm1.hour24h=alarm1.hour24h - (23+1);
                              }
                            alarm1.minutes = alarm1.minutes + minutos;//sumamos estas cantidades, el minutero actual mas los minutos cargados.
                            if(alarm1.minutes >60){//Si la suma supera a 60...
                              alarm1.hour24h=alarm1.hour24h + alarm1.minutes/60;//le sumamos la parte entera de la division
                              alarm1.minutes=alarm1.minutes % 60;//queda el resto de la division, osea lo que le resta para alcanzar 1 hora...
                            }
                            alarm1.seconds = alarm1.seconds;
                        
                        }
//ACTIVAMOS LOS RELES RELATIVOS A LOS HUBS Y LAS LUCES
                      digitalWrite(led_rgb_1,LOW);
                      digitalWrite(usb_1,LOW);

                      aux=1;
                      ban_1=1;
                        
                    }
                    if(digitalRead(pulsador_1)==HIGH  and digitalRead(pulsador_2)==LOW and digitalRead(pulsador_3)==HIGH  and digitalRead(pulsador_4)==HIGH and digitalRead(pulsador_5)==HIGH  /*and digitalRead(pulsador_6)==HIGH*/ and aux==0){
                      lcd.fillScreen(BLACK);
                      lcd.setTextSize(2);
                      lcd.setTextColor(WHITE,BLACK);
                      lcd.setCursor ( 120, 40 );
                      lcd.print ("Casilla 2");
                      lcd.setCursor ( 30, 120 );
                      lcd.print ("Conecte y espere...");
                      audios_casilla.play(12);
                      delay(3000);
                      if(ban_2==0){//Si es la primera vez...
                          d2=rtc.getData();
                          alarm_2=((d2.hour24h*3600)+(d2.minutes*60)+d2.seconds)+credito*60;//para tener la cantidad de segundos que se ha introducido para luego quitar el celular de su casilla...
                          alarm2.hour24h = d2.hour24h + horas;//Cargaremos la hora de la alarma de las casillas
                        if(alarm2.hour24h > 23){//por si se pase 23...
                          alarm2.hour24h=alarm2.hour24h - (23+1);
                        }
                        alarm2.minutes = d2.minutes + minutos;//sumamos estas cantidades, el minutero actual mas los minutos cargados.
                        if(alarm2.minutes >60){//Si la suma supera a 60...
                            alarm2.hour24h=alarm2.hour24h + alarm2.minutes/60;//le sumamos la parte entera de la division
                            alarm2.minutes=alarm2.minutes % 60;//queda el resto de la division, osea lo que le resta para alcanzar 1 hora...
                        }
                        alarm2.seconds = d2.seconds;
                        
                        }
                        else{//PARA AUMENTAR TIEMPO
                            alarm_2=alarm_2+credito*60;
                            alarm2.hour24h = alarm2.hour24h + horas;//Cargaremos la hora de la alarma de las casillas
                            if(alarm2.hour24h > 23){//por si se pase 23...
                              alarm2.hour24h=alarm2.hour24h - (23+1);
                              }
                            alarm2.minutes = alarm2.minutes + minutos;//sumamos estas cantidades, el minutero actual mas los minutos cargados.
                            if(alarm2.minutes >60){//Si la suma supera a 60...
                              alarm2.hour24h=alarm2.hour24h + alarm2.minutes/60;//le sumamos la parte entera de la division
                              alarm2.minutes=alarm2.minutes % 60;//queda el resto de la division, osea lo que le resta para alcanzar 1 hora...
                            }
                            alarm2.seconds = alarm2.seconds;
                        
                        }
//ACTIVAMOS LOS RELES RELATIVOS A LOS HUBS Y LAS LUCES
                      digitalWrite(led_rgb_2,LOW);
                      digitalWrite(usb_2,LOW);
                      
                      aux=1;
                      ban_2=1;
                    }
                    if(digitalRead(pulsador_1)==HIGH  and digitalRead(pulsador_2)==HIGH and digitalRead(pulsador_3)==LOW  and digitalRead(pulsador_4)==HIGH and digitalRead(pulsador_5)==HIGH  /*and digitalRead(pulsador_6)==HIGH*/ and aux==0){
                      lcd.fillScreen(BLACK);
                      lcd.setTextSize(2);
                      lcd.setTextColor(WHITE,BLACK);
                      lcd.setCursor ( 120, 40 );
                      lcd.print ("Casilla 3");
                      lcd.setCursor ( 30, 120 );
                      lcd.print ("Conecte y espere...");
                      audios_casilla.play(13);
                      delay(3000);
                      if(ban_3==0){//Si es la primera vez...
                          d3=rtc.getData();
                          alarm_3=((d3.hour24h*3600)+(d3.minutes*60)+d3.seconds)+credito*60;//para tener la cantidad de segundos que se ha introducido para luego quitar el celular de su casilla...
                          alarm3.hour24h = d3.hour24h + horas;//Cargaremos la hora de la alarma de las casillas
                        if(alarm3.hour24h > 23){//por si se pase 23...
                          alarm3.hour24h=alarm3.hour24h - (23+1);
                        }
                        alarm3.minutes = d3.minutes + minutos;//sumamos estas cantidades, el minutero actual mas los minutos cargados.
                        if(alarm3.minutes >60){//Si la suma supera a 60...
                            alarm3.hour24h=alarm3.hour24h + alarm3.minutes/60;//le sumamos la parte entera de la division
                            alarm3.minutes=alarm3.minutes % 60;//queda el resto de la division, osea lo que le resta para alcanzar 1 hora...
                        }
                        alarm3.seconds = d3.seconds;
                        
                        }
                        else{//PARA AUMENTAR TIEMPO
                            alarm_3=alarm_3+credito*60;
                            alarm3.hour24h = alarm3.hour24h + horas;//Cargaremos la hora de la alarma de las casillas
                            if(alarm3.hour24h > 23){//por si se pase 23...
                              alarm3.hour24h=alarm3.hour24h - (23+1);
                              }
                            alarm3.minutes = alarm3.minutes + minutos;//sumamos estas cantidades, el minutero actual mas los minutos cargados.
                            if(alarm3.minutes >60){//Si la suma supera a 60...
                              alarm3.hour24h=alarm3.hour24h + alarm3.minutes/60;//le sumamos la parte entera de la division
                              alarm3.minutes=alarm3.minutes % 60;//queda el resto de la division, osea lo que le resta para alcanzar 1 hora...
                            }
                            alarm3.seconds = alarm3.seconds;
                        
                        }
//ACTIVAMOS LOS RELES RELATIVOS A LOS HUBS Y LAS LUCES
                      digitalWrite(led_rgb_3,LOW);
                      digitalWrite(usb_3,LOW);
                        
                      aux=1;
                      ban_3=1;
                    }
                    if(digitalRead(pulsador_1)==HIGH  and digitalRead(pulsador_2)==HIGH and digitalRead(pulsador_3)==HIGH  and digitalRead(pulsador_4)==LOW and digitalRead(pulsador_5)==HIGH  /*and digitalRead(pulsador_6)==HIGH*/ and aux==0){
                      
                      lcd.fillScreen(BLACK);
                      lcd.setTextSize(2);
                      lcd.setTextColor(WHITE,BLACK);
                      lcd.setCursor ( 120, 40 );
                      lcd.print ("Casilla 4");
                      lcd.setCursor ( 30, 120 );
                      lcd.print ("Conecte y espere...");
                      audios_casilla.play(14);
                      delay(3000);
                      if(ban_4==0){//Si es la primera vez...
                          d4=rtc.getData();
                          alarm_4=((d4.hour24h*3600)+(d4.minutes*60)+d4.seconds)+credito*60;//para tener la cantidad de segundos que se ha introducido para luego quitar el celular de su casilla...
                          alarm4.hour24h = d4.hour24h + horas;//Cargaremos la hora de la alarma de las casillas
                        if(alarm4.hour24h > 23){//por si se pase 23...
                          alarm4.hour24h=alarm4.hour24h - (23+1);
                        }
                        alarm4.minutes = d4.minutes + minutos;//sumamos estas cantidades, el minutero actual mas los minutos cargados.
                        if(alarm4.minutes >60){//Si la suma supera a 60...
                            alarm4.hour24h=alarm4.hour24h + alarm4.minutes/60;//le sumamos la parte entera de la division
                            alarm4.minutes=alarm4.minutes % 60;//queda el resto de la division, osea lo que le resta para alcanzar 1 hora...
                        }
                        alarm4.seconds = d4.seconds;
                        
                        }
                        else{//PARA AUMENTAR TIEMPO
                            alarm_4=alarm_4+credito*60;
                            alarm4.hour24h = alarm4.hour24h + horas;//Cargaremos la hora de la alarma de las casillas
                            if(alarm4.hour24h > 23){//por si se pase 23...
                              alarm4.hour24h=alarm4.hour24h - (23+1);
                              }
                            alarm4.minutes = alarm4.minutes + minutos;//sumamos estas cantidades, el minutero actual mas los minutos cargados.
                            if(alarm4.minutes >60){//Si la suma supera a 60...
                              alarm4.hour24h=alarm4.hour24h + alarm4.minutes/60;//le sumamos la parte entera de la division
                              alarm4.minutes=alarm4.minutes % 60;//queda el resto de la division, osea lo que le resta para alcanzar 1 hora...
                            }
                            alarm4.seconds = alarm4.seconds;
                        
                        }
//ACTIVAMOS LOS RELES RELATIVOS A LOS HUBS Y LAS LUCES
                      digitalWrite(led_rgb_4,LOW);
                      digitalWrite(usb_4,LOW);
                        
                      aux=1;
                      ban_4=1;
                    }
                    if(digitalRead(pulsador_1)==HIGH  and digitalRead(pulsador_2)==HIGH and digitalRead(pulsador_3)==HIGH  and digitalRead(pulsador_4)==HIGH and digitalRead(pulsador_5)==LOW  /*and digitalRead(pulsador_6)==HIGH*/ and aux==0){ 
                      lcd.fillScreen(BLACK);
                      lcd.setTextSize(2);
                      lcd.setTextColor(WHITE,BLACK);
                      lcd.setCursor ( 120, 40 );
                      lcd.print ("Casilla 5");
                      lcd.setCursor ( 30, 120 );
                      lcd.print ("Conecte y espere...");
                      audios_casilla.play(15);
                      delay(3000);
                      if(ban_5==0){//Si es la primera vez...
                          d5=rtc.getData();
                          alarm_5=((d5.hour24h*3600)+(d5.minutes*60)+d5.seconds)+credito*60;//para tener la cantidad de segundos que se ha introducido para luego quitar el celular de su casilla...
                          alarm5.hour24h = d5.hour24h + horas;//Cargaremos la hora de la alarma de las casillas
                        if(alarm5.hour24h > 23){//por si se pase 23...
                          alarm5.hour24h=alarm5.hour24h - (23+1);
                        }
                        alarm5.minutes = d5.minutes + minutos;//sumamos estas cantidades, el minutero actual mas los minutos cargados.
                        if(alarm5.minutes >60){//Si la suma supera a 60...
                            alarm5.hour24h=alarm5.hour24h + alarm5.minutes/60;//le sumamos la parte entera de la division
                            alarm5.minutes=alarm5.minutes % 60;//queda el resto de la division, osea lo que le resta para alcanzar 1 hora...
                        }
                        alarm5.seconds = d5.seconds;
                        
                        }
                        else{//PARA AUMENTAR TIEMPO
                            alarm_5=alarm_5+credito*60;
                            alarm5.hour24h = alarm5.hour24h + horas;//Cargaremos la hora de la alarma de las casillas
                            if(alarm5.hour24h > 23){//por si se pase 23...
                              alarm5.hour24h=alarm5.hour24h - (23+1);
                              }
                            alarm5.minutes = alarm5.minutes + minutos;//sumamos estas cantidades, el minutero actual mas los minutos cargados.
                            if(alarm5.minutes >60){//Si la suma supera a 60...
                              alarm5.hour24h=alarm5.hour24h + alarm5.minutes/60;//le sumamos la parte entera de la division
                              alarm5.minutes=alarm5.minutes % 60;//queda el resto de la division, osea lo que le resta para alcanzar 1 hora...
                            }
                            alarm5.seconds = alarm5.seconds;
                        
                        }
//ACTIVAMOS LOS RELES RELATIVOS A LOS HUBS Y LAS LUCES
                      digitalWrite(led_rgb_5,LOW);
                      digitalWrite(usb_5,LOW);
                        
                      aux=1;
                      ban_5=1;
                      }
                    /*if(digitalRead(pulsador_1)==HIGH  and digitalRead(pulsador_2)==HIGH and digitalRead(pulsador_3)==HIGH  and digitalRead(pulsador_4)==HIGH and digitalRead(pulsador_5)==HIGH  and digitalRead(pulsador_6)==LOW and aux==0){
                      lcd.fillScreen(BLACK);
                      lcd.setTextSize(2);
                      lcd.setTextColor(WHITE,BLACK);
                      lcd.setCursor ( 120, 40 );
                      lcd.print ("Casilla 6");
                      lcd.setCursor ( 30, 120 );
                      lcd.print ("Conecte y espere...");
                      audios_casilla.play(16);
                      delay(3000);
                      if(ban_6==0){//Si es la primera vez...
                          d6=rtc.getData();
                          alarm_6=((d6.hour24h*3600)+(d6.minutes*60)+d6.seconds)+credito*60;//para tener la cantidad de segundos que se ha introducido para luego quitar el celular de su casilla...
                          alarm6.hour24h = d6.hour24h + horas;//Cargaremos la hora de la alarma de las casillas
                        if(alarm6.hour24h > 23){//por si se pase 23...
                          alarm6.hour24h=alarm6.hour24h - (23+1);
                        }
                        alarm6.minutes = d6.minutes + minutos;//sumamos estas cantidades, el minutero actual mas los minutos cargados.
                        if(alarm6.minutes >60){//Si la suma supera a 60...
                            alarm6.hour24h=alarm6.hour24h + alarm6.minutes/60;//le sumamos la parte entera de la division
                            alarm6.minutes=alarm6.minutes % 60;//queda el resto de la division, osea lo que le resta para alcanzar 1 hora...
                        }
                        alarm6.seconds = d6.seconds;
                        
                        }
                        else{//PARA AUMENTAR TIEMPO
                            alarm_6=alarm_6+credito*60;
                            alarm6.hour24h = alarm6.hour24h + horas;//Cargaremos la hora de la alarma de las casillas
                            if(alarm6.hour24h > 23){//por si se pase 23...
                              alarm6.hour24h=alarm6.hour24h - (23+1);
                              }
                            alarm6.minutes = alarm6.minutes + minutos;//sumamos estas cantidades, el minutero actual mas los minutos cargados.
                            if(alarm6.minutes >60){//Si la suma supera a 60...
                              alarm6.hour24h=alarm6.hour24h + alarm6.minutes/60;//le sumamos la parte entera de la division
                              alarm6.minutes=alarm6.minutes % 60;//queda el resto de la division, osea lo que le resta para alcanzar 1 hora...
                            }
                            alarm6.seconds = alarm6.seconds;
                        
                        }
//ACTIVAMOS LOS RELES RELATIVOS A LOS HUBS Y LAS LUCES
                       digitalWrite(led_rgb_6,LOW);
                       digitalWrite(usb_6,LOW);
                        
                       aux=1; 
                       ban_6=1;
                       }
                       */
                    
                    tiempo_actual=millis()-tiempo1;//para controlar el tiempo
              }
            if(aux==1 or tiempo_actual>=50000){//caso de que se haya colocado una moneda aunque sea...
                 lcd.fillScreen(BLACK);//limpiamos la pantalla del lcd...
                 aux=0;
                 tiempo_actual=0;
                 audios_casilla.disableLoop(); //disable loop.
              }
              mon=0;
              lcd.setTextSize(2);
              lcd.setCursor(65,0);
              lcd.print("Tiempo restante:");
              lcd.setTextColor(WHITE,BLACK);
           
            }
          
          rechazar_bill();
          actual = rtc.getData();
          aceptar_bill();
          
          if(ban_1==1){
            b=(actual.hour24h*3600)+(actual.minutes*60)+actual.seconds;
              if(actual.hour24h==0){
                a=(alarm1.hour24h*3600)+(alarm1.minutes*60)+alarm1.seconds;
                }
               else{
                a=alarm_1;
                }
              dif1=a-b;
            if(dif1/3600>=1){
                  lcd.setTextColor(WHITE,BLACK);
                  lcd.setCursor(20,50);
                  lcd.fillCircle(10, 57, 5,RED );
                  lcd.print("1:");
                  lcd.print(String(dif1/3600));
                  lcd.print(" h ");
                  lcd.print(String((dif1%3600)/60));
                  lcd.print(" m");
              }
              else{
                lcd.setTextColor(WHITE,BLACK);
                lcd.setCursor(20,50);
                lcd.fillCircle(10, 57, 5,RED );
                lcd.print("1:");
                lcd.print(String(dif1/60));
                lcd.print("m ");
                lcd.print(String(dif1%60));
                lcd.print("s ");
                }
              
              act1=1;
            }
          check_bill_acceptor();
          if(ban_2==1){
              b=(actual.hour24h*3600)+(actual.minutes*60)+actual.seconds;
              if(actual.hour24h==0){
                a=(alarm2.hour24h*3600)+(alarm2.minutes*60)+alarm2.seconds;
                }
               else{
                a=alarm_2;
                }
              dif2=a-b;
              if(dif2/3600>=1){
                   lcd.setCursor(200,50);
                   lcd.fillCircle(190, 57, 5,RED );
                   lcd.print("2:");
                   lcd.print(String(dif2/3600));
                   lcd.print(" h ");
                   lcd.print(String((dif2%3600)/60));
                   lcd.print(" m");
               }
              else{
                lcd.setCursor(200,50);
                lcd.fillCircle(190, 57, 5,RED );
                lcd.print("2:");
                lcd.print(String(dif2/60));
                lcd.print("m ");
                lcd.print(String(dif2%60));
                lcd.print("s ");
              }
              
              act2=1;
             }
          check_bill_acceptor();
          if(ban_3==1){
            b=(actual.hour24h*3600)+(actual.minutes*60)+actual.seconds;
              if(actual.hour24h==0){
                a=(alarm3.hour24h*3600)+(alarm3.minutes*60)+alarm3.seconds;
                }
               else{
                a=alarm_3;
                }
              dif3=a-b;
            if(dif3/3600>=1){
                lcd.setCursor(20,110);
                lcd.fillCircle(10, 117, 5,RED );
                lcd.print("3:");
                lcd.print(String(dif3/3600));
                lcd.print(" h ");
                lcd.print(String((dif3%3600)/60));
                lcd.print(" m");
              }
            else{
             lcd.setCursor(20,110);
             lcd.fillCircle(10, 117, 5,RED );
             lcd.print("3:");
             lcd.print(String(dif3/60));
             lcd.print("m ");
             lcd.print(String(dif3%60));
             lcd.print("s ");
            }
            
            act3=1;
            }
          check_bill_acceptor();
          if(ban_4==1){
              b=(actual.hour24h*3600)+(actual.minutes*60)+actual.seconds;
              if(actual.hour24h==0){
                a=(alarm4.hour24h*3600)+(alarm4.minutes*60)+alarm4.seconds;
                }
               else{
                a=alarm_4;
                }
              dif4=a-b;
              if(dif4/3600>=1){
                  lcd.setCursor(200,110);
                  lcd.fillCircle(190, 117, 5,RED );
                  lcd.print("4:");
                  lcd.print(String(dif4/3600));
                  lcd.print(" h ");
                  lcd.print(String((dif4%3600)/60));
                  lcd.print(" m");
                }
             else{
                lcd.setCursor(200,110);
                lcd.fillCircle(190, 117, 5,RED );
                lcd.print("4:");
                lcd.print(String(dif4/60));
                lcd.print("m ");
                lcd.print(String(dif4%60));
                lcd.print("s ");
                }
                
             act4=1;
            }
          check_bill_acceptor();
          if(ban_5==1){
              b=(actual.hour24h*3600)+(actual.minutes*60)+actual.seconds;
              if(actual.hour24h==0){
                a=(alarm5.hour24h*3600)+(alarm5.minutes*60)+alarm5.seconds;
                }
               else{
                a=alarm_5;
                }
              dif5=a-b;
              if(dif5/3600>=1){
                  lcd.setCursor(20,170);
                  lcd.fillCircle(10, 177, 5,RED );
                  lcd.print("5:");
                  lcd.print(String(dif5/3600));
                  lcd.print(" h ");
                  lcd.print(String((dif5%3600)/60));
                  lcd.print(" m");
                }
      
              else{
                lcd.setCursor(20,170);
                lcd.fillCircle(10, 177, 5,RED );
                lcd.print("5:");
                lcd.print(String(dif5/60));
                lcd.print("m ");
                lcd.print(String(dif5%60));
                lcd.print("s ");
                }
                
              act5=1;
            }
          check_bill_acceptor();
          /*(ban_6==1){
              b=(actual.hour24h*3600)+(actual.minutes*60)+actual.seconds;
              if(actual.hour24h==0){
                a=(alarm6.hour24h*3600)+(alarm6.minutes*60)+alarm6.seconds;
                }
               else{
                a=alarm_6;
                }
              dif6=a-b;
              if(dif6/3600>=1){
                  lcd.setCursor(210,170);
                  lcd.fillCircle(200, 177, 5,RED );
                  lcd.print("6:");
                  lcd.print(String(dif6/3600));
                  lcd.print(" h ");
                  lcd.print(String((dif6%3600)/60));
                  lcd.print(" m");
                }
              else{
                lcd.setCursor(230,170);
                lcd.fillCircle(200, 177, 5,RED );
                lcd.print("6:");
                lcd.print(String(dif6/60));
                lcd.print("m ");
                lcd.print(String(dif6%60));
                lcd.print("s ");
                }
                
              act6=1;
            }
            */
          check_bill_acceptor();
          //PARA DESPEGAR LOS RELES EN CASO DE QUE AL MENOS 1 haya agotado su tiempo o que no esté activo. Esto puede ocurrir cuando no todos los casilleros esten ocupados...
            actual=rtc.getData();//PEDIMOS LA HORA ACTUAL
            if(dif1<=0 ){//sale si es que ha llegado la hora...
                    ban_1=0;
            }
            if(dif2<=0){//sale si es que ha llegado la hora...
                    ban_2=0;
            }
            if(dif3<=0){//sale si es que ha llegado la hora...
                    ban_3=0;
            }
            if(dif4<=0){//sale si es que ha llegado la hora...
                    ban_4=0;
            }
            if(dif5<=0){//sale si es que ha llegado la hora...
                  ban_5=0;
            }
            /*if(dif6<=0){//sale si es que ha llegado la hora...
                  ban_6=0;
            }
            */
            check_bill_acceptor();
            
                    //EN CASO DE QUE ALGUNA CASILLA ESTE DESACTIVADA...
          if(ban_1==0){//apagar reles primera casilla
                  if(act1==1){
                    digitalWrite(led_rgb_1,HIGH);
                    digitalWrite(usb_1,HIGH);
                    lcd.fillScreen(BLACK);
                    
                    act1=0;
                    alarm_1=0;
                    a=0;
                    b=0;
                    dif1=0;
                    }
                  lcd.setCursor(20,50);
                  lcd.fillCircle(10, 57, 5,GREEN );
                  lcd.print("1:");
                  lcd.print("LIBRE");
                  
            }
          if(ban_2==0){//apagar reles segunda casilla
                  if(act2==1){
                    digitalWrite(led_rgb_2,HIGH);
                    digitalWrite(usb_2,HIGH);
                    lcd.fillScreen(BLACK);
                    
                    act2=0;
                    alarm_2=0;
                    a=0;
                    b=0;
                    dif2=0;
                  }
                  lcd.setCursor(200,50);
                  lcd.fillCircle(190, 57, 5,GREEN );
                  lcd.print("2:");
                  lcd.print("LIBRE");
                  
            }
          if(ban_3==0){//apagar reles tercera casilla
                  if(act3==1){
                    digitalWrite(led_rgb_3,HIGH);
                    digitalWrite(usb_3,HIGH);
                    lcd.fillScreen(BLACK);
                    
                    act3=0;
                    alarm_3=0;
                    a=0;
                    b=0;
                    dif3=0;
                  }
                  lcd.setCursor(20,110);
                  lcd.fillCircle(10, 117, 5,GREEN );
                  lcd.print("3:");
                  lcd.print("LIBRE");
                  
                  
            }
          if(ban_4==0){//apagar reles cuarta casilla
                  if(act4==1){
                    digitalWrite(led_rgb_4,HIGH);
                    digitalWrite(usb_4,HIGH);
                    lcd.fillScreen(BLACK);
                    
                    act4=0;
                    alarm_4=0;
                    a=0;
                    b=0;
                    dif4=0;
                  }
                  lcd.setCursor(200,110);
                  lcd.fillCircle(190, 117, 5,GREEN );
                  lcd.print("4:");
                  lcd.print("LIBRE");
                  
                  
            }
          if(ban_5==0){//apagar reles quinta casilla
                  if(act5==1){
                    digitalWrite(led_rgb_5,HIGH);
                    digitalWrite(usb_5,HIGH);
                    lcd.fillScreen(BLACK);
                    
                    act5=0;
                    alarm_5=0;
                    a=0;
                    b=0;
                    dif5=0;
                  }
                  lcd.setCursor(20,170);
                  lcd.fillCircle(10, 177, 5,GREEN );
                  lcd.print("5:");
                  lcd.print("LIBRE");
                  
            }
          /*if(ban_6==0){//apagar reles sexta casilla
                  if(act6==1){
                    digitalWrite(led_rgb_6,HIGH);
                    digitalWrite(usb_6,HIGH);
                    lcd.fillScreen(BLACK);
                    
                    act6=0;
                    alarm_6=0;
                    a=0;
                    b=0;
                    dif6=0;
                  }
                  lcd.setCursor(210,170);
                  lcd.fillCircle(200, 177, 5,GREEN );
                  lcd.print("6:");
                  lcd.print("LIBRE");
                  
            }
            */
      check_bill_acceptor();
      //AL LIBERARSE TODAS LAS CASILLAS!!!!!!!!
      if(ban_1==0 and ban_2==0 and ban_3 ==0 and ban_4 ==0 and ban_5 ==0 /*and ban_6==0*/){
              nino=1;
              res=1;
              dinero=0;
              credito=0;
         }
      check_bill_acceptor();
          
        

}///FIN DEL LOOP

//DECLARAMOS LAS SUBFUNCIONES QUE SE ESTARAN UTILIZANDO EN EL PROYECTO

ISR(TIMER4_OVF_vect){//EN CASO DE OVERFLOW DEL TIMER 4...
    conteo_overflow_bill++;
}

 ISR(TIMER3_COMPA_vect){//SE ACTIVA CUANDO EL TIMER3 llega a un punto donde TCNT3 ES IGUAL A OCR3A

  lapso_activated = true;
  TCNT3 = 0;//Colocamos el contador a cero.
  TIMSK3 = 0b00000000;//Se desactiva la interrupción por OVERFLOW.
 
   
 }

ISR(TIMER4_CAPT_vect){//INTERRUPT SERVICE RUTINE PARA LA CAPTURA DE FLANCOS EN EL TIMER 4 DEL MEGA (PIN 48)
   
   //REGISTRAMOS EL VALOR DE CONTEO DONDE SE DETECTO EL EVENTO(SEA RISING O FALLING)
   unsigned int valor_conteo_bill;
   valor_conteo_bill=ICR4;
   
   //REALIZAMOS LA COPIA DEL VALOR DEL NUMERO DE VECES QUE SE DESBORDÓ EL TIMER PARA NO TOCAR EL VALOR DEL CONTADOR DE OVERFLOW YA PRESENTE
   unsigned long conteo_overflow_copy_bill;
   conteo_overflow_copy_bill=conteo_overflow_bill;
    
    // SI SE PERDIO UN CONTEO DE OVERFLOW... SE PREGUNTA SI SE ACTIVO LA BANDERA DE INTERRUPCION POR OVERFLOW Y SE PREGUNTA POR EL VALOR DE CONTEO...
    if ((TIFR4 & bit (TOV4)) && valor_conteo_bill < 0x7FFF)
      {conteo_overflow_copy_bill++;}
    
    
    //PARA SALIR DE LA RUTINA DE LA INTERRUPCION CASO DE QUE AUN NO SE TOMO EL VALOR CALCULADO EN EL LOOP...
    if (success_bill)
      {return;}
    
    
    //ESTO OCURRE CUANDO EL PULSO RECIEN INICIA(DETECTADO EL RISING) Y SE SALE DE LA RUTINA DE INTERRUPCION!!!
    if (first_bill)
        {
        startTime_bill = (conteo_overflow_copy_bill << 16) + valor_conteo_bill;
        TIFR4 = 0b00100001;      //SE LIMPIA LA BANDERA CORRESPONDIENTE A CAPTURA (ICF4)
        TCCR4B =  0b11000001;     // NO PREESCALER, SE SELECCIONA LA BUSQUEDA DE UN FLANCO DE SUBIDA EN EN PIN DEL COIN
        first_bill = false;            //PARA PROCEDER EN LA SGTE DETECCION A CALCULAR EL PUNTO DONDE CAE EL PULSO
        return;                   //PARA SALIR DE LA RUTINA DE INTERRUPCION!!!!! OJO!!!
        }
      
      
      //SE PROCEDE A CALCULAR  EL VALOR EXACTO DEL MOMENTO EN EL QUE OCURRIO EL RISING (MISMO SI OCURRIO UN OVERFLOW, COMO SI NO) 
      finishTime_bill = (conteo_overflow_copy_bill << 16) + valor_conteo_bill;
      success_bill = true;
      TIMSK4 = 0b00000000;    // DESACTIVAMOS LAS INTERRUPCIONES QUE NOS INTERESAN DETECTAR PARA EL TIMER 4
    
}

void prepareForInterrupts_timer3(){
  noInterrupts();
  TCCR3A = 0b00000000;
  TCCR3B = 0b00000000;
  TCNT3 = 0;
  OCR3A = 0x5208;//1770;//calculado a partir de la formula de registro de comparación.
  TCCR3B |= (1 << WGM32)|(1<<CS30)|(1 << CS32);   // modo CTC, prescaler de 1024: CS12 = 1 e CS10 = 1  
  TIMSK3 |= (1 << OCIE3A);//Se activa la interrupción en caso de que el contador del timer3 
                          //coicida con el valor guardado en registro A correspondiente a 300ms
  lapso_activated = false;
  //digitalWrite(LED_BUILTIN,HIGH);
  interrupts();
 }
 
void prepareForInterrupts_bill()//PARA PREPARAR PARA LAS INTERRUPCIONES...
  {
    noInterrupts (); 
    first_bill = true;
    success_bill = false;  // re-arm for next time
    
    // reset Timer 4
    TCCR4A = 0b00000000;
    TCCR4B = 0b00000000;
    
    // LIMPIAMOS LAS BANDERAS PARA NO TENERLAS INDICANDO ALGO QUE NO QUEREMOS REGISTRAR
    TIFR4 = bit (ICF4) | bit (TOV4);  
   
    //CONTADOR A CERO
    TCNT4 = 0;          // Counter to zero

    //POR ESA RAZON, NO TENEMOS CONTEOS OVERFLOW
    conteo_overflow_bill = 0;
    
    // ACTIVAMOS LAS INTERRUPCIONES QUE NOS INTERESAN DETECTAR PARA EL TIMER 4, UNO PARA EL INPUT CAPTURE Y EL OTRO PARA EL DESBORDE DE CONTEO
    TIMSK4 = 0b00100001;
    
    // INICIAMOS TIMER 4, CON UN PRESCALER DE 1 Y ESPERANDO PARA DETECTAR UN FALLING...
    TCCR4B = 0b10000001;
    interrupts ();
  }


void resetear_valores(){
    dinero = 0;
    intro = 0;
    var_int = 0;
    var_int_aux = 0;
    alarm1.year    = d.year;
    alarm1.month   = d.month;
    alarm1.day     = d.day;
    alarm2.year    = d.year;
    alarm2.month   = d.month;
    alarm2.day     = d.day;
    alarm3.year    = d.year;
    alarm3.month   = d.month;
    alarm3.day     = d.day;
    alarm4.year    = d.year;
    alarm4.month   = d.month;
    alarm4.day     = d.day;
    alarm4.year    = d.year;
    alarm4.month   = d.month;
    alarm4.day     = d.day;
    alarm5.year    = d.year;
    alarm5.month   = d.month;
    alarm5.day     = d.day;
    /*alarm6.year    = d.year;
    alarm6.month   = d.month;
    alarm6.day     = d.day;*/
    alarm_1=0;
    alarm_2=0;
    alarm_3=0;
    alarm_4=0;
    alarm_5=0;
    /*alarm_6=0;*/
    dif1=0;
    dif2=0;
    dif3=0;
    dif4=0;
    dif5=0;
   /* dif6=0;*/
    a=0;
    b=0;
    act1=0;
    act2=0;
    act3=0;
    act4=0;
    act5=0;
    /*act6=0;*/
    digitalWrite(usb_1,HIGH); 
    digitalWrite(usb_2,HIGH);
    digitalWrite(usb_3,HIGH);
    digitalWrite(usb_4,HIGH);
    digitalWrite(usb_5,HIGH);
    /*digitalWrite(usb_6,HIGH);*/
    digitalWrite(led_rgb_1,HIGH);
    digitalWrite(led_rgb_2,HIGH);
    digitalWrite(led_rgb_3,HIGH);
    digitalWrite(led_rgb_4,HIGH);
    digitalWrite(led_rgb_5,HIGH);
    /*digitalWrite(led_rgb_6,HIGH);*/
  
    prepareForInterrupts_bill();
  }

  
void check_bill_acceptor(){
   
   if(success_bill){
      rechazar_bill();
      intro = 0;
      //Serial.println("ADENTRO");
      noInterrupts();
      valor_bill = finishTime_bill - startTime_bill;
      show_bill=0.0000625*valor_bill;//62.5ns es el timepo que tarda un ciclo de reloj del timer
     
      if(show_bill>20.00 && show_bill<60.00){ //AL MENOS SE COMPRUEBA QUE 2000 GS SE INTRODUJO.
         contador = 1;
        // Serial.println("HOLO");
         nino=0;//para salir del bucle...
         mon=1;//para entrar en la rutina de moneda aceptada..
         interrupts();
         prepareForInterrupts_timer3();//activamos el timer 3 y esperamos a que se dispare en 300 ms.
         prepareForInterrupts_bill ();
         lapso_activated = false;
         success_bill=false;

      }

      success_bill=false;//para poder volver a entrar otv...
      lapso_activated = false;  
      interrupts();
      prepareForInterrupts_bill(); 
      prepareForInterrupts_timer3();
             
      while(lapso_activated == false){//SE ESPERA LA FINALIZACIÓN DEL RELOJ QUE DURA 300 ms
      
         if(success_bill){
            
            noInterrupts();
            valor_bill = finishTime_bill - startTime_bill;
            show_bill=0.0000625*valor_bill;//62.5ns es el timepo que tarda un ciclo de reloj del timer
            
            if(show_bill>20.00 && show_bill<60.00){ 
               contador = contador+1;
            }

            success_bill = false;
            interrupts();
            prepareForInterrupts_bill();
            
 
         }
         
      }
     // Serial.println(contador);
      prepareForInterrupts_bill();
      success_bill=false;//para poder volver a entrar otv...
      lapso_activated = false;  
      interrupts();
       
       //Serial.print("el contador va en: ");
         //Serial.println(contador);
         
      
      if (contador >0.5 and contador <=3){//EL BILLETE ES DE 2000 GS.
        intro = 2000;
        var_int_aux = 3000;//EQUIVALENTE A 45 MIN... 3 X 15
       }
      if (contador >= 3 and contador <=5){//EL BILLETE ES DE 5000 GS.
        intro = 5000;
        var_int_aux = 8000;//EQUIVALENTE A 120 MIN... 8 X 15
        }
      if (contador > 5 and contador<=10){//EL BILLETE ES DE 10000 GS.
        intro = 10000;
        var_int_aux = 16000;//EQUIVALENTE A 240 MIN... 16 X 15
        
        }
     // tiempo1=millis();
      if(intro==0){return;}//para salir del if en caso de que el pulso sea inválido...
          
      contador = 0;
    aceptar_bill();
   }
    
}

void check_bill_acceptor_plus(){//EN CASO DE COLOCAR OTRA MONEDA...
  
  if(success_bill){
      rechazar_bill();
      intro = 0;

      noInterrupts();
      valor_bill = finishTime_bill - startTime_bill;
      show_bill=0.0000625*valor_bill;//62.5ns es el timepo que tarda un ciclo de reloj del timer
     
      if(show_bill>20.00 && show_bill<60.00){ //AL MENOS SE COMPRUEBA QUE 2000 GS SE INTRODUJO.
         contador = 1;
         intro = 2000;
         interrupts();
         prepareForInterrupts_timer3();//activamos el timer 3 y esperamos a que se dispare en 300 ms.
         prepareForInterrupts_bill ();
         lapso_activated = false;
         success_bill=false;

      }
      else{contador = 0;}

      success_bill=false;//para poder volver a entrar otv...
      lapso_activated = false;  
      interrupts();
      prepareForInterrupts_bill(); 
      prepareForInterrupts_timer3();
             
      while(lapso_activated == false){//SE ESPERA LA FINALIZACIÓN DEL RELOJ QUE DURA 300 ms
      
         if(success_bill){
            
            noInterrupts();
            valor_bill = finishTime_bill - startTime_bill;
            show_bill=0.0000625*valor_bill;//62.5ns es el timepo que tarda un ciclo de reloj del timer
            
            if(show_bill>20.00 && show_bill<60.00){ 
               contador = contador+1;
            }

            success_bill = false;
            interrupts();
            prepareForInterrupts_bill();
            
 
         }
         
      }
     
      prepareForInterrupts_bill();
      success_bill=false;//para poder volver a entrar otv...
      lapso_activated = false;  
      interrupts();
       
      
      if (contador >0.5 and contador <=3){//EL BILLETE ES DE 2000 GS.
        intro = 2000;
        var_int_aux = 3000;//EQUIVALENTE A 45 MIN (15 min x 3)
        }
     if (contador >= 3 and contador <=5){//EL BILLETE ES DE 5000 GS.
        intro = 5000;
        var_int_aux = 8000;//EQUIVALENTE A 120 MINUTOS (15 min x 8)
        }
      if (contador > 5 and contador<=10){//EL BILLETE ES DE 10000 GS.
        intro = 10000;
        var_int_aux = 16000;//EQUIVALENTE A 240 MINUTOS (15 min x 16)
   
        
        }
      dinero = dinero + intro;//LO QUE SE MUESTRA EN PANTALLA
      var_int = var_int + var_int_aux;//lo que se calcula para credito final
      tiempo1=millis();
      if(intro==0){return;}//para salir del if en caso de que el pulso sea inválido...
          
      contador = 0;
    aceptar_bill();
   }
  
}
void aceptar_bill(){
  digitalWrite(inhi_bill, LOW);
  }
void rechazar_bill(){
  digitalWrite(inhi_bill, HIGH);
  }
