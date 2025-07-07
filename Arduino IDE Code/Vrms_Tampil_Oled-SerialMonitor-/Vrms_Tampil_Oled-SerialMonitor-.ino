#include <Wire.h>
#include <math.h>
#include "arduinoFFT.h"
#include <SimpleKalmanFilter.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>

// === Konstanta ===
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define MPU6050_I2C_ADDRESS 0x68
#define ACCEL_SENSITIVITY 16384.0
#define SAMPLING_RATE 100
#define DT (1.0 / SAMPLING_RATE)
#define NUM_SAMPLES 128
#define MA_WINDOW_SIZE 7
#define HISTORY_SIZE 6
#define RMS_INTERVAL 60000  // ms

// === Inisialisasi objek ===
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
SimpleKalmanFilter kalmanX(2, 2, 0.01);
SimpleKalmanFilter kalmanY(2, 2, 0.01);
SimpleKalmanFilter kalmanZ(2, 2, 0.01);
arduinoFFT FFT;

double vReal[NUM_SAMPLES];
double vImag[NUM_SAMPLES];
uint8_t fftIndex = 0;

float ax_prev = 0, ay_prev = 0, az_prev = 0;
float ax_hpf = 0, ay_hpf = 0, az_hpf = 0;
float velocityRMSHistory[HISTORY_SIZE] = {0};
float prevVelocityRMS = 0;
int historyIndex = 0;
int16_t ax_offset = 0, ay_offset = 0, az_offset = 0;

// WiFi
const char* ssid = "Ifta";
const char* password = "12345678";
const char* server = "192.168.0.104";

void sendDataToServer(float velocityRMS);

// === Fungsi level dan status motor ===
uint8_t levelGet(float v) {
  return (v <= 1.8) ? 1 : (v <= 2.8) ? 2 : (v <= 7.1) ? 3 : 4;
}

String getStatusMotor(float v) {
  uint8_t lvl = levelGet(v);
  switch (lvl) {
    case 1: return "BAIK";
    case 2: return "WASPADA";
    case 3: return "BAHAYA";
    default: return "KRITIS";
  }
}

// === Setup ===
void setup() {
  Serial.begin(115200);
  Wire.begin();

  // WiFi
  WiFi.begin(ssid, password);
  Serial.print("Menghubungkan ke WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println(" Terhubung!");

  // OLED
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("OLED gagal"));
    while (true);
  }
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println("Inisialisasi...");
  display.display();

  // MPU6050
  Wire.beginTransmission(MPU6050_I2C_ADDRESS);
  Wire.write(0x6B);
  Wire.write(0x00);
  Wire.endTransmission();

  calibrateAccelerometer();
}

// === Loop utama ===
void loop() {
  static unsigned long lastSample = 0;
  static unsigned long lastRMSUpdate = 0;

  if (millis() - lastSample < (1000 / SAMPLING_RATE)) return;
  lastSample = millis();

  int16_t ax_raw, ay_raw, az_raw;
  readAccelerometer(ax_raw, ay_raw, az_raw);

  float ax = kalmanX.updateEstimate((ax_raw - ax_offset) * (9.81 / ACCEL_SENSITIVITY));
  float ay = kalmanY.updateEstimate((ay_raw - ay_offset) * (9.81 / ACCEL_SENSITIVITY));
  float az = kalmanZ.updateEstimate((az_raw - az_offset) * (9.81 / ACCEL_SENSITIVITY));

  // High-pass filter
  const float alpha = 0.98;
  ax_hpf = alpha * (ax_hpf + ax - ax_prev);
  ay_hpf = alpha * (ay_hpf + ay - ay_prev);
  az_hpf = alpha * (az_hpf + az - az_prev);
  ax_prev = ax;
  ay_prev = ay;
  az_prev = az;

  float velocity = sqrt(sq(ax_hpf * DT) + sq(ay_hpf * DT) + sq(az_hpf * DT)) * 1000;
  float velocityFiltered = movingAverage(velocity);

  // Simpan untuk FFT
  vReal[fftIndex] = velocityFiltered;
  vImag[fftIndex] = 0;
  fftIndex = (fftIndex + 1) % NUM_SAMPLES;
  if (fftIndex == 0) performFFT();

  // Update RMS per interval
  if (millis() - lastRMSUpdate >= RMS_INTERVAL) {
    lastRMSUpdate = millis();

    float velocityRMS = calculateVelocityRMS();
    velocityRMSHistory[historyIndex] = velocityRMS;
    historyIndex = (historyIndex + 1) % HISTORY_SIZE;

    float sum = 0;
    for (float v : velocityRMSHistory) sum += v;
    float velocityRMSAvg = sum / HISTORY_SIZE;

    if (abs(velocityRMSAvg - prevVelocityRMS) > (prevVelocityRMS * 0.05)) {
      Serial.print("Velocity RMS: ");
      Serial.print(velocityRMSAvg, 3);
      Serial.println(" mm/s");

      String status = getStatusMotor(velocityRMSAvg);

      // === TAMPILAN OLED SESUAI FOTO ===
      display.clearDisplay();
      display.setTextColor(SSD1306_WHITE);
      display.setTextSize(1);

      // Judul
      display.setCursor(20, 0);
      display.print("Motor - ");
      display.print(status);

      // Label Vrms
      display.setCursor(0, 20);
      display.print("Vrms:");

      // Nilai
      display.setCursor(40, 20);
      display.print(velocityRMSAvg, 3);
      display.print(" mm/s");

      display.display();

      prevVelocityRMS = velocityRMSAvg;

      // Kirim data ke server
      sendDataToServer(velocityRMSAvg);
    }
  }
}

// === Fungsi pendukung ===
void calibrateAccelerometer() {
  const int samples = 1000;
  int32_t sumX = 0, sumY = 0, sumZ = 0;
  for (int i = 0; i < samples; i++) {
    int16_t ax, ay, az;
    readAccelerometer(ax, ay, az);
    sumX += ax;
    sumY += ay;
    sumZ += az;
    delay(1);
  }
  ax_offset = sumX / samples;
  ay_offset = sumY / samples;
  az_offset = sumZ / samples;
}

void readAccelerometer(int16_t &ax, int16_t &ay, int16_t &az) {
  Wire.beginTransmission(MPU6050_I2C_ADDRESS);
  Wire.write(0x3B);
  Wire.endTransmission(false);
  Wire.requestFrom(MPU6050_I2C_ADDRESS, 6);
  ax = Wire.read() << 8 | Wire.read();
  ay = Wire.read() << 8 | Wire.read();
  az = Wire.read() << 8 | Wire.read();
}

float movingAverage(float newValue) {
  static float buffer[MA_WINDOW_SIZE] = {0};
  static int index = 0;
  static float sum = 0;

  sum -= buffer[index];
  buffer[index] = newValue;
  sum += newValue;

  index = (index + 1) % MA_WINDOW_SIZE;
  return sum / MA_WINDOW_SIZE;
}

float calculateVelocityRMS() {
  float sumSq = 0;
  for (int i = 0; i < NUM_SAMPLES; i++) {
    sumSq += vReal[i] * vReal[i];
  }
  return sqrt(sumSq / NUM_SAMPLES);
}

void performFFT() {
  FFT.Windowing(vReal, NUM_SAMPLES, FFT_WIN_TYP_HAMMING, FFT_FORWARD);
  FFT.Compute(vReal, vImag, NUM_SAMPLES, FFT_FORWARD);
  FFT.ComplexToMagnitude(vReal, vImag, NUM_SAMPLES);
  double peak = FFT.MajorPeak(vReal, NUM_SAMPLES, SAMPLING_RATE);
  // Optional: Serial.println(peak);
}

void sendDataToServer(float velocityRMS) {
  WiFiClient client;
  const int httpPort = 80;

  if (!client.connect(server, httpPort)) {
    Serial.println("Gagal terkoneksi ke web server");

    // TAMPILKAN STATUS GAGAL DI OLED
    display.setCursor(0, 40);
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.print("Status: GAGAL koneksi");
    display.display();
    return;
  }

  HTTPClient http;
  String link = "http://" + String(server) + "/getaran/kirimdata.php?getaran=" + String(velocityRMS, 3);
  http.begin(client, link);
  int httpResponseCode = http.GET();

  display.setCursor(0, 40);
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  if (httpResponseCode > 0) {
    Serial.println("Berhasil mengirim data ke server!");
    String response = http.getString();
    Serial.println("Response: " + response);

    display.print("Status: Terkirim");
  } else {
    Serial.print("Gagal mengirim data. Kode error: ");
    Serial.println(httpResponseCode);

    display.print("Status: Error ");
    display.print(httpResponseCode);
  }

  display.display(); // Pastikan tampilan OLED diupdate
  http.end();
}
