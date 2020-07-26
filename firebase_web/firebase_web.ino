// 노드MCU 및 파이어베이스, NTP 라이브러리 추가
#include <NTPClient.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <FirebaseArduino.h>

// DHT11 설정
#define DHTPIN D4
#define DHTTYPE DHT11
#include <DHT.h>
DHT dht(DHTPIN, DHTTYPE);

// NTP 서버 설정
WiFiUDP udp;
// 한국시간은 +9 이므로, 3600초(1시간) * 9로 32400을 입력
NTPClient timeClient(udp, "kr.pool.ntp.org", 32400, 3600000);

#define FIREBASE_HOST "last-firebase-30d35.firebaseio.com"
#define FIREBASE_AUTH "50946uML8Y2xGIts7rZtAP7XniGBYJoofU4AKUXQ"
#define WIFI_SSID "U+Net38A0"
#define WIFI_PASSWORD "5165000493"
// 날짜를 받을 구조체 선언
struct tm * ptm;

void setup() {
  Serial.begin(9600);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD); // 와이파이 설정 값을 기반으로 실행
  dht.begin(9600);
  
  Serial.print("connecting");
  while (WiFi.status() != WL_CONNECTED) { // 와이파이 연결없을 시 실행
    Serial.print(".");
    delay(500);
  }
  Serial.println();
  Serial.print("connected: ");
  Serial.println(WiFi.localIP()); // 접속 된 와이파이의 아이피 출력

  timeClient.begin(); // NTP 실행
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH); // 설정한 데이터베이스를 기반으로 실행
}

void loop() {
  timeClient.update();
  // 넘겨줄 변수들 초기화
  float temp = dht.readTemperature();
  float humi = dht.readHumidity();
  int date_data = timeClient.getEpochTime(); // NTP 라이브러리 내에 getEpochTime을 이용하여 시간 받아오기
  ptm = gmtime((time_t *)&date_data); // 구조체로 각 월,일,시,분,초를 저장
  int year = (ptm->tm_year + 1900);
  int month = (ptm->tm_mon + 1);
  int day = ptm->tm_mday;
  int hour = ptm->tm_hour;
  int minute = ptm->tm_min;
  int sec = ptm->tm_sec;
  String date = (String)year + "-" + (String)month + "-" + (String)day + " " + (String)hour + ":" + (String)minute + ":" + (String)sec;

  // 데이터베이스로 업로드
  StaticJsonBuffer<200> jsonbuffer;
  JsonObject& root = jsonbuffer.createObject();
  root["date"] = date;
  root["temperature"] = temp;
  root["humidity"] = humi;
  String name = Firebase.push("logDHT", root);
  
  // 실패 시 에러 출력
  if (Firebase.failed()) {
  Serial.print("pushing /logs failed:");
  Serial.println(Firebase.error());
  return;
  }
  Serial.print("pushed: /logDHT/ \n");
  delay(1000);
}
