#include <ESP8266WiFi.h>

// config
const char* ssid = "Paul"; // Hotspot smartphone
const char* password = "wifi2paul"; // Mot de passe hotspot
const char* mountpoint = "HBC77"; // Station Centipede la plus proche
const char* ntripHost = "caster.centipede.fr";
const int ntripPort = 2101;

// pt d'accès wifi pour la page web 
const char* apSSID = "gps_diff_esp"; // nom réseau sur lequel il faut se connecter pour avoir accès au serveur web
const char* apPassword = "gps123"; // mdp

// objets réseau 
WiFiServer server(80);
WiFiClient ntrip;

// données gps rtk (lc29hea)
String rtkLat = "En attente...";
String rtkLon = "En attente...";
String rtkAlt = "--";
String rtkSats = "--";
String rtkFix = "0";

// utilitaires nmea
String getField(String data, int index) {
  int found = 0, start = 0;
  for (int i = 0; i <= (int)data.length(); i++) {
    if (i == (int)data.length() || data.charAt(i) == ',') {
      if (found == index) return data.substring(start, i);
      found++;
      start = i + 1;
    }
  }
  return "";
}

void parseGGA(String line) {
  if (line.indexOf("GGA") == -1) return;
  String la = getField(line, 2);
  String lo = getField(line, 4);
  if (la.length() < 4) return;

  rtkLat = la.substring(0, 2) + "° " + la.substring(2) + "' " + getField(line, 3);
  rtkLon = lo.substring(0, 3) + "° " + lo.substring(3) + "' " + getField(line, 5);
  rtkFix = getField(line, 6);
  rtkSats = getField(line, 7);
  rtkAlt = getField(line, 9) + " m";
}

// connexion ntrip réseau centipede
void connectNTRIP() {
  if (ntrip.connect(ntripHost, ntripPort)) {
    ntrip.print("GET /" + String(mountpoint) + " HTTP/1.0\r\n");
    ntrip.print("User-Agent: NTRIP ESP8266\r\n");
    ntrip.print("Authorization: Basic \r\n");
    ntrip.print("\r\n");
  }
}

// page web
void sendPage(WiFiClient& client) {
  String fixLabel, fixColor, fixBg;
  if      (rtkFix == "4") { fixLabel = "RTK FIX";    fixColor = "#00ff88"; fixBg = "#003320"; }
  else if (rtkFix == "5") { fixLabel = "RTK FLOAT";  fixColor = "#ffaa00"; fixBg = "#332200"; }
  else if (rtkFix == "2") { fixLabel = "DGPS";        fixColor = "#00aaff"; fixBg = "#001433"; }
  else                    { fixLabel = "GPS SIMPLE";  fixColor = "#ff4444"; fixBg = "#330000"; }

  String precisionLabel;
  if      (rtkFix == "4") precisionLabel = "Precision : ~1-2 cm";
  else if (rtkFix == "5") precisionLabel = "Precision : ~10-30 cm";
  else if (rtkFix == "2") precisionLabel = "Precision : ~1-3 m";
  else                    precisionLabel = "Precision : ~3-10 m";

  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: text/html; charset=utf-8");
  client.println("Refresh: 2");
  client.println("Connection: close");
  client.println();

  client.println("<!DOCTYPE html><html><head>");
  client.println("<meta name='viewport' content='width=device-width,initial-scale=1'>");
  client.println("<title>GPS RTK</title>");
  client.println("<style>");
  client.println("*{box-sizing:border-box;margin:0;padding:0;}");
  client.println("body{background:#0d0d0d;color:#e0e0e0;font-family:'Courier New',monospace;padding:16px;min-height:100vh;}");
  client.println("h1{text-align:center;font-size:1.3em;color:#00e5ff;letter-spacing:4px;text-transform:uppercase;margin-bottom:4px;padding-top:8px;}");
  client.println(".sub{text-align:center;color:#444;font-size:0.7em;letter-spacing:2px;margin-bottom:16px;}");
  // Badge fix
  client.println(".fix-badge{display:flex;flex-direction:column;align-items:center;justify-content:center;padding:12px;border-radius:10px;margin-bottom:14px;border:1px solid;gap:4px;}");
  client.println(".fix-badge .fix-title{font-size:1.1em;font-weight:bold;letter-spacing:3px;}");
  client.println(".fix-badge .fix-precision{font-size:0.75em;opacity:0.8;}");
  client.println(".fix-badge .fix-sats{font-size:0.7em;color:#888;}");
  // Cards
  client.println(".card{background:#161616;border-radius:10px;padding:14px 16px;margin-bottom:10px;border-left:3px solid #00e5ff;}");
  client.println(".card-label{color:#555;font-size:0.65em;text-transform:uppercase;letter-spacing:3px;margin-bottom:6px;}");
  client.println(".card-val{font-size:1.25em;font-weight:bold;color:#00e5ff;word-break:break-all;}");
  client.println(".card-alt{border-left-color:#7c4dff;}");
  client.println(".card-alt .card-val{color:#b388ff;}");
  // Grille 2 colonnes
  client.println(".grid2{display:grid;grid-template-columns:1fr 1fr;gap:10px;margin-bottom:10px;}");
  client.println(".grid2 .card{margin-bottom:0;}");
  // Footer
  client.println(".footer{text-align:center;color:#333;font-size:0.65em;margin-top:20px;letter-spacing:1px;}");
  client.println("</style></head><body>");

  client.println("<h1>GPS RTK</h1>");
  client.println("<div class='sub'>LC29HEA + CENTIPEDE</div>");

  // Badge statut
  client.println("<div class='fix-badge' style='color:" + fixColor + ";background:" + fixBg + ";border-color:" + fixColor + ";'>");
  client.println("<span class='fix-title'>" + fixLabel + "</span>");
  client.println("<span class='fix-precision'>" + precisionLabel + "</span>");
  client.println("<span class='fix-sats'>Satellites : " + rtkSats + "</span>");
  client.println("</div>");

  // Latitude / Longitude
  client.println("<div class='card'><div class='card-label'>Latitude</div><div class='card-val'>" + rtkLat + "</div></div>");
  client.println("<div class='card'><div class='card-label'>Longitude</div><div class='card-val'>" + rtkLon + "</div></div>");

  // Altitude
  client.println("<div class='card card-alt'><div class='card-label'>Altitude</div><div class='card-val'>" + rtkAlt + "</div></div>");

  client.println("<div class='footer'>Actualisation toutes les 2s</div>");
  client.println("</body></html>");
}

// gestion requête http
void handleClient(WiFiClient& client) {
  String request = "";
  unsigned long t = millis();
  while (client.connected() && millis() - t < 1000) {
    if (client.available()) {
      char c = client.read();
      request += c;
      if (request.endsWith("\r\n\r\n")) break;
    }
  }
  sendPage(client);
  client.stop();
}

// setup 
void setup() {
  Serial.begin(460800);
  Serial.swap(); // LC29HEA sur GPIO13(RX) / GPIO15(TX)

  // Connexion au hotspot pour Centipède
  WiFi.mode(WIFI_AP_STA); // AP + Station en même temps
  WiFi.begin(ssid, password);

  // Point d'accès pour la page web
  WiFi.softAP(apSSID, apPassword);

  // Attendre connexion hotspot (max 15s)
  unsigned long start = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - start < 15000) {
    delay(500);
  }

  server.begin();

  if (WiFi.status() == WL_CONNECTED) {
    connectNTRIP();
  }
}

// bouclage
static String nmeaBuf = "";
static unsigned long dernierCheck = 0;

void loop() {
  // 1. Envoie les corrections RTCM de Centipède vers le LC29HEA
  while (ntrip.available()) {
    Serial.write(ntrip.read());
  }

  // 2. Lit les trames NMEA du LC29HEA
  while (Serial.available()) {
    char c = Serial.read();
    if (c == '\n') {
      parseGGA(nmeaBuf);
      nmeaBuf = "";
    } else if (c >= 32) {
      nmeaBuf += c;
    }
  }

  // 3. Reconnexion Centipède si coupé
  if (millis() - dernierCheck > 10000) {
    dernierCheck = millis();
    if (WiFi.status() != WL_CONNECTED) {
      WiFi.begin(ssid, password);
    } else if (!ntrip.connected()) {
      connectNTRIP();
    }
  }

  // 4. Répondre aux navigateurs
  WiFiClient client = server.available();
  if (client) handleClient(client);
}
