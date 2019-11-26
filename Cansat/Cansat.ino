#include <SFE_BMP180.h>
#include <I2Cdev.h>
#include <MPU6050.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>

#define SEPARADOR "; "

File archivoCsv;
SFE_BMP180 bmp180;
double presionNivelMar = 1013.25; //presion sobre el nivel del mar en mbar
const int chipSelect = 4; //Es el PIN de SS por defecto para la microSD

MPU6050 mpu6050;


void setup() {
  Serial.begin(57600);
  while (!Serial) {
    ; 
  }
  Wire.begin();           //Iniciando I2C  

  //Falta codgio de calibración del acelerometro y giroscopo
  mpu6050.initialize();   //Iniciando el sensor
  if (mpu6050.testConnection()) Serial.println(F("MPU6050: Inicializacion exitosa"));
  else 
  {
    Serial.println(F("MPU6050: Error al iniciar"));
    while(1);
  }
  
  if (bmp180.begin())
    Serial.println(F("BMP180: Inicializacion exitosa"));
  else
  {
    Serial.println(F("BMP180: Error al iniciar"));
    while(1);
  }

  if (!SD.begin(chipSelect)) {
    Serial.println(F("SD: No se pudo inicializar"));
    while(1);
  }
  Serial.println(F("SD: Inicializacion exitosa"));
  if(!SD.exists("cansat.csv")){
    archivoCsv = SD.open("cansat.csv", FILE_WRITE);//El nombre de archivo no debe superar 8 caracteres
    if(archivoCsv){
      archivoCsv.println(F("Temperatura (*C); Presion (mb); Humedad (m s.n.m.); AX; AY; AZ; GX; GY; GZ"));//El acento en presión lo toma como caracter extraño
      archivoCsv.close();
    } else {
      Serial.println(F("SD: Error creando el archivo cansat.csv"));
    }    
  }
}

void loop() {
  double T, P, A;
  int16_t ax, ay, az, gx, gy, gz;
  sensarMPU6050(ax, ay, az, gx, gy, gz);
  sensarBMP180(T, P, A);
  escribirArchivo(T, P, A, ax, ay, az, gx, gy, gz);  
  delay(1000);
}

void sensarMPU6050(int16_t &ax, int16_t &ay, int16_t &az, int16_t &gx, int16_t &gy, int16_t &gz) {
  mpu6050.getAcceleration(&ax, &ay, &az);
  mpu6050.getRotation(&gx, &gy, &gz);
}

void sensarBMP180(double &temp, double &pres, double &alt) {
  char status;
  
  status = bmp180.startTemperature();//Inicio de lectura de temperatura
  if (status != 0)
  {   
    delay(status); //Pausa para que finalice la lectura
    status = bmp180.getTemperature(temp); //Obtener la temperatura
    if (status != 0)
    {
      status = bmp180.startPressure(3);//Inicio lectura de presión
      if (status != 0)
      {        
        delay(status);//Pausa para que finalice la lectura        
        status = bmp180.getPressure(pres,temp);//Obtenemos la presión
        if (status != 0)
        {                  
          alt = bmp180.altitude(pres,presionNivelMar);
        }      
      }      
    }   
  }
}

void escribirArchivo(double temp, double pres, double alti, int16_t ax, int16_t ay, int16_t az, int16_t gx, int16_t gy, int16_t gz){
  archivoCsv = SD.open("cansat.csv", FILE_WRITE);
  if(archivoCsv){
    String linea((char *)0);
    linea.reserve(100);
    linea += String(temp);
    linea += SEPARADOR;
    linea += String(pres);
    linea += SEPARADOR;
    linea += String(alti);
    linea += SEPARADOR;
    linea += String(ax);
    linea += SEPARADOR;
    linea += String(ay);
    linea += SEPARADOR;
    linea += String(az);
    linea += SEPARADOR;
    linea += String(gx);
    linea += SEPARADOR;
    linea += String(gy);
    linea += SEPARADOR;
    linea += String(gz);
    archivoCsv.println(linea);
    archivoCsv.close();
    Serial.println("SD: Dato almacenado");
  } else {
    Serial.println("SD: Error escribiendo el archivo cansat.csv");
  }
}
