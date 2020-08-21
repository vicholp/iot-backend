//Librerias

    #include <Arduino.h>
    #include <IPAddress.h>

    #include <UIPEthernet.h> //Ethernet
    #include <UIPUDP.h> //EthernetUDP
    #include <UIPServer.h>
    #include <UIPClient.h> //EthernetClient

    #include <Time.h>
    #include <TimeLib.h>
    #include <TimeAlarms.h>

    #include "src/leds/leds.h"
    #include "src/reles/rele.h"


//Serial--------------------------------------------------------

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

//internet

    static IPAddress IPself(192, 168, 1, 50);
    static IPAddress IPpc(192,168,1,8);
    static IPAddress IPdns(8, 8, 4, 4);

    class netUDP : public EthernetUDP{

        private:
            int portSelf = 45098; //mi puerto
            int portLocal = 45093;

        public:
            netUDP() : EthernetUDP(){};
            int udpStart(); //Inicia envio y recepcion.
            int udpRestart();
            int udpSend(String mensaje, IPAddress ip, int port); //Envia mensaje UDP a ip.
            int udpSend(String mensaje, int id, int port = -1);
            int udpCheckReceived(); //Comprueba nuevos mensajes.
    };

    class netServer : public EthernetServer{

        private:

            //int portLocal = 45097; //No funciona si lo uso al iniciar ¿?

        public:

            netServer() : EthernetServer(45097){}

            int serverStart(); //Inicia escucha
            void serverCheckReceived(); //Comprueba nuevos mensajes.
    };

    class Internet : public netUDP, public netServer{
        public:

            Internet() : netUDP(), netServer(){}; // Inicia todo

            void start();
            bool check1();
            int getExternalIP();
    };

    class ntp{ //TODO improve code
        private:
            int ntpZonaHoraria = -3;

            IPAddress *server1;
            IPAddress *server2;

            byte packet[48];

            bool wait();
            bool send(int hostid); //Envia el paquete

        public:

            ntp();

            void printTime();
            int refresh(); // 0 = Imposible | 1 = Correcto | 2 = Correcto al segundo intento | 3 = no se pudo iniciar
    };


    Internet net;

    //netUDP---


        int netUDP::udpStart(){ //Inicia envio y recepcion.

            return begin(portSelf); //Return 1 or 0
        }

        int netUDP::udpRestart(){
            stop();
            return begin(portSelf);
        }

        int netUDP::udpSend(String mensaje, IPAddress ip, int port){ //Envia mensaje UDP a ip.

            char msj[mensaje.length()+1]; //Debe enviarse un char.

            mensaje.toCharArray(msj, mensaje.length()+1); //Paso el String a Char

            beginPacket(ip, port);
            write(msj);

            int r = endPacket();

            udpRestart();

            return r;
        }

        int netUDP::udpSend(String mensaje, int id, int port = -1){
            if(port==-1) port = portLocal;
            IPAddress ip(192, 168, 1 , id);
            char msj[mensaje.length()+1];

            mensaje.toCharArray(msj, mensaje.length()+1);

            beginPacket(ip, port);
            write(msj);

            int r = endPacket();

            udpRestart();

            return r;
        }

        int netUDP::udpCheckReceived(){ //Comprueba nuevos mensajes.

            int size = parsePacket(); //Tamaño del supuesto mensaje, 0 si no hay.

            if (size > 0){


                char buffer[size]; //Crea buffer para mensaje entrante
                read(buffer, size);

                String mensaje = (String)buffer;
                mensaje = mensaje.substring(0,size); //no se de donde salio el jugo aca.

                IPAddress ip(remoteIP());

                procesarMensaje(mensaje, "UDP", ip);

                udpRestart();



            }
        }

    //netServer
        int netServer::serverStart(){ //Inicia escucha

            begin();

            return 1;
        }

        void netServer::serverCheckReceived(){ //Comprueba nuevos mensajes.

            EthernetClient CLIENT = available(); //Crea cliente

            if(CLIENT){ //Si cliente existe

                String query; //Buffer para mensaje entrante

                char c = ""; //Buffer para caracter entrante

                while ((CLIENT.available()) && (c != '\n')) { //Lee caracter a caracter mientras aun queden datos y no sea un salto de linea

                    c = CLIENT.read();
                    query.concat(c);


                }

                //Algunos reemplazos.

                    query.replace("GET /?", "");
                    query.replace("GET /", "");

                    query.replace(" HTTP/1.1", "");
                    query.replace("%3B", ";");
                    query.remove((query.lastIndexOf('\n')));



                procesarMensaje(query, "HTTP", 1);

                //Respuesta a cliente

                    CLIENT.println("HTTP/1.1 200 OK");
                    CLIENT.println("Content-Type: text");
                    CLIENT.println();
                    //CLIENT.println(answer);
                    //CLIENT.println(query);
                    Alarm.delay(1);

                CLIENT.stop();

                //Serial.println(answer);
            }
        }

    //netInternet

        void Internet::start(){ //conecta a internet

            const byte selfMac[] = { 0x82, 0x68, 0x69, 0x2D, 0x30, 0x31 };

            Ethernet.begin(selfMac, IPself, IPdns);
        }

        bool Internet::check1(){

            EthernetClient CLIENT;

            int r = CLIENT.connect("www.google.com", 80);

            CLIENT.stop();

            return (r == 1 ? true : false);
        }

        int Internet::getExternalIP() {

            EthernetClient CLIENT; //Crea cliente temporal

            //Serial.println("Obteniendo IP Publica..");

            if (CLIENT.connect("api.ipify.org", 80)) {  //Inicia conexion. Lineas siguienten inician comunicacion.

                //Serial.println("Procesando...");

                CLIENT.println("GET / HTTP/1.0");
                CLIENT.println("Host: api.ipify.org");
                CLIENT.println(); //termino

                uint32_t beginWait = millis(); //para guardar tiempo actual.

                while (millis() - beginWait < 2000) {

                    if(CLIENT.available() > 0) { //Esperar conexion y respuesta, hasta 1 segundo

                        for(int n = 0; n<9; ){ //lee basura, 8 lineas.

                            if(CLIENT.read() == '\n'){ //Contar saltos de linea

                                n++;

                            }

                        }

                        //Serial.print("IP Publica: ");

                        while(CLIENT.available()){ //Lee IP

                            Serial.print((char)CLIENT.read());

                        }

                        Serial.println();

                        CLIENT.stop(); //Termino conexion.

                        return 1;


                    }

                }

                //Serial.println("Tiempo de espera excedido, no se ha obtenido IP");

                CLIENT.stop(); //Termino conexion.

                return 0;

            }else{ //Si nunca conecto

                //Serial.println("No ha sido posible obtener IP publica.");

                CLIENT.stop(); //Termino conexion.

                return 0;

            }
        }

    //ntp
        ntp::ntp(){

            server1 = new IPAddress(37, 235, 52, 50);
            server2 = new IPAddress(200, 1, 19, 16);

            memset(packet, 0, 48);

            packet[0] = 0b11100011;
            packet[1] = 0;
            packet[2] = 6;
            packet[3] = 0xEC;
            packet[12]  = 49;
            packet[13]  = 0x4E;
            packet[14]  = 49;
            packet[15]  = 52;
        }

        bool ntp::wait(){

                    uint32_t beginWait = millis(); //para guardar tiempo actual.

                    while (millis() - beginWait < 2000) { //Espera maxima 2 segundos

                        int size = net.EthernetUDP::parsePacket();



                        if (size >= 48) {

                            byte packetBuffer[48]; //Buffer de guardado.

                            //Serial.println("Procesando...");

                            net.EthernetUDP::read(packetBuffer, size);  //lee

                            //MAGIA

                                unsigned long secsSince1900;
                                secsSince1900 =  (unsigned long)packetBuffer[40] << 24;
                                secsSince1900 |= (unsigned long)packetBuffer[41] << 16;
                                secsSince1900 |= (unsigned long)packetBuffer[42] << 8;
                                secsSince1900 |= (unsigned long)packetBuffer[43];

                            setTime(secsSince1900 - 2208988800UL + ntpZonaHoraria * 3600); //listo

                            return true;


                        }
                    }


                    return false;
        }

        bool ntp::send(int hostid){ //Envia el paquete

            if (hostid == 1) net.EthernetUDP::beginPacket(*server1, 123);
            if (hostid == 2) net.EthernetUDP::beginPacket(*server2, 123);

            net.EthernetUDP::write(packet, 48);
            int r = net.EthernetUDP::endPacket();

            //net.udpRestart();

            return (r == 1 ? true : false);
        }

        void ntp::printTime(){
            Serial.print(hour());
            Serial.print(":");
            Serial.print(minute());
            Serial.print(" ");
            Serial.print(day());
            Serial.print("/");
            Serial.print(month());
            Serial.print("/");
            Serial.print(year());
            Serial.println();
        }

        int ntp::refresh(){ // 0 = Imposible | 1 = Correcto | 2 = Correcto al segundo intento | 3 = no se pudo iniciar


            //Serial.println("Sincronizando hora...");

            if (send(1) == true){ //Primer intento

                //Serial.println("Esperando respuesta...");

                if(wait()  == true){

                    net.udpRestart();

                    //Serial.println("Hora sincronizada.");
                    printTime();

                    return 1;

                }else{

                    net.udpRestart();

                    //Serial.println("Primer intento fallido. Reintentando...");

                }

            }else{ //Error de envio

                net.udpRestart();

                //Serial.println("No se ha podido iniciar la comunicacion. Reintentando...");

            }

            if (send(2) == true){ //Segundo intento

                //Serial.println("Esperando respuesta...");

                if(wait() == true){

                    net.udpRestart();

                    //Serial.println("Hora sincronizada");
                    printTime();

                    return 2;

                }else{

                    net.udpRestart();

                    //Serial.println("Segundo intento fallido. No se ha podido sincronizar la hora.");

                    printTime();

                    return 0;

                }


            }else{ //Error de envio

                net.udpRestart();

                //Serial.println("No se ha podido iniciar la comunicacion. No se ha podido sincronizar la hora.");

                printTime();

                return 3;

            }
        }

admLed adminLed;

ntp hora;

//Procesar mensaje

    int procesarMensaje(String msg, String from, IPAddress ip){
        Serial.print("-> ");
        Serial.println(msg);
        //Serial.print("  Desde: ");
        //Serial.print(from);
        //Serial.print("  Por: ");
        //Serial.println(ip);

        //Procesado
            char separador = ";"; //Variables iniciales
            char *ptr;
            int i = 0;
            char frase[500];
            String seccion[200];

            msg.toCharArray(frase, 200); //Se acomoda el String

            ptr = strtok( frase, ";" ); //Primer token
            seccion[i] = ptr;
            i++;
            while((ptr = strtok( NULL, ";" )) != NULL ){ //Posteriores tokens
                seccion[i] = ptr;
                i++;
            } //Salida es String seccion[i], sin el ";"

        //Redireccion

            if (seccion[0].toInt() != 0){
                Serial.println("Redireccion");
                return 2;
            }

        //Acciones

            if(seccion[1] == "ready"){
                Serial.println("yes");
                return 1;
            }

            if(seccion[1] == "admin"){
                adminLed.input(seccion);

           }
        return "1";
    }

    int procesarMensaje(String msg, String from, int id){
        IPAddress q(0,0,0,id);
        return procesarMensaje(msg, from, q);
    }


void setup() {
    serialStart();

    Serial.println("Iniciando");
    Serial.println();

    bool internet = false;
    bool udp = false;
    bool server = false;
    bool ntp = false;
    bool ipExt = false;

    if (internet){
        Serial.print("Internet: ");
        net.start(); //Conecta

        if(net.check1()){ //comprueba

            Serial.println("Correcto.");
            Serial.println();

            if (udp){

                Serial.print("UDP: ");

                if(net.udpStart() == 1){

                    Serial.println("Conectado.");

                }else{

                    Serial.println("Error.");

                }
                Serial.println();
            }

            if(server){
                Serial.print("Servidor: ");

                    if(net.serverStart() == 1){

                        Serial.println("Disponible.");

                    }else{

                        Serial.println("Error.");

                    }
                Serial.println();
            }
            if(ntp){
                Serial.print("Fecha y hora: ");

                    switch (hora.refresh()){
                        case 0:

                            Serial.println(" Error.");
                            break;

                        case 1:

                            break;

                        case 2:

                            Serial.println(" SEGUNDO INTENTO.");
                            break;

                        case 3:

                            Serial.println(" No hubo comunicacion");
                            break;

                    }
                Serial.println();
            }
            if(ipExt){
                Serial.print("IP Externa: ");

                    if(net.getExternalIP() == 0){

                        Serial.println("Error.");

                    }
                Serial.println();
            }

        }else{

            Serial.println("Error.");
        }
    }


    Serial.print("INICIADO ( ");
    Serial.print(millis()/1000);
    Serial.println(")");
    procesarMensaje("0;admin;led;new;1;2;|", "Serial", 0);
    procesarMensaje("0;admin;led;setTarget;0;1;199;|", "Serial", 0);
    procesarMensaje("0;admin;led;new;1;3;|", "Serial", 0);
    procesarMensaje("0;admin;led;setTarget;1;1;50;|", "Serial", 0);
    procesarMensaje("0;admin;led;debug;|", "Serial", 0);

}

void loop() {
    net.serverCheckReceived();
    net.udpCheckReceived();
    serialCheckReceived();

    adminLed.checkLeds();

    Alarm.delay(0);
}
