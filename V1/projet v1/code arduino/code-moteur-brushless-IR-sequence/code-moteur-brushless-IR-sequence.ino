#include <IRremote.h>
#include <Servo.h>


const int brocheDemarrerArreter1 = 5; // ON/OFF le moteur 1
const int brocheVitesse1 = 6;         // vitesse du moteur 1 (PWM)
const int brocheDemarrerArreter2 = 3; // ON/OFF le moteur 2
const int brocheVitesse2 = 4;         // vitesse du moteur 2 (PWM)
const int brocheReceptionIR = 12;     // capteur IR
const int brocheServo = 10;           // servomoteur

Servo monServo;  // Création de l'objet servomoteur
IRrecv irrecv(brocheReceptionIR);
decode_results resultats;

int vitesse1 = 0;
int vitesse2 = 0;
const int nombreDeLancements = 3; // Nombre fixe de lancements pour chaque configuration
volatile bool arretUrgence = false; // Flag pour l'arrêt d'urgence

struct IntervallePWM {
  int minPwm;
  int maxPwm;
};

struct ConfigurationMoteur {
  const char* nom;
  IntervallePWM moteur1;
  IntervallePWM moteur2;
};

// Tableau des configurations possibles pour les moteurs
ConfigurationMoteur configurations[] = {
  {"Latéral Droite", {160, 184}, {126, 160}},
  {"Latéral Gauche", {123, 160}, {160, 178}},
  {"Lifté Bas", {160, 184}, {100, 160}}
};

void setup() {
  pinMode(brocheDemarrerArreter1, OUTPUT);
  pinMode(brocheVitesse1, OUTPUT);
  pinMode(brocheDemarrerArreter2, OUTPUT);
  pinMode(brocheVitesse2, OUTPUT);

  Serial.begin(115200);
  while (!Serial); 
  Serial.println(F("Démarrage du système - Configuration initiale complète."));

  irrecv.enableIRIn(); 
  Serial.print("Prêt à recevoir des signaux IR à la broche ");
  Serial.println(brocheReceptionIR);

  monServo.attach(brocheServo); 
}

void loop() {
  if (irrecv.decode(&resultats)) {
    handleCommandeIR(resultats.value);
    irrecv.resume(); // Prépare pour la prochaine valeur reçue
  }
}

void handleCommandeIR(unsigned long commande) {
  Serial.print("Commande IR reçue: ");
  Serial.println(commande, HEX);
  if (commande == 0x45) { // Commande d'arrêt immédiat
    arretUrgence = true;
    arreterMoteurs();
    return;
  }

  switch (commande) {
    case 0x16: lancerConfiguration(0); break;
    case 0xC: lancerConfiguration(1); break;
    case 0x18: lancerConfiguration(2); break;
    default: Serial.println("Commande non reconnue."); break;
  }
}

void lancerConfiguration(int indexConfig) {
  for (int i = 0; i < nombreDeLancements && !arretUrgence; i++) {
    appliquerConfiguration(indexConfig);
    activerServomoteur();
  }
  arreterMoteurs();
  arretUrgence = false;
}

void appliquerConfiguration(int indexConfig) {
  if (arretUrgence) return;
  ConfigurationMoteur configSelectionnee = configurations[indexConfig];
  vitesse1 = random(configSelectionnee.moteur1.minPwm, configSelectionnee.moteur1.maxPwm + 1);
  vitesse2 = random(configSelectionnee.moteur2.minPwm, configSelectionnee.moteur2.maxPwm + 1);

  Serial.print("Configuration appliquée: ");
  Serial.println(configSelectionnee.nom);
  Serial.print("Vitesse moteur 1: ");
  Serial.println(vitesse1);
  Serial.print("Vitesse moteur 2: ");
  Serial.println(vitesse2);

  mettreAJourMoteurs();
}

void arreterMoteurs() {
  vitesse1 = 0;
  vitesse2 = 0;
  Serial.println("Arrêt immédiat des deux moteurs.");
}

void mettreAJourMoteurs() {
  analogWrite(brocheVitesse1, vitesse1);
  digitalWrite(brocheDemarrerArreter1, vitesse1 > 0 ? HIGH : LOW);
  analogWrite(brocheVitesse2, vitesse2);
  digitalWrite(brocheDemarrerArreter2, vitesse2 > 0 ? HIGH : LOW);
}

void activerServomoteur() {
  monServo.write(180); // Position pour laisser passer une balle
  delay(225);
  monServo.write(90); // Retour à la position initiale
  delay(2000);
}
