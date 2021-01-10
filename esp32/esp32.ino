//Librerias---------------------------------------------------------------------


    #include <Arduino.h>

    #include <IPAddress.h>
    #include <WiFi.h>
    #include <WiFiClient.h>
    #include <WebServer.h>

    #include <Time.h>
    #include <TimeLib.h>
    #include <TimeAlarms.h>

    #include "src/leds/leds.h"
    #include "src/reles/rele.h"
    #include "src/temp/temp.h"

    #include "src/ArduinoJson/ArduinoJson.h"


//Conf--------------------------------------------------------------------------


    const char* ssid = "Casa 2.4 GHz";
    const char* password = "XsV&`B_bPcBK41/&kV[%v81L_";


//Serial------------------------------------------------------------------------


    void serialStart(){
        Serial.begin(115200);
    }

    void serialCheckReceived(){

        while (Serial.available()) { //COMPROBAR SERIAL

            static String serialLinea;

            char serialCaracter = Serial.read();
            serialLinea += serialCaracter;

            if (serialCaracter=='|'){
                procesarMensaje(serialLinea, "Serial", 1);
                serialLinea = ""; //limpia la linea
            }
        }
    }


admLed adminLed;

sensorTemp temp(A0);


WebServer server(80);


//Procesar mensaje--------------------------------------------------------------

    DynamicJsonDocument procesarMensaje(String msg, String from, IPAddress ip){
        
        //Serial.print("  Desde: ");
        //Serial.print(from);
        //Serial.print("  Por: ");
        //Serial.println(ip);

        //Procesado
            const char separador = ';'; //Variables iniciales
            char *ptr;
            int i = 0;
            char frase[500];
            String seccion[200];

            msg.toCharArray(frase, 200); //Se acomoda el String

            ptr = strtok( frase, "-"); //Primer token
            seccion[i] = ptr;
            i++;
            while((ptr = strtok( NULL, "-" )) != NULL ){ //Posteriores tokens
                seccion[i] = ptr;
                i++;
            } //Salida es String seccion[i], sin el ";"
            Serial.println(msg);
        //Redireccion

            DynamicJsonDocument json(1024);

            json["command"] = msg;
    

            if (seccion[0].toInt() != 0){
                json["body"] = "redirected";
                
            }else if(seccion[1] == "ready"){
               json["body"] = "yes";

            }else if(seccion[1] == "admin"){
                json["body"] = adminLed.input(seccion);

            }else{
                json["body"]  = "Not found";
            }

        return json;
        
    }

    DynamicJsonDocument procesarMensaje(String msg, String from, int id){
        IPAddress q(0,0,0,id);
        return procesarMensaje(msg, from, q);
    }


void setup() {
    serialStart();

    Serial.println("Iniciando");
    Serial.println();
  
    IPAddress local_IP(192, 168, 1, 6);
    IPAddress gateway(192, 168, 1, 1);
    IPAddress subnet(255, 255, 255, 0);
    IPAddress primaryDNS(8, 8, 8, 8); 
    IPAddress secondaryDNS(8, 8, 4, 4);

    if (!WiFi.config(local_IP, gateway, subnet, primaryDNS, secondaryDNS)) {
        Serial.println("STA Failed to configure");
    }

    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }

    Serial.println("");
    Serial.print("Connected to ");
    Serial.println(ssid);
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());

    server.on("/", serverRoot);
    server.on("/api/command", HTTP_POST, serverCommand);
    server.onNotFound(serverMissing);
    server.begin();
    Serial.println("HTTP server started");
    
    procesarMensaje("0-admin-led-new-1-4|", "Serial", 0);
    procesarMensaje("0-admin-led-new-1-16|", "Serial", 0);
    procesarMensaje("0-admin-led-new-3-17-5-18|", "Serial", 0);
    procesarMensaje("0-admin-led-setTarget-0-1-1023|", "Serial", 0);    
    procesarMensaje("0-admin-led-setTarget-1-1-1023|", "Serial", 0);
    procesarMensaje("0-admin-led-setTarget-2-3-1023-1023-1023|", "Serial", 0);

    
	Serial.print("INICIADO (");
    Serial.print(millis()/1000);
    Serial.println(")");

    pinMode(2, OUTPUT);
    digitalWrite(2, LOW);

}

void loop() {
    serialCheckReceived();
    server.handleClient();

    adminLed.checkLeds();
    Alarm.delay(0);
}

void serverRoot() {
    char temp[400];
    int sec = millis() / 1000;
    int min = sec / 60;
    int hr = min / 60;

    const char* html = R"(
        <!DOCTYPE html>
        <html>
            <body>
                Uptime: %02d:%02d:%02d
            </body>
        </html>)";

    snprintf(temp, 400, html, hr, min % 60, sec % 60);
    server.send(200, "text/html", temp);
}

void serverMissing() {
    String message = "File Not Found\n\n";
    message += "URI: ";
    message += server.uri();
    message += "\nMethod: ";
    message += (server.method() == HTTP_GET) ? "GET" : "POST";
    message += "\nArguments: ";
    message += server.args();
    message += "\n";

    for (uint8_t i = 0; i < server.args(); i++) {
        message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
    }

    server.send(404, "text/plain", message);
}

void serverCommand() {
    Serial.println("recibido");
    String answer;
    String command = server.arg("command");

    Serial.print("command:");
    Serial.println(command);

    serializeJson(procesarMensaje(command, "server", server.client().remoteIP()), answer);;

    Serial.println("respondiendo...");
    server.send(200, "application/json", answer);
}
