#include <ArduinoJson.h>

// Broches pour Arduino Mega 2560 vers cartes de puissance pour moteur brushless
int broche_demarrer_arreter_1 = 5;  // Broche ON/OFF moteur 1
int broche_vitesse_1 = 6;           // Broche contrôle vitesse moteur 1

int broche_demarrer_arreter_2 = 8;  // Broche ON/OFF moteur 2
int broche_vitesse_2 = 9;           // Broche contrôle vitesse moteur 2

// Variables pour les vitesses des moteurs en PWM
int vitesse1 = 0;  
int vitesse2 = 0;  

// Conversion RPM en PWM
int rpmVersPwm(float rpm) {
  return (int)((rpm + 86.06) / 6.27);
}

void setup() {
  Serial.begin(115200);  
  Serial.println("Initialisation");

  pinMode(broche_demarrer_arreter_1, OUTPUT); 
  pinMode(broche_vitesse_1, OUTPUT); 

  pinMode(broche_demarrer_arreter_2, OUTPUT); 
  pinMode(broche_vitesse_2, OUTPUT); 
}

// ##################
// Écoute des commandes envoyées via le port série
void loop() {
  
  // exemple : {"vitesse1": 1000, "vitesse2": 1000} valeur en rpm
  if (Serial.available() > 0) {
    String commande = Serial.readString();  
    Serial.print("Commande reçue : ");
    Serial.println(commande);

    // Arrêt des moteurs si la commande contient "STOP" ou "stop"
    if (commande.indexOf("STOP") != -1 || commande.indexOf("stop") != -1) {
      arreterMoteurs();
    } else {
      // Désérialisation de la commande JSON
      StaticJsonDocument<400> doc;
      DeserializationError erreur = deserializeJson(doc, commande);

      if (!erreur) {
        vitesse1 = rpmVersPwm(doc["vitesse1"].as<int>());
        vitesse2 = rpmVersPwm(doc["vitesse2"].as<int>());
        demarrerMoteurs();
      } else {
        Serial.print("Erreur de syntaxe JSON : ");
        Serial.println(erreur.c_str());
      }
    }
  }
}


// ##################
// Fonctions de controle

void demarrerMoteurs() {
  analogWrite(broche_vitesse_1, vitesse1);
  digitalWrite(broche_demarrer_arreter_1, vitesse1 > 0 ? HIGH : LOW);

  analogWrite(broche_vitesse_2, vitesse2);
  digitalWrite(broche_demarrer_arreter_2, vitesse2 > 0 ? HIGH : LOW);

  Serial.print("Moteur 1 PWM : ");
  Serial.println(vitesse1);
  Serial.print("Moteur 2 PWM : ");
  Serial.println(vitesse2);
}

void arreterMoteurs() {
  digitalWrite(broche_demarrer_arreter_1, LOW);
  digitalWrite(broche_demarrer_arreter_2, LOW);
  Serial.println("Moteurs arrêtés");
}

