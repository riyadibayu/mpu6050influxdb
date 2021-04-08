/**
 * Basic Write Example code for InfluxDBClient library for Arduino
 * Data can be immediately seen in a InfluxDB UI: wifi_status measurement
 * Enter WiFi and InfluxDB parameters below
 *
 * Measures signal level of the actually connected WiFi network
 * This example supports only InfluxDB running from unsecure (http://...)
 * For secure (https://...) or Influx Cloud 2 use SecureWrite example
 **/

#if defined(ESP32)
#include <WiFiMulti.h>
WiFiMulti wifiMulti;
#define DEVICE "ESP32"
#elif defined(ESP8266)
#include <ESP8266WiFiMulti.h>
ESP8266WiFiMulti wifiMulti;
#define DEVICE "ESP8266"
#endif
#include <Wire.h>      // MPU6050 Slave Device Address
#include <InfluxDbClient.h>
const uint8_t MPU6050SlaveAddress = 0x68; // Select SDA and SCL pins for I2C communication 
const uint8_t scl = D6;
const uint8_t sda = D7;     // sensitivity scale factor respective to full scale setting 
        // provided  in datasheet 
const uint16_t AccelScaleFactor = 16384;
const uint16_t GyroScaleFactor = 131; // MPU6050 few configuration register addresses
const uint8_t MPU6050_REGISTER_SMPLRT_DIV   =  0x19;
const uint8_t MPU6050_REGISTER_USER_CTRL    =  0x6A;
const uint8_t MPU6050_REGISTER_PWR_MGMT_1   =  0x6B;
const uint8_t MPU6050_REGISTER_PWR_MGMT_2   =  0x6C;
const uint8_t MPU6050_REGISTER_CONFIG       =  0x1A;
const uint8_t MPU6050_REGISTER_GYRO_CONFIG  =  0x1B;
const uint8_t MPU6050_REGISTER_ACCEL_CONFIG =  0x1C;
const uint8_t MPU6050_REGISTER_FIFO_EN      =  0x23;
const uint8_t MPU6050_REGISTER_INT_ENABLE   =  0x38;
const uint8_t MPU6050_REGISTER_ACCEL_XOUT_H =  0x3B;
const uint8_t MPU6050_REGISTER_SIGNAL_PATH_RESET  = 0x68;

int16_t AccelX, AccelY, AccelZ, Temperature, GyroX, GyroY, GyroZ;

// WiFi AP SSID
#define WIFI_SSID "SepedaRiang"
// WiFi password
#define WIFI_PASSWORD "riang251213"
// InfluxDB  server url. Don't use localhost, always server name or ip address.
// E.g. http://192.168.1.48:8086 (In InfluxDB 2 UI -> Load Data -> Client Libraries), 
#define INFLUXDB_URL "http://192.168.0.102:8086"
// InfluxDB 2 server or cloud API authentication token (Use: InfluxDB UI -> Load Data -> Tokens -> <select token>)
#define INFLUXDB_TOKEN "vqkCoefGka_B5eh2KK1l3pJhPz2rOon8ajvnzEVp5BPTpwazZ5TqgwQgxX5-5XZmHQtKyNh1ustGohb6tYNhRA=="
// InfluxDB 2 organization id (Use: InfluxDB UI -> Settings -> Profile -> <name under tile> )
#define INFLUXDB_ORG "bgp"
// InfluxDB 2 bucket name (Use: InfluxDB UI -> Load Data -> Buckets)
#define INFLUXDB_BUCKET "mpu6050_new"
// InfluxDB v1 database name 
//#define INFLUXDB_DB_NAME "database"

// InfluxDB client instance
InfluxDBClient client(INFLUXDB_URL, INFLUXDB_ORG, INFLUXDB_BUCKET, INFLUXDB_TOKEN);

Point gyrox("gyrox");
Point gyroy("gyroy");
Point gyroz("gyroz");
Point axcelx("axcelx");
Point axcely("axcely");
Point axcelz("axcelz");
Point temperature("temperature");


void setup() {
  Serial.begin(115200);
  Serial.begin(9600);  
  Wire.begin(sda, scl);  
  MPU6050_Init();
  
  // Connect WiFi
  Serial.println("Connecting to WiFi");
  WiFi.mode(WIFI_STA);
  wifiMulti.addAP(WIFI_SSID, WIFI_PASSWORD);
  while (wifiMulti.run() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println();

  if (client.validateConnection()) {
    Serial.print("Connected to InfluxDB: ");
    Serial.println(client.getServerUrl());
  } else {
    Serial.print("InfluxDB connection failed: ");
    Serial.println(client.getLastErrorMessage());
  }
}

void loop() {
  double Ax, Ay, Az, T, Gx, Gy, Gz;     
  Read_RawValue(MPU6050SlaveAddress, MPU6050_REGISTER_ACCEL_XOUT_H);    
  //divide each with their sensitivity scale factor  
  Ax = (double)AccelX/AccelScaleFactor;  
  Ay = (double)AccelY/AccelScaleFactor;  
  Az = (double)AccelZ/AccelScaleFactor;  
  T = (double)Temperature/340+36.53; //temperature formula  
  Gx = (double)GyroX/GyroScaleFactor;  
  Gy = (double)GyroY/GyroScaleFactor;  
  Gz = (double)GyroZ/GyroScaleFactor;  
  // Store measured value into point
  gyrox.clearFields();
  gyrox.addField("gyrox",Gx);
  gyroy.clearFields();
  gyroy.addField("gyroy",Gy);
  gyroz.clearFields();
  gyroz.addField("gyroz",Gz);
  axcelz.clearFields();
  axcelz.addField("axcelz",Az);
  axcelx.clearFields();
  axcelx.addField("axcelx",Ax);
  axcely.clearFields();
  axcely.addField("axcely",Ay);
  temperature.clearFields();
  temperature.addField("axcely",T);
  Serial.print("Writing: ");
  Serial.print(client.pointToLineProtocol(gyrox));
  Serial.print(client.pointToLineProtocol(gyroy));
  Serial.print(client.pointToLineProtocol(gyroz));
  Serial.print(client.pointToLineProtocol(axcelz));
  Serial.print(client.pointToLineProtocol(axcelx));
  Serial.print(client.pointToLineProtocol(axcely));
  Serial.print(client.pointToLineProtocol(temperature));
  client.writePoint(gyrox);
  client.writePoint(gyroy);
  client.writePoint(gyroz);
  client.writePoint(axcelx);
  client.writePoint(axcely);
  client.writePoint(axcelz);
  client.writePoint(temperature);
  delay(500);
}

void I2C_Write(uint8_t deviceAddress, uint8_t regAddress, uint8_t data)
{  
  Wire.beginTransmission(deviceAddress);  
  Wire.write(regAddress);  Wire.write(data);  
  Wire.endTransmission();   // read all 14 register
}

void Read_RawValue(uint8_t deviceAddress, uint8_t regAddress)
{  
  Wire.beginTransmission(deviceAddress);  
  Wire.write(regAddress);  
  Wire.endTransmission();  
  Wire.requestFrom(deviceAddress, (uint8_t)14);  
  AccelX = (((int16_t)Wire.read()<<8) | Wire.read());  
  AccelY = (((int16_t)Wire.read()<<8) | Wire.read());  
  AccelZ = (((int16_t)Wire.read()<<8) | Wire.read());  
  Temperature = (((int16_t)Wire.read()<<8) | Wire.read());  
  GyroX = (((int16_t)Wire.read()<<8) | Wire.read());  
  GyroY = (((int16_t)Wire.read()<<8) | Wire.read());  
  GyroZ = (((int16_t)Wire.read()<<8) | Wire.read());
}

//configure MPU6050

void MPU6050_Init()
{   
  delay(150);  
  I2C_Write(MPU6050SlaveAddress, MPU6050_REGISTER_SMPLRT_DIV, 0x07);    I2C_Write(MPU6050SlaveAddress, MPU6050_REGISTER_PWR_MGMT_1, 0x01);    I2C_Write(MPU6050SlaveAddress, MPU6050_REGISTER_PWR_MGMT_2, 0x00);
  I2C_Write(MPU6050SlaveAddress, MPU6050_REGISTER_CONFIG, 0x00);    I2C_Write(MPU6050SlaveAddress, MPU6050_REGISTER_GYRO_CONFIG, 0x00);    
  //set +/-250 degree/second full scale  
  
  I2C_Write(MPU6050SlaveAddress, MPU6050_REGISTER_ACCEL_CONFIG, 0x00);   // set +/- 2g full scale   I2C_Write(MPU6050SlaveAddress, MPU6050_REGISTER_FIFO_EN, 0x00);  
  I2C_Write(MPU6050SlaveAddress, MPU6050_REGISTER_INT_ENABLE, 0x01);    I2C_Write(MPU6050SlaveAddress, MPU6050_REGISTER_SIGNAL_PATH_RESET,   0x00);   I2C_Write(MPU6050SlaveAddress, MPU6050_REGISTER_USER_CTRL, 0x00);
}
 
