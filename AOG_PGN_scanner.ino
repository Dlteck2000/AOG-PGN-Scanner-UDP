#include "EtherCard_AOG.h"
#include <IPAddress.h>

// ========== CONFIG CARTE RESEAU ==========
#define CS_Pin 10 //Broche CS Arduino Nano

static uint8_t myip[] = { 192, 168, 1, 123 };// Adresse IP de la carte machine
static uint8_t gwip[] = { 192, 168, 1, 1 };// Adresse de la passerelle
static uint8_t myDNS[] = { 8, 8, 8, 8 };// DNS
static uint8_t mask[] = { 255, 255, 255, 0 };// Masque de sous réseau
uint16_t portMy = 5123;// Port de la carte machine

//Adresse Ip et port d'AOG
static uint8_t ipDestination[] = {192, 168, 1, 255};
uint16_t portDestination = 9999;

// ethernet mac address - must be unique on your network
static uint8_t mymac[] = { 0x00, 0x00, 0x56, 0x00, 0x00, 0x7B };

uint8_t Ethernet::buffer[200]; // udp send and receive buffer

//========== CONFIG de L'INO ==========

// Gestion du temps en millisecondes
const uint8_t LOOP_TIME = 200; // 5hz
uint32_t lastTime = LOOP_TIME;
uint32_t currentTime = LOOP_TIME;
uint32_t fifthTime = 0;
uint16_t count = 0;

// Gestion du temps de communication entre AOG et la carte machine
uint8_t watchdogTimer = 20; // Max 20*200ms = 4s (LOOP TIME) si plus de 20 déclaration de perte de communication avec le module
uint8_t serialResetTimer = 0; // Vidange de la mémoire tampon


void setup()
{
  Serial.begin(38400);
  if (ether.begin(sizeof Ethernet::buffer, mymac, CS_Pin) == 0)
    Serial.println(F("Failed to access Ethernet controller"));
  //set up connection
  ether.staticSetup(myip, gwip, myDNS, mask);
  ether.printIp("IP:  ", ether.myip);
  ether.printIp("GW:  ", ether.gwip);
  ether.printIp("DNS: ", ether.dnsip);

  //register to port 8888
  ether.udpServerListenOnPort(&udpSteerRecv, 8888);

  Serial.println("Setup complete, waiting for AgOpenGPS");
}

void loop() {
  currentTime = millis(); // Maintenant

  if (currentTime - lastTime >= LOOP_TIME) // Si maintenant - avant plus grand ou égale à 200
  {
    lastTime = currentTime; // Avant = maintenant

    //If connection lost to AgOpenGPS, the watchdog will count up
    if (watchdogTimer++ > 250) watchdogTimer = 20;

    //clean out serial buffer to prevent buffer overflow
    if (serialResetTimer++ > 20)
    {
      while (Serial.available() > 0) Serial.read();
      serialResetTimer = 0;
    }

  }
  //this must be called for ethercard functions to work. Calls udpSteerRecv() defined way below.
  ether.packetLoop(ether.packetReceive());
}

{
  for (int16_t i = 0; i < len; i++)
  {
    Serial.print("[");
    Serial.print(i);
    Serial.print("]");
    Serial.print(udpData[i]);
    Serial.print("(");
    Serial.print(udpData[i], HEX);
    Serial.print(")");
    Serial.print("\t");
  }
  Serial.println(len);
}
