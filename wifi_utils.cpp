#include "wifi_utils.h"

#include "config.h"

#include <WiFiMulti.h>

// Point d'acces personnel (SSID + mot de passe) : dans wifi_secrets.h, ignore par Git.
// Sans ce fichier, seuls les points d'acces du cours sont essayes.
#if __has_include("wifi_secrets.h")
#include "wifi_secrets.h"
#endif

#define USE_SERIAL Serial

/*--------------------------------------------------------------------------*/
void wificonnect_multi(String hostname){
  int nbtry = 0; // Nb of try to connect

  WiFiMulti wm; // Creates an instance of the WiFiMulti class
  // Attention ! PAS arrive a sortir l'instance de la fonction => heap error ! Why ????
  // Points d'acces du cours : a ne pas supprimer (utilises pour l'evaluation)
  wm.addAP("HUAWEI-6EC2", "FGY9MLBL");
  wm.addAP("HUAWEI-553A", "QTM06RTT");
  wm.addAP("GMAP", "vijx47050");
  wm.addAP("IOT","iotmiage");
  wm.addAP("Bbox-EA6814CF","Hvtx6bGfD4Sn7PCs2r");
#ifdef WIFI_MY_SSID
  wm.addAP(WIFI_MY_SSID, WIFI_MY_PASSWORD); // point d'acces personnel (wifi_secrets.h)
#endif

  WiFi.mode(WIFI_STA); // Set WiFi to station mode
  WiFi.disconnect(true); // Disconnect from an AP if it was previously connected

  // Define hostname => in C str ! not C++
  WiFi.setHostname(hostname.c_str());

  while(wm.run() != WL_CONNECTED && (nbtry < WiFiMaxTry)) {
    USE_SERIAL.printf("\nAttempting %d to connect AP", nbtry);
    delay(SaveDisconnectTime);
    USE_SERIAL.print(".");
    nbtry++;
  }

  if(wm.run() == WL_CONNECTED) {
    // Correction : %s attend un texte C => WiFi.SSID().c_str()
    USE_SERIAL.printf("\nWifi Multi connected on %s !", WiFi.SSID().c_str());
    wifi_printstatus(0);
  }
  else if (WIFI_RESTART_IF_FAILED) {
    ESP.restart();
  }
  // Sinon la regulation continue sans WiFi (le JSON indique alors "NOP" dans net)
}
/*--------------------------------------------------------------------------*/
static String translateEncryptionType(wifi_auth_mode_t encryptionType) {
  //  encryptiontype to string
  // cf https://www.arduino.cc/en/Reference/WiFiEncryptionType
  switch (encryptionType) {
  case (WIFI_AUTH_OPEN):
    return "Open";
  case (WIFI_AUTH_WEP):
    return "WEP";
  case (WIFI_AUTH_WPA_PSK):
    return "WPA_PSK";
  case (WIFI_AUTH_WPA2_PSK):
    return "WPA2_PSK";
  case (WIFI_AUTH_WPA_WPA2_PSK):
    return "WPA_WPA2_PSK";
  case (WIFI_AUTH_WPA2_ENTERPRISE):
    return "WPA2_ENTERPRISE";
  default: // Correction : autres types (WPA3...) => sinon erreur de compilation
    return "Autre";
  }
}
/*--------------------------------------------------------------------------*/
void wifi_printstatus(int C){
  /* print the status of the connected wifi  in two ways ! */

  if (C){
    // Use Pure C =>  array of chars
    USE_SERIAL.printf("WiFi Status : \n");
    USE_SERIAL.printf("\tIP address : %s\n", WiFi.localIP().toString().c_str());
    USE_SERIAL.printf("\tMAC address : %s\n", WiFi.macAddress().c_str());
    USE_SERIAL.printf("\tSSID : %s\n", WiFi.SSID().c_str()); // Correction : .c_str()
    USE_SERIAL.printf("\tReceived Signal Strength Indication : %d dBm\n", WiFi.RSSI());
    USE_SERIAL.printf("\tReceived Signal Strength Indication : %d %%\n",
                      constrain(2 * (WiFi.RSSI() + 100), 0, 100));
    USE_SERIAL.printf("\tBSSID : %s\n", WiFi.BSSIDstr().c_str());
    USE_SERIAL.printf("\tEncryption type : %s\n", translateEncryptionType(WiFi.encryptionType(0)).c_str()); // Correction : .c_str()
  }
  else {
    // Use of C++ =>  String !
    String s = "WiFi Status : \n";
    s += "\tIP address : " + WiFi.localIP().toString() + "\n";
    s += "\tMAC address : " + String(WiFi.macAddress()) + "\n";
    s += "\tSSID : " + String(WiFi.SSID()) + "\n";
    s += "\tReceived Sig Strength Indication : " + String(WiFi.RSSI()) + " dBm\n";
    s += "\tReceived Sig Strength Indication : " +
      String(constrain(2 * (WiFi.RSSI() + 100), 0, 100)) + " %\n";
    s += "\tBSSID : " + String(WiFi.BSSIDstr()) + "\n";
    s += "\tEncryption type : " + translateEncryptionType(WiFi.encryptionType(0))+ "\n";
    USE_SERIAL.print(s);
  }
}
