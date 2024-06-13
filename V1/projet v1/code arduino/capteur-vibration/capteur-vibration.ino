#include <Wire.h>
#include <MPU6050.h>

MPU6050 mpu;

unsigned long lastTime = 0;
unsigned long intervalTime = 0;
float az;
float vz = 0; // vitesse initiale
float dz = 0; // déplacement initial

void setup() {
  Serial.begin(115200);
  Wire.begin(D2, D1); // D2: SDA, D1: SCL

  Serial.println("Initialisation du MPU-6050...");
  mpu.initialize();
  if (mpu.testConnection()) {
    Serial.println("MPU-6050 connecté.");
  } else {
    Serial.println("Échec de la connexion au MPU-6050.");
    while (1);
  }
  
  lastTime = millis();
  intervalTime = millis();
}

void loop() {
  ESP.wdtFeed();

  // Lecture valeurs accélération
  int16_t rawAx, rawAy, rawAz;
  mpu.getAcceleration(&rawAx, &rawAy, &rawAz);

  // Conversion en g puis en m/s^2
  az = rawAz / 16384.0;
  float az_mps2 = az * 9.81;

  // Calcule du temps écoulé
  unsigned long currentTime = millis();
  float dt = (currentTime - lastTime) / 1000.0;
  lastTime = currentTime;

  // Intégration pour obtenir les deplacements
  vz += az_mps2 * dt;
  dz += vz * dt;
  float dz_mm = dz * 1000;

  // Ajout d'un seuil
  if (abs(dz_mm) < 0.01) dz_mm = 0;

  // Affichage des déplacements en millimètres toutes les 5 ms
    if (currentTime - intervalTime >= 5ms) {
    Serial.print("Déplacement en Z : "); Serial.print(dz_mm); Serial.println(" mm");

    vz = 0;
    dz = 0;
    intervalTime = currentTime;
  }

  delay(1);
}