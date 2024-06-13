#include <ArduinoJson.h>
#include <IRremote.h>

// Définition des broches pour les moteurs
const int broche_demarrer_arreter_1 = 5;  // Broche pour démarrer/arrêter le moteur 1
const int broche_vitesse_1 = 6;           // Broche pour la vitesse du moteur 1 (PWM)
const int broche_demarrer_arreter_2 = 3;  // Broche pour démarrer/arrêter le moteur 2
const int broche_vitesse_2 = 4;           // Broche pour la vitesse du moteur 2 (PWM)
const int IR_RECEIVE_PIN = 12;            // Broche pour recevoir les signaux IR

IRrecv irrecv(IR_RECEIVE_PIN);
decode_results results;

int vitesse1 = 0;
int vitesse2 = 0;

void setup() {
  pinMode(broche_demarrer_arreter_1, OUTPUT);
  pinMode(broche_vitesse_1, OUTPUT);
  pinMode(broche_demarrer_arreter_2, OUTPUT);
  pinMode(broche_vitesse_2, OUTPUT);

  Serial.begin(115200);
  while (!Serial);  // Attend que la connexion série soit établie
  Serial.println(F("Démarrage du système - Configuration initiale complète."));
  
  irrecv.enableIRIn(); // Démarre le récepteur
  Serial.print("Prêt à recevoir des signaux IR à la broche ");
  Serial.println(IR_RECEIVE_PIN);
}

void loop() {
  if (irrecv.decode(&results)) {
    if (results.value != 0xFFFFFFFF) {  // Ignore les codes de répétition
      handleIRCommand(results.value);
    }
    irrecv.resume(); // Prépare pour la prochaine valeur
  }

  if (Serial.available() > 0) {
    handleSerialCommand();
  }
}

void handleIRCommand(unsigned long command) {
  Serial.print("Commande IR reçue: ");
  Serial.println(command, HEX);
  switch (command) {
    case 0xFF22DD: // Augmenter la vitesse du moteur 1
      vitesse1 = min(255, vitesse1 + 10);
      Serial.print("Augmenter la vitesse du moteur 1 à ");
      Serial.println(vitesse1);
      break;
    case 0xFFE01F: // Diminuer la vitesse du moteur 1
      vitesse1 = max(0, vitesse1 - 10);
      Serial.print("Diminuer la vitesse du moteur 1 à ");
      Serial.println(vitesse1);
      break;
    case 0xFFC23D: // Augmenter la vitesse du moteur 2
      vitesse2 = min(255, vitesse2 + 10);
      Serial.print("Augmenter la vitesse du moteur 2 à ");
      Serial.println(vitesse2);
      break;
    case 0xFF906F: // Diminuer la vitesse du moteur 2
      vitesse2 = max(0, vitesse2 - 10);
      Serial.print("Diminuer la vitesse du moteur 2 à ");
      Serial.println(vitesse2);
      break;
    case 0xFFA25D: // Arrêter les deux moteurs
      vitesse1 = 0;
      vitesse2 = 0;
      Serial.println("Arrêt des deux moteurs");
      break;
    case 0xFF9867: // Égaliser les vitesses des roues
      int maxSpeed = max(vitesse1, vitesse2);
      vitesse1 = vitesse2 = maxSpeed;
      Serial.print("Égalisation des vitesses à ");
      Serial.println(maxSpeed);
      break;
    default:
      Serial.println("Commande non reconnue.");
      break;
  }
  updateMotors(); // Mettre à jour les moteurs après chaque commande IR
}

void handleSerialCommand() {
  StaticJsonDocument<400> doc;
  DeserializationError error = deserializeJson(doc, Serial);
  if (error) {
    Serial.print("Erreur de décodage JSON: ");
    Serial.println(error.c_str());
    return;
  }

  vitesse1 = rpmVersPwm(doc["speed1"].as<int>());
  vitesse2 = rpmVersPwm(doc["speed2"].as<int>());
  updateMotors();
}

void updateMotors() {
  Serial.print("Mise à jour des moteurs - Moteur 1 à la vitesse ");
  Serial.println(vitesse1);
  Serial.print("Mise à jour des moteurs - Moteur 2 à la vitesse ");
  Serial.println(vitesse2);
  
  analogWrite(broche_vitesse_1, vitesse1);
  digitalWrite(broche_demarrer_arreter_1, vitesse1 > 0 ? HIGH : LOW);

  analogWrite(broche_vitesse_2, vitesse2);
  digitalWrite(broche_demarrer_arreter_2, vitesse2 > 0 ? HIGH : LOW);
}

int rpmVersPwm(int rpm) {
  return (int)((rpm + 86.06) / 6.27);
}
