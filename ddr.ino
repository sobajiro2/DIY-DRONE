//#include "FS.h"
#include "SPIFFS.h"
#include "BluetoothSerial.h"
#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
#endif
BluetoothSerial SerialBT;

#include <Wire.h>
#define MPU6050_ADDR         0x68 // MPU-6050 device address
#define MPU6050_SMPLRT_DIV   0x19 // MPU-6050 register address
#define MPU6050_CONFIG       0x1a
#define MPU6050_GYRO_CONFIG  0x1b
#define MPU6050_ACCEL_CONFIG 0x1c
#define MPU6050_WHO_AM_I     0x75
#define MPU6050_PWR_MGMT_1   0x6b
int32_t  cgyrox,cgyroy,cgyroz,calmotx = 0,calmoty = 0,calmotz = 0; // cal giroxyz,cal motor xy 
int32_t  cax,cay;
int32_t raw_acc_x, raw_acc_y, raw_acc_z, raw_t, gyrox, gyroy, gyroz ;
char bdat[] = {0,0,0,0,0,0,0,0,0,0,0,0}; // flash memory BYTE data
int fst; // フラッシュメモリ状態　flash memory status
//Write I2c
void writeMPU6050(byte reg, byte data) {
  Wire.beginTransmission(MPU6050_ADDR);
  Wire.write(reg);
  Wire.write(data);
  Wire.endTransmission();
}
//Read i2C
byte readMPU6050(byte reg) {
  Wire.beginTransmission(MPU6050_ADDR);
  Wire.write(reg);
  Wire.endTransmission(true);
  Wire.requestFrom(MPU6050_ADDR, 1/*length*/); 
  byte data =  Wire.read();
  return data;
}
int SHT31_Addr = 0x45; // Address set SHT31
int p;
void setup() {
Serial.begin(115200);
SerialBT.begin("ESPTX");
      //SPIFFS.format();
      // フラッシュメモリ 初回フォーマット時のみ有効にする（次回書き込みからは無効にする）Flash memory Enable only when formatting for the first time (disable from next writing)
  if(!SPIFFS.begin()){
    fst |= B100; //"SPIFFS initialization failed!");  // エラー error
  }
// フラッシュメモリ読み出し Flash memory read
  String file_name = "/cal.txt";  // ファイル名の指定 Specifying the file name
  if (SPIFFS.exists(file_name)) {  // ファイルが存在すれば if the file exists
    File file = SPIFFS.open(file_name, "r"); // ファイルオープン file open
    if(!file){  // ファイルが開けなければ
        fst |= B10; // Failed to open file"); // エラー error
      return;
    } else {    // ファイルが開ければ
        fst |= B1000; // Open file to read");   // 読み取り実行表示 Read execution display
    }
    int i = 0;
    while(file.available()){
      bdat[i] = file.read(); // ファイルから1バイトずつデータを読み取り Read data one byte at a time from the file
      i++;
      if(i > 11)break;
    } 
  // for(i = 0;i < 3;i++){
  // SerialBT.println( *((int32_t*)( (int)&bdat)+i));
  // }
  calmotx = *((int32_t*)( (int)&bdat)+0);
  calmoty = *((int32_t*)( (int)&bdat)+1);
  calmotz = *((int32_t*)( (int)&bdat)+2);
      file.close(); 
    } else {  // ファイルが存在しなければ if the file does not exist
        fst |= B1; // File does not exist");  // エラー error
    }
 





pinMode(25,OUTPUT); //Setting motor pwm
ledcAttach(25,40000,8);
pinMode(32,OUTPUT);
ledcAttach(32,40000,8);
pinMode(27,OUTPUT);
ledcAttach(27,40000,8);
pinMode(12,OUTPUT);
ledcAttach(12,40000,8);

Wire.begin();
Wire.beginTransmission(MPU6050_ADDR);
Wire.write(0x6B); // PWR_MGMT_1 register
Wire.write(0); // set to zero (wakes up the MPU-6050)
Wire.endTransmission(true);

delay(100);
Serial.println("a");
//Connect ok check
if (readMPU6050(MPU6050_WHO_AM_I) != 0x68) {
  Serial.println("\nGYRO WHO_AM_I error.");
  while (true) ;
}
//setting
writeMPU6050(MPU6050_SMPLRT_DIV, 0x00);   // sample rate: 8kHz/(7+1) = 1kHz
writeMPU6050(MPU6050_CONFIG, 0x00);       // 0x00 0x06 disable DLPF, gyro output rate = 8kHz
writeMPU6050(MPU6050_GYRO_CONFIG, 0x08);  // gyro range: ±500dps
writeMPU6050(MPU6050_ACCEL_CONFIG, 0x00); // accel range: ±2g
writeMPU6050(MPU6050_PWR_MGMT_1, 0x01);   // disable sleep mode, PLL with X gyro
//Calibration
delay(1000);
//int32_t raw_acc_x, raw_acc_y, raw_acc_z, raw_t, raw_gyro_x, raw_gyro_y, raw_gyro_z ;
gcal();
ledcWrite(25,0); //motor off 
ledcWrite(32,0);
ledcWrite(27,0);
ledcWrite(12,0);
}
int16_t fl = 0;
int16_t fr = 0;
int16_t rl = 0;
int16_t rr = 0;
int rxdt[5];
int rxd[4];
int rxdc = 0;
char rxt0;
char rxr0;
char rxy0;
char rxp0;
int16_t rxt;
int16_t rxr;
int16_t rxy;
int16_t rxp;
char rxcom1;
char rxcom11;
char rxcom2;
char htryp;
char rxsum;
int lms;
long lastus;
int rxrrl = 0; //前回のロール previous roll
int rxppl = 0;  //前回のピッチ previous pitch
int rxpdif; //ピッチ絶対値変化　操作後の戻り防止 Pitch absolute value change Prevention of return after operation
int rxrdif; //ロール絶対値変化　操作後の戻り防止 Roll absolute value change Prevention of return after operation
int rxpd3;
int rxpd2;
int rxpd1;
int rxrd3;
int rxrd2;
int rxrd1;

// 各係数　割り算なので普通と逆　kd 以外小さいほうがよく効く
// Since each coefficient is a division, it works better if it is smaller, except for normal and inverse kd.
int avgn = 32;  // moving average 2,4,8,16,32
int gyrat = 60; //ジャイロ効き具合係数 Gyro effectiveness coefficient
int rprat = 100; // ロールピッチ効き具合係数 roll pitch effectiveness coefficient
int yrat = 8; //ヨー効き具合係数 yaw effectiveness coefficient
int16_t kd = 6; //PID　の　D係数 D coefficient of PID
int arat = 20;  // acc coefficient Number of right shifts
int akd = 2;  // acc D coefficient of PID
int rparat = 40;  // ロールピッチ効き具合係数 roll pitch effectiveness coefficient
int realm = 0; // realtime monitor 0 no  1 yes
// 

int stout = 0;
int stoutc = 0;
int okcnt = 0; // ok count serialBT ok 1 throttle lo 2 
int32_t gx[32],gy[32],gz[32],ax[32],ay[32]; // ジャイロアクセル移動平均用 For gyro acc moving average
int avg = 0;
int16_t gdifxi = 0; //目標との差x　積分計算用 Difference from target x for integral calculation
int16_t gdifxd = 0; //目標との差x　微分計算用 Difference x from target for differential calculation
int16_t lgdifx; //目標との差x  Difference x
int16_t gdifyi = 0;
int16_t gdifyd = 0;
int16_t lgdify;
int16_t gdifzi = 0;
int16_t gdifzd = 0;
int16_t lgdifz;
int16_t adifxd = 0; // differential
int16_t ladifx;
int16_t adifyd = 0; // differential
int16_t ladify;
int32_t deadzone = 200;
int lus = 0;
int caflg = 0;
int onsec = 0; //power on second
int flysec = 0; //fly second
int32_t seccnt = 0; //second counter
int stcentf = 0; // yaw stick center flag
int strpcentf = 0; // roll pitch stick center flag
int pwait;
int lmics;
int ratemode = 1;
int lratemode = 0;

void loop() {
if(lratemode != ratemode){
  if(ratemode == 1){
    //-------------ratemode-----------------
    avgn = 8;
    gyrat = 8;  // ジャイロ効き具合係数 Gyro effectiveness coefficient
    rprat = 10;  // ロールピッチ効き具合係数 roll pitch effectiveness coefficient
    yrat = 3;  // ヨー効き具合係数 yaw effectiveness coefficient
    kd = 3;   // PID　の　D係数 D coefficient of PID
    arat = 16; // acc coefficient Number of right shifts
    akd = 0;  // acc D coefficient of PID
    rparat = 250; // ロールピッチ効き具合係数 roll pitch effectiveness coefficient
    writeMPU6050(MPU6050_CONFIG, 0x00);
  }else{
    //-------------levelmode-----------------
    avgn = 2;
    gyrat = 70; //ジャイロ効き具合係数 Gyro effectiveness coefficient
    rprat = 200; // ロールピッチ効き具合係数 roll pitch effectiveness coefficient
    yrat = 8; //ヨー効き具合係数 yaw effectiveness coefficient
    kd = 0; //PID　の　D係数 D coefficient of PID
    arat = 14; // acc coefficient Number of right shifts
    akd = 2;  // acc D coefficient of PID
    rparat = 60; // ロールピッチ効き具合係数 roll pitch effectiveness coefficient
    writeMPU6050(MPU6050_CONFIG, 0x03);
  }
}
lratemode = ratemode;



int wtmout = 0;
rxcom11 = 0;
while (SerialBT.available() ) {
    rxt0 = rxr0;
    rxr0 = rxy0;
    rxy0 = rxp0;
    rxp0 = htryp;
    htryp = rxcom1;
    rxcom1 = rxcom2;
    rxcom2 = rxsum;
    rxsum = SerialBT.read();

      if(((rxt0 & 128) == 0) && ((rxr0 & 128) == 0) && ((rxy0 & 128) == 0) && ((rxp0 & 128) == 0) && ((htryp & 128) == 128)){
          int rxtt = rxt0 | ((htryp << 4)& 128);
          int rxrr = rxr0 | ((htryp << 3)& 128);
          int rxyy = rxy0 | ((htryp << 2)& 128);
          int rxpp = rxp0 | ((htryp << 1)& 128);

          lms = micros();
          if((int)rxsum == ((rxtt + rxrr + rxyy + rxpp + htryp + rxcom1 + rxcom2) & 255) ){
                rxt = rxtt; // throttle 
                rxr = rxrr - 128; // roll leftMAX:-128
                rxy = rxyy - 128; // yo leftMAX:+128
                rxp = rxpp - 128; // pitch temaeMAX:-128
                rxcom11 = rxcom1;
          }else{
          // Serial.print("e");  //error
          }
          rxrdif = abs(rxr) - rxrrl;
          rxpdif = abs(rxp) - rxppl;
          rxrrl = abs(rxr);
          rxppl = abs(rxp); 
      }
  wtmout++;
  if(wtmout > 10)break;
}
if(stcentf == 1 && rxt < 3 && rxr < -123 && rxy > 123 && rxp < -123){ // left stick right-down right stick left-down gyro calibration
  stcentf = 0;
  ratemode = ratemode ^ 1; //xor 1   reverse bit1 
  SerialBT.print("\nRATEMODE:");
  SerialBT.print(ratemode);
  SerialBT.print("\t");
  
  SerialBT.print(cgyrox);
  SerialBT.print("\t");
  SerialBT.print(cgyroy);
  SerialBT.print("\t");
  SerialBT.print(cgyroz);
  gcal();
  SerialBT.print(cgyrox);
  SerialBT.print("\t");
  SerialBT.print(cgyroy);
  SerialBT.print("\t");
  SerialBT.println(cgyroz);
  SerialBT.println("flash status 1;OpenOK;InitERR;OpenERR;FileNotExist ");
  SerialBT.println(fst,BIN);
    pwait = 800;
}

if(strpcentf == 1 && rxt < 3 && rxy < -123 && rxp > -3 && rxp < 3){
  if(rxr < -123){
  strpcentf = 0;
  calmotx += 1;
  caflg = 1000;
  }
  if(rxr > 123){
  strpcentf = 0;
  calmotx -= 1;
  caflg = 1000;
  }
}
if(strpcentf == 1 && rxt < 3 && rxy < -123 && rxr > -3 && rxr < 3){
  if(rxp < -123){
  strpcentf = 0;
  calmoty += 1;
  caflg = 1000;
  }
  if(rxp > 123){
  strpcentf = 0;
  calmoty -= 1;
  caflg = 1000;
  }
}
if(stcentf == 1 && rxt < 3 && rxp > 123 && rxr > 123){
  if(rxy < -123){
  stcentf = 0;
  calmotz -= 1;
  caflg = 1000;
  }
  if(rxy > 123){
  stcentf = 0;
  calmotz += 1;
  caflg = 1000;
  }
}
if(caflg > 1)caflg--;
if(caflg == 1){
  caflg = 0;
  SerialBT.print(calmotx);
  SerialBT.print("\t");
  SerialBT.print(calmoty);
  SerialBT.print("\t");
  SerialBT.println(calmotz);
//　フラッシュメモリへ書き込み Write to flash memory
  int wdat[12];
  wdat[0] = calmotx;
  wdat[1] = calmoty;
  wdat[2] = calmotz;
        File file = SPIFFS.open("/cal.txt", "w"); // ファイルを作成して開く create and open file
  if(!file){
    SerialBT.println("Failed to create file");
    return;
  } else {
    SerialBT.println("Create file"); 
  }
   // ファイルにデータを書き込む write data to file
  for(int i = 0;i < 12;i++){
  file.print((char)*((int32_t*)( (int)&wdat+i)));             // 文字列書き込み実行 String writing execution
  }
  file.close();
    pwait = 800;
}
if(rxr > -3 && rxr < 3 && rxp > -3 && rxp < 3 )strpcentf = 1; // Once the stick is centered, accept the next instruction.
if(rxy > -3 && rxy < 3)stcentf = 1; // Once the stick is centered, accept the next instruction.
// SeriaBT command reception
  char ratchflg = 0;
  if(rxcom11 == 1){
    gyrat = rxcom2;
    ratchflg = 1;
    }
  if(rxcom11 == 2){
    rprat = rxcom2;
    ratchflg = 1;
    }
  if(rxcom11 == 3){
    yrat = rxcom2;
    ratchflg = 1;
    }

  if(rxcom11 == 4){
    if(rxcom2 != 255)  kd = rxcom2;
    ratchflg = 1;
    }
  if(rxcom11 == 5){
    arat = rxcom2;
    ratchflg = 1;
    }
  if(rxcom11 == 6){
    akd = rxcom2;
    ratchflg = 1;
    }
    if(rxcom11 == 7){
    rparat = rxcom2;
    ratchflg = 1;
    }
    if(rxcom11 == 8){
    realm = rxcom2;
    ratchflg = 1;
    }
    if(rxcom11 == 9){
      if(rxcom2 == 2 || rxcom2 == 4 || rxcom2 == 8 || rxcom2 == 16 || rxcom2 == 32)avgn = rxcom2;
    ratchflg = 1;
    }
  if(ratchflg == 1){
    ratchflg = 0;
    SerialBT.print("\t1gyrat:");
    SerialBT.print(gyrat);
    SerialBT.print("\t2rprat:");
    SerialBT.print(rprat);
    SerialBT.print("\t3yrat:"); 
    SerialBT.print(yrat);
    SerialBT.print("\t4kd:");
    SerialBT.print(kd);
    SerialBT.print("\t5arat:");
    SerialBT.print(arat);
    SerialBT.print("\t6akd:");
    SerialBT.print(akd);
    SerialBT.print("\t7rparat:");
    SerialBT.print(rparat);
    SerialBT.print("\t8realm:");
    SerialBT.print(realm);
    SerialBT.print("\t9 avgn:");
    SerialBT.print(avgn);
    pwait = 800;

  }
if(micros() - lms > 100000){
  if(stoutc < 3){  // Motor stop if serialBT timeout  lms:last rx microsec
  stoutc++;
  }
  }else{
  stoutc = 0;
  if(okcnt == 0)okcnt = 1;
  }
    //レジスタアドレス0x3Bから、計14バイト分のデータを出力するようMPU6050へ指示 下記4行1.6msかかる
    //Instructs MPU6050 to output a total of 14 bytes of data from register address 0x3B The following 4 lines take 1.6ms
  Wire.beginTransmission(MPU6050_ADDR);
  Wire.write(0x3B);
  Wire.endTransmission(false);
  Wire.requestFrom(MPU6050_ADDR, 14, true);

  //出力されたデータを読み込み、ビットシフト演算 Read the output data and perform bit shift operation
  int16_t axx = Wire.read() << 8 | Wire.read();
  int16_t ayy = Wire.read() << 8 | Wire.read();
  raw_acc_z = Wire.read() << 8 | Wire.read();
  raw_t = Wire.read() << 8 | Wire.read();
  int16_t xx = (Wire.read() << 8 | Wire.read());
  int16_t yy = (Wire.read() << 8 | Wire.read());
  int16_t zz = (Wire.read() << 8 | Wire.read());
  ax[avg] = axx;
  ay[avg] = ayy;
  gx[avg] = xx;
  gy[avg] = yy;
  gz[avg] = zz;
 // int avgn = 32; // Moving average number 4,8,16...
  avg = (avg + 1) & (avgn - 1);
  int32_t acc_x = 0;
  int32_t acc_y = 0;
  gyrox = 0;
  gyroy = 0;
  gyroz = 0;
   for(int i = 0;i < avgn;i++){
  acc_x += ax[i];
  acc_y += ay[i];
  gyrox += gx[i];
  gyroy += gy[i];
  gyroz += gz[i];
  }
  acc_x = acc_x / avgn - cax;
  acc_y = acc_y / avgn - cay;
  gyrox = gyrox / avgn - cgyrox;
  gyroy = gyroy / avgn - cgyroy;  // - cagtroy 241008kesita
  gyroz = gyroz / avgn - cgyroz;

if(gyrat == 0)gyrat = 1;
if(rprat == 0)rprat = 1;
if(yrat == 0)yrat = 1;
int16_t gdifx = ((gyrox >> 2) / gyrat) - ((rxr << 4) / rprat);
int16_t gdify = ((gyroy >> 2) / gyrat) - ((rxp << 4) / rprat);
int16_t gdifz = ((gyroz >> 2) / gyrat) + ((rxy << 4) / yrat);
int16_t tt = rxt;

int16_t adifx = (acc_y >> arat) - ((rxr << 4) / rparat);
int16_t adify = (acc_x >> arat) + ((rxp << 4) / rparat);

adifxd = ladifx - adifx; // differential
ladifx = adifx;
adifyd = ladify - adify; // differential
ladify = adify;

gdifxi += gdifx; // integral
if(tt < 2)gdifxi = 0;
gdifxd = lgdifx - gdifx; // differential
lgdifx = gdifx;
gdifyi += gdify; // integral
if(tt < 2)gdifyi = 0;
gdifyd = lgdify - gdify; // differential
lgdify = gdify;
gdifzi += gdifz; // integral
if(tt < 2)gdifzi = 0;
gdifzd = lgdifz - gdifz; // differential
lgdifz = gdifz;

if((rxpd3 + rxpd2 + rxpd1 + rxpdif) / 4 < 0)gdifxd = 0;// Moving average
rxpd3 = rxpd2;
rxpd2 = rxpd1;
rxpd1 = rxpdif;
if((rxrd3 + rxrd2 + rxrd1 + rxrdif) / 4 < 0)gdifyd = 0;// Moving average
rxrd3 = rxrd2;
rxrd2 = rxrd1;
rxrd1 = rxrdif;

fl = tt - adifx + adifxd * akd -calmotx- gdifx + gdifxd * kd - adify + adifyd * akd +calmoty+ gdify - gdifyd * kd - gdifz - gdifzd * 10 - calmotz;
fr = tt + adifx - adifxd * akd +calmotx+ gdifx - gdifxd * kd - adify + adifyd * akd +calmoty+ gdify - gdifyd * kd + gdifz + gdifzd * 10 + calmotz;
rl = tt - adifx + adifxd * akd -calmotx- gdifx + gdifxd * kd + adify - adifyd * akd -calmoty- gdify + gdifyd * kd + gdifz + gdifzd * 10 + calmotz;
rr = tt + adifx - adifxd * akd +calmotx+ gdifx - gdifxd * kd + adify - adifyd * akd -calmoty- gdify + gdifyd * kd - gdifz - gdifzd * 10 - calmotz;
if(fl<0||tt<1)fl=0;
if(fr<0||tt<1)fr=0;
if(rl<0||tt<1)rl=0;
if(rr<0||tt<1)rr=0;
int plopmax = 255;
if(fl>plopmax)fl=plopmax;
if(fr>plopmax)fr=plopmax;
if(rl>plopmax)rl=plopmax;
if(rr>plopmax)rr=plopmax;

if(okcnt == 1 && tt == 0)okcnt = 2;  //serialBT ok throttle lo okcnt 2

if(okcnt == 2 && stoutc < 3){
    ledcWrite(25,fl);
    ledcWrite(32,fr);
    ledcWrite(27,rl);
    ledcWrite(12,rr);
}else{
    ledcWrite(25,0);
    ledcWrite(32,0);
    ledcWrite(27,0);
    ledcWrite(12,0);
    pwait = 3;
}
if(millis() > seccnt + 1000){
onsec++;
if(okcnt == 2 && stoutc < 3 && tt > 0){
  flysec++;
  if(flysec % 30 == 0){
/*    SerialBT.print("onsec flysec:");
    SerialBT.print(onsec);
    SerialBT.print("\t");
    SerialBT.println(flysec);  */
  }
}
seccnt = millis();
}
if(pwait == 0 && realm == 1){
SerialBT.println();
SerialBT.print(acc_y);
SerialBT.print("\t");
SerialBT.print(acc_x);
SerialBT.print("\t");
SerialBT.print(tt);
SerialBT.print("\t");
SerialBT.print(adifx);
SerialBT.print("\t");
SerialBT.print(adifxd);
SerialBT.print("\t");
SerialBT.print(gdifx);
SerialBT.print("\t");
SerialBT.print(gdifxd);
SerialBT.print("\t");
SerialBT.print(adify);
SerialBT.print("\t");
SerialBT.print(adifyd);
SerialBT.print("\t");
SerialBT.print(gdify);
pwait = 1;
}
if(pwait > 0)pwait--;
while(micros() < (lastus + 2000));
lastus = micros();
}

// gyro calibration
void gcal(){
SerialBT.println("---cal---");
int32_t sumax = 0;
int32_t sumay = 0;
int32_t sumgx = 0;
int32_t sumgy = 0;
int32_t sumgz = 0;
for(int i = 0;i < 1000;i++){
    delay(1);
    Wire.beginTransmission(MPU6050_ADDR);
    Wire.write(0x3B);
    Wire.endTransmission(false);
    Wire.requestFrom(MPU6050_ADDR, 14, true);
    int16_t acc_x = Wire.read() << 8 | Wire.read();
    int16_t acc_y = Wire.read() << 8 | Wire.read();
    raw_acc_z = Wire.read() << 8 | Wire.read();
    raw_t = Wire.read() << 8 | Wire.read();
    int16_t x = Wire.read() << 8 | Wire.read();
    int16_t y = Wire.read() << 8 | Wire.read();
    int16_t z = Wire.read() << 8 | Wire.read();
sumax += acc_x;
sumay += acc_y;
sumgx += x;
sumgy += y;
sumgz += z;
}
  cax = sumax / 1000;
  cay = sumay / 1000;
  cgyrox = sumgx / 1000;
  cgyroy = sumgy / 1000;
  cgyroz = sumgz / 1000;
  stoutc = 0;
}