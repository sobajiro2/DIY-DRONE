//https://rcj-lightweight.hateblo.jp/entry/try_esp32   wo sannkou ni sita
#include "BluetoothSerial.h"

BluetoothSerial SerialBT;

String MACadd = "9c:9c:1f:d3:69:6a";
uint8_t address[6]  = {0x9c, 0x9c, 0x1f, 0xd3, 0x69, 0x6a};
bool connected;
long tt = 0;

void setup() {
  Serial.begin(115200);
  delay(100);
  SerialBT.begin("ESPTX", true); 
    connected = SerialBT.connect(address);
  if (SerialBT.disconnect()) {
  }
  SerialBT.connect();
}
int cent[] = {0x100,0xe6,0xe8,0xe6}; //スティックのセンター
char aport[] = {36,39,34,35};  //アナログポート
char inbuf[10];  //入力バッファ
int ibc = 0;

void loop() {
  Serial.println("ESPSTART");
while(1){
//　シリアルポートにコマンドを入力してドローンへ送る
int com1 = 0;
int com2 = 0;
  int ival;      // シリアル受信整数
  if (Serial.available()) {
  inbuf[ibc] = Serial.read();
  ibc++;
  if(ibc == 1){
    if(inbuf[0] < '0' || (inbuf[0] > '9' && inbuf[0] < 'a') || inbuf[0] > 'z'){  //文字目は0-9,a-zなら受け付ける
        ibc = 0;
        }
  }else{
      if(inbuf[ibc-1] < '0' || inbuf[ibc-1] > '9'){  //　2文字目以降は数字のみ
        ibc = 0;
        }
  }
if (ibc > 3) {  // 文字列の長さが3文字以上の場合
    ibc = 0;
    //　0-9　→　0-9　　a-z　→　10-35
    ival = (inbuf[0] < 0x3a ? inbuf[0] - 0x30 : inbuf[0] - 0x57) * 1000 + (inbuf[1] & 15) * 100 + (inbuf[2] & 15)*10 + (inbuf[3] & 15);
    com1 = ival / 1000;
    com2 = ival % 1000;
    if(com2 > 255)com2 = 255;
    Serial.println(ival);
}
}


int inta[4];
int suma = 0; //　チェックサム
int outah = 0; //　判別用MSB必ず1　各チャネルの最上位ビットを入れる
for(int i = 0;i < 4;i++){
int a = analogRead(aport[i]) >> 3 & 0x1ff;
int outa = (a < cent[i] ? a * 0x100 / cent[i] : (a - cent[i])  * cent[i] / 0xfe + 0x100) / 2;
if(outa > 0xff)outa = 0xff;
suma = suma + outa;
SerialBT.write(outa & 127);
outah = (outah >> 1) | outa & 128;
inta[i] = outa;
}
if(inta[0] < 4 && inta[1] > 251 && inta[2] < 4 && inta[3] < 4){  //逆ハの字でリセット
Serial.println("ESP.restart");
  ESP.restart();
}
outah = (outah >> 1) | 128;
suma = suma + outah;
suma = suma + com1 + com2;

SerialBT.write(outah);
SerialBT.write(com1);
SerialBT.write(com2);
SerialBT.write(suma & 255);

//スキャンタイム10ms待ち　その間　ドローンからの受信内容をシリアルポートへ
while(micros() - tt < 10000){
  while(SerialBT.available()){
    Serial.print((char)SerialBT.read());
  }

}
tt = micros();
}
}

