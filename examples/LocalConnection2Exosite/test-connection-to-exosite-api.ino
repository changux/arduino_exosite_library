#include <SPI.h>
#include <Ethernet.h>

byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
char server[] = "m2.exosite.com";
EthernetClient client;

void setup() {
  Serial.begin(9600);
  while (!Serial);

  if (Ethernet.begin(mac) == 0) {
    Serial.println("eth problem");
    while(1); //stop
  }
  delay(1000);

  do{
    Serial.println("connecting");
  } while (!client.connect(server, 80));
  
  Serial.println("connected");

  client.println("GET /timestamp HTTP/1.1");
  client.println("Host: m2.exosite.com");
  client.println("Connection: close");
  client.println();
}

void loop()
{
  if (client.available()) {
    Serial.print((char)client.read());
  } else if (!client.connected()) {
    Serial.println("\n\ndone");
    client.stop();

    while (1); //stop
  }
}
