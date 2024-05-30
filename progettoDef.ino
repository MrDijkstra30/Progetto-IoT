#include <Arduino.h>
#include <ESP32Servo.h>
#include <WiFi.h>
#include <HTTPClient.h>

const char* ssid = "Redmi Note 9 Pro"; // SSID della tua rete Wi-Fi
const char* password = "forzabologna"; // Password della tua rete Wi-Fi

//Your Domain name with URL path or IP address with path
String serverName = "http://192.168.171.177/Progetto%20IoT/Parcheggio.php";

unsigned long lastTime = 0;
unsigned long timerDelay = 500;

// Definizione dei pin per i sensori ad ultrasuoni
const int trigPinIngresso = 26;
const int echoPinIngresso = 25;
const int trigPinUscita = 33;  // Pin per il trigger del sensore di uscita
const int echoPinUscita = 32;  // Pin per l'echo del sensore di uscita

// Definizione dei pin per i motori DC
const int motorPinIngresso = 27;
const int motorPinUscita = 14;  // Pin per il servo motore dell'uscita

// Definizione del limite di distanza per sollevare la sbarra (in centimetri)
const int distanceThreshold = 5;

// Dichiarazione degli oggetti Servo per controllare i motori
Servo myMotorIngresso;
Servo myMotorUscita;

const int ledPinIngresso = 0;
const int ledPinUscita = 4;

int postiDisponibili = 10; // Numero iniziale di posti disponibili

void setup() {
  // Inizializzazione della comunicazione seriale
  Serial.begin(115200);

  pinMode(ledPinIngresso, OUTPUT);
  pinMode(ledPinUscita, OUTPUT);

  // Inizializzazione dei pin dei sensori ad ultrasuoni
  pinMode(trigPinIngresso, OUTPUT);
  pinMode(echoPinIngresso, INPUT);
  pinMode(trigPinUscita, OUTPUT);
  pinMode(echoPinUscita, INPUT);

  // Inizializzazione dei pin dei motori DC
  pinMode(motorPinIngresso, OUTPUT);
  pinMode(motorPinUscita, OUTPUT);

  // Collegamento degli oggetti Servo ai pin dei motori
  myMotorIngresso.attach(motorPinIngresso);
  myMotorUscita.attach(motorPinUscita);

  WiFi.begin(ssid, password);
  Serial.println("Connecting");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to WiFi network with IP Address: ");
  Serial.println(WiFi.localIP());
}

void loop() {
  //manda un HTTP POST ogni timerDelay
  if ((millis() - lastTime) > timerDelay) {
    //controlla stato connessione WiFi 
    if(WiFi.status()== WL_CONNECTED){
      HTTPClient http;
      
      // Gestione sbarra d'ingresso
      gestisciIngresso(http);

      // Gestione sbarra d'uscita
      gestisciUscita(http);

      // Libera risorse
      http.end();
      // Attendi un breve intervallo prima di eseguire una nuova misurazione
      delay(100);
    }
    else {
      Serial.println("WiFi Disconnected");
    }
    lastTime = millis();
  }
}

void gestisciIngresso(HTTPClient& http) {
  // Trasmette un impulso breve al pin di trigger del sensore ad ultrasuoni d'ingresso
  digitalWrite(trigPinIngresso, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPinIngresso, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPinIngresso, LOW);

  // Misura la durata dell'impulso di ritorno sull'echo pin e calcola la distanza
  long duration = pulseIn(echoPinIngresso, HIGH);
  int distance = duration * 0.034 / 2;

  // Stampa la distanza misurata sulla porta seriale
  // Serial.print("Distanza ingresso: ");
  // Serial.print(distance);
  // Serial.println(" cm");

  if ((distance < distanceThreshold) && (postiDisponibili > 0)) {
    Serial.println("Inserire targa in ingresso:");
    while (Serial.available() == 0) {
      // Aspetta che l'utente inserisca la targa
    }
    String targa = Serial.readStringUntil('\n');
    targa.trim(); // Rimuove eventuali spazi bianchi iniziali e finali

    if (targa != "") {
      String serverPath = serverName + "?targa=" + targa;
      http.begin(serverPath.c_str());

      int httpResponseCode = http.GET();
      
      if (httpResponseCode > 0) {
       // Serial.print("HTTP Response code: ");
       // Serial.println(httpResponseCode);
        String payload = http.getString();
        // Serial.println(payload);
      } else {
       // Serial.print("Error code: ");
       // Serial.println(httpResponseCode);
      }

      // Solleva la sbarra di ingresso di 90 gradi
      raiseBarrierIngresso();
      delay(1500); // Attendi un secondo per evitare azioni multiple
    }
  } else {
    // Se la distanza è superiore al threshold, abbassa la sbarra
    lowerBarrierIngresso();
  }
}


void gestisciUscita(HTTPClient& http) {
  // Trasmette un impulso breve al pin di trigger del sensore ad ultrasuoni d'uscita
  digitalWrite(trigPinUscita, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPinUscita, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPinUscita, LOW);

  // Misura la durata dell'impulso di ritorno sull'echo pin e calcola la distanza
  long duration = pulseIn(echoPinUscita, HIGH);
  int distance = duration * 0.034 / 2;

  // Stampa la distanza misurata sulla porta seriale
  // Serial.print("Distanza uscita: ");
  // Serial.print(distance);
  // Serial.println(" cm");

  if (distance < distanceThreshold) {
    Serial.println("Inserire targa in uscita:");
    while (Serial.available() == 0) {
      // Aspetta che l'utente inserisca la targa
    }
    String targa = Serial.readStringUntil('\n');
    targa.trim(); // Rimuove eventuali spazi bianchi iniziali e finali

    // Solleva la sbarra di uscita di 90 gradi
    raiseBarrierUscita();
    delay(2000); // Attendi un secondo per evitare azioni multiple

    if (targa != "") {
      String serverPath = serverName + "?update=" + targa;
      http.begin(serverPath.c_str());

      int httpResponseCode = http.GET();
      
      if (httpResponseCode > 0) {
        // Serial.print("HTTP Response code: ");
        // Serial.println(httpResponseCode);
        String payload = http.getString();
        // Serial.println(payload);
      } else {
        // Serial.print("Error code: ");
        // Serial.println(httpResponseCode);
      }
    }

    // Abbassa la sbarra dopo un breve intervallo
    lowerBarrierUscita();
  }
}


void raiseBarrierIngresso() {
  // Solleva la sbarra di ingresso di 90 gradi
  myMotorIngresso.write(90);
  digitalWrite(ledPinIngresso, HIGH);
  postiDisponibili--;
  Serial.print("Posti disponibili: ");
  Serial.println(postiDisponibili);
  Serial.println("SBARRA ALZATA INGRESSO ");
}

void lowerBarrierIngresso() {
  // Abbassa la sbarra di ingresso a 0 gradi
  myMotorIngresso.write(0);
  digitalWrite(ledPinIngresso, LOW);
  // Serial.print("SBARRA abbassata: ");
}

void raiseBarrierUscita() {
  // Solleva la sbarra di uscita di 90 gradi
  myMotorUscita.write(90);
  digitalWrite(ledPinUscita, HIGH);
  postiDisponibili++;
  Serial.print("Posti disponibili: ");
  Serial.println(postiDisponibili);
  Serial.println("SBARRA ALZATA USCITA ");
}

void lowerBarrierUscita() {
  // Abbassa la sbarra di uscita a 0 gradi
  myMotorUscita.write(0);
  digitalWrite(ledPinUscita, LOW);
    //Serial.print("SBARRA abbassata: ");
}
