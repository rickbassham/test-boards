#ifndef VERSION
#define VERSION "0.0.1"
#endif

#define MDNS_FEATURE
#define USE_THIS_SS_PIN 17
#define RESPONSE_BUFFER_SIZE 512

#define LIGHT_TRIGGER_PIN 23

#define BOARD_ID 0xDA

#include <Arduino.h>
#include <SPI.h>
#include <Ethernet_Generic.h>
#include <Wire.h>
#include <SHT85.h>

#ifdef MDNS_FEATURE
#include <MDNS_Generic.h>
#endif
#include <LittleFS.h>
#include <ArduinoJson.h>
#include <MD5Builder.h>

#include <ident.h>
#include <mimetype.h>
#include <unzip_helpers.h>

#include <EthernetWebServer.h>
#include <Updater.h>

#ifdef MDNS_FEATURE
EthernetUDP udp;
MDNS mdns(udp);
#endif

EthernetUDP alpacaDiscovery;
EthernetUDP ddaDiscovery;

EthernetWebServer server(80);

SHT30 sht30;

byte *mac;
String macAddress;

bool isLightOn = false;
byte lightLevel = 0;
bool isConnected = false;
float temperature = 0;
float humidity = 0;
float dewPoint = 0;

uint32_t transactionId = 1;
String hostname;

void setLight()
{
    if (isLightOn)
    {
        analogWrite(LED_BUILTIN, lightLevel);
        analogWrite(LIGHT_TRIGGER_PIN, lightLevel);
    }
    else
    {
        analogWrite(LED_BUILTIN, 0);
        analogWrite(LIGHT_TRIGGER_PIN, 0);
    }
}

void sendDocument(StaticJsonDocument<RESPONSE_BUFFER_SIZE> &doc)
{
    size_t length = measureJsonPretty(doc);

    server.setContentLength(length);

    server.send(200, "application/json");

    EthernetClient client = server.client();

    serializeJsonPretty(doc, client);
}

void sendError(uint32_t clientTransactionId, int32_t errorNumber, String errorMessage)
{
    StaticJsonDocument<RESPONSE_BUFFER_SIZE> doc;

    doc["ClientTransactionID"] = clientTransactionId;
    doc["ServerTransactionID"] = transactionId++;
    doc["ErrorNumber"] = errorNumber;
    doc["ErrorMessage"] = errorMessage;

    sendDocument(doc);
}

void handleNotImplemented()
{
    uint32_t clientTransactionID = strtoul(server.arg("ClientTransactionID").c_str(), NULL, 10);
    sendError(clientTransactionID, 0x400, "Not implemented");
}

void handleAlpacaManagementApiVersions()
{
    uint32_t clientTransactionID = strtoul(server.arg("ClientTransactionID").c_str(), NULL, 10);

    StaticJsonDocument<RESPONSE_BUFFER_SIZE> doc;

    JsonArray apiVersions = doc.createNestedArray("Value");
    apiVersions.add(1);
    doc["ClientTransactionID"] = clientTransactionID;
    doc["ServerTransactionID"] = transactionId++;

    sendDocument(doc);
}

void handleAlpacaManagementDescription()
{
    uint32_t clientTransactionID = strtoul(server.arg("ClientTransactionID").c_str(), NULL, 10);

    StaticJsonDocument<RESPONSE_BUFFER_SIZE> doc;

    doc["Value"]["Manufacturer"] = "Dark Dragons Astronomy, LLC";
    doc["Value"]["ManufacturerVersion"] = VERSION;
    doc["Value"]["Location"] = Ethernet.localIP().toString();
    doc["Value"]["ServerName"] = hostname;
    doc["ClientTransactionID"] = clientTransactionID;
    doc["ServerTransactionID"] = transactionId++;

    sendDocument(doc);
}

void handleAlpacaManagementConfiguredDevices()
{
    uint32_t clientTransactionID = strtoul(server.arg("ClientTransactionID").c_str(), NULL, 10);

    StaticJsonDocument<RESPONSE_BUFFER_SIZE> doc;

    JsonArray devices = doc.createNestedArray("Value");

    JsonObject device = devices.createNestedObject();

    device["DeviceName"] = "Dragon Light";
    device["DeviceType"] = "CoverCalibrator";
    device["DeviceNumber"] = 0;
    device["UniqueID"] = getBoardSerial();

    doc["ClientTransactionID"] = clientTransactionID;
    doc["ServerTransactionID"] = transactionId++;

    sendDocument(doc);
}

void handleAlpacaGetConnected()
{
    uint32_t clientTransactionID = strtoul(server.arg("ClientTransactionID").c_str(), NULL, 10);

    StaticJsonDocument<RESPONSE_BUFFER_SIZE> doc;

    doc["Value"] = isConnected;
    doc["ClientTransactionID"] = clientTransactionID;
    doc["ServerTransactionID"] = transactionId++;

    sendDocument(doc);
}

void handleAlpacaPutConnected()
{
    uint32_t clientTransactionID = strtoul(server.arg("ClientTransactionID").c_str(), NULL, 10);

    isConnected = server.arg("Connected").equalsIgnoreCase("true");

    StaticJsonDocument<RESPONSE_BUFFER_SIZE> doc;

    doc["ClientTransactionID"] = clientTransactionID;
    doc["ServerTransactionID"] = transactionId++;

    sendDocument(doc);
}

void handleAlpacaGetDescription()
{
    uint32_t clientTransactionID = strtoul(server.arg("ClientTransactionID").c_str(), NULL, 10);

    StaticJsonDocument<RESPONSE_BUFFER_SIZE> doc;

    doc["ClientTransactionID"] = clientTransactionID;
    doc["ServerTransactionID"] = transactionId++;
    doc["Value"] = "Dragon Light Flat Panel Controller";

    sendDocument(doc);
}

void handleAlpacaGetVersion()
{
    uint32_t clientTransactionID = strtoul(server.arg("ClientTransactionID").c_str(), NULL, 10);

    StaticJsonDocument<RESPONSE_BUFFER_SIZE> doc;

    doc["ClientTransactionID"] = clientTransactionID;
    doc["ServerTransactionID"] = transactionId++;
    doc["Value"] = VERSION;

    sendDocument(doc);
}

void handleAlpacaGetInterfaceVersion()
{
    uint32_t clientTransactionID = strtoul(server.arg("ClientTransactionID").c_str(), NULL, 10);

    StaticJsonDocument<RESPONSE_BUFFER_SIZE> doc;

    doc["ClientTransactionID"] = clientTransactionID;
    doc["ServerTransactionID"] = transactionId++;
    doc["Value"] = 2;

    sendDocument(doc);
}

void handleAlpacaGetName()
{
    uint32_t clientTransactionID = strtoul(server.arg("ClientTransactionID").c_str(), NULL, 10);

    StaticJsonDocument<RESPONSE_BUFFER_SIZE> doc;

    doc["ClientTransactionID"] = clientTransactionID;
    doc["ServerTransactionID"] = transactionId++;
    doc["Value"] = "Dragon Light";

    sendDocument(doc);
}

void handleAlpacaGetSupportedActions()
{
    uint32_t clientTransactionID = strtoul(server.arg("ClientTransactionID").c_str(), NULL, 10);

    StaticJsonDocument<RESPONSE_BUFFER_SIZE> doc;

    doc["ClientTransactionID"] = clientTransactionID;
    doc["ServerTransactionID"] = transactionId++;
    doc.createNestedArray("Value");

    sendDocument(doc);
}

void handleAlpacaGetBrightness()
{
    uint32_t clientTransactionID = strtoul(server.arg("ClientTransactionID").c_str(), NULL, 10);

    StaticJsonDocument<RESPONSE_BUFFER_SIZE> doc;

    doc["ClientTransactionID"] = clientTransactionID;
    doc["ServerTransactionID"] = transactionId++;
    doc["Value"] = lightLevel;

    sendDocument(doc);
}

void handleAlpacaGetLightState()
{
    uint32_t clientTransactionID = strtoul(server.arg("ClientTransactionID").c_str(), NULL, 10);

    StaticJsonDocument<RESPONSE_BUFFER_SIZE> doc;

    doc["ClientTransactionID"] = clientTransactionID;
    doc["ServerTransactionID"] = transactionId++;
    doc["Value"] = isLightOn && lightLevel > 0 ? 3 : 1;

    sendDocument(doc);
}

void handleAlpacaGetCoverState()
{
    uint32_t clientTransactionID = strtoul(server.arg("ClientTransactionID").c_str(), NULL, 10);

    StaticJsonDocument<RESPONSE_BUFFER_SIZE> doc;

    doc["ClientTransactionID"] = clientTransactionID;
    doc["ServerTransactionID"] = transactionId++;
    doc["Value"] = 0;

    sendDocument(doc);
}

void handleAlpacaGetMaxBrightness()
{
    uint32_t clientTransactionID = strtoul(server.arg("ClientTransactionID").c_str(), NULL, 10);

    StaticJsonDocument<RESPONSE_BUFFER_SIZE> doc;

    doc["ClientTransactionID"] = clientTransactionID;
    doc["ServerTransactionID"] = transactionId++;
    doc["Value"] = 255;

    sendDocument(doc);
}

void handleAlpacaPutLightOff()
{
    uint32_t clientTransactionID = strtoul(server.arg("ClientTransactionID").c_str(), NULL, 10);

    StaticJsonDocument<RESPONSE_BUFFER_SIZE> doc;

    doc["ClientTransactionID"] = clientTransactionID;
    doc["ServerTransactionID"] = transactionId++;

    isLightOn = false;
    lightLevel = 0;

    setLight();

    sendDocument(doc);
}

void handleAlpacaPutLightOn()
{
    uint32_t clientTransactionID = strtoul(server.arg("ClientTransactionID").c_str(), NULL, 10);
    long brightness = strtol(server.arg("Brightness").c_str(), NULL, 10);

    StaticJsonDocument<RESPONSE_BUFFER_SIZE> doc;

    doc["ClientTransactionID"] = clientTransactionID;
    doc["ServerTransactionID"] = transactionId++;

    isLightOn = true;

    if (brightness > 0)
    {
        lightLevel = brightness;
    }

    setLight();

    sendDocument(doc);
}

void handleGetStatus()
{
    StaticJsonDocument<RESPONSE_BUFFER_SIZE> doc;
    doc["power"] = isLightOn;
    doc["brightness"] = lightLevel;

    doc["serialNumber"] = getBoardSerial();
    doc["macAddress"] = macAddress;
    doc["version"] = VERSION;
    doc["ipAddress"] = Ethernet.localIP().toString();
    doc["hostname"] = hostname;

    sendDocument(doc);
}

void handlePostUpdate()
{
    StaticJsonDocument<RESPONSE_BUFFER_SIZE> doc;

    deserializeJson(doc, server.arg("plain"));

    isLightOn = doc["lightOn"].as<bool>();
    lightLevel = doc["lightLevel"].as<byte>();

    setLight();

    handleGetStatus();
}

void handleStaticFile()
{
    String path = server.uri();

    if (path.equals("/") || path.equals("/setup") || path.equals("/setup/v1/covercalibrator/0/setup"))
    {
        path = "/index.html";
    }

    if (path.equals("/firmware"))
    {
        path = "/update.html";
    }

    String contentType = getMimeType(path);

    path = path + ".gz";

    if (LittleFS.exists(path))
    {
        File file = LittleFS.open(path, "r");

        server.setContentLength(file.size());
        server.sendHeader("Content-Encoding", "gzip");
        server.send(200, contentType, "");

        char buffer[512];
        while (file.available())
        {
            size_t read = file.readBytes(buffer, sizeof(buffer));
            server.sendContent_P(buffer, read);
        }

        file.close();
    }
    else
    {
        server.send(404, "text/plain", "File not found");
    }
}

String firmwareZipFilePath;

void unzipFirmware(String path)
{
    UNZIP zip;
    unz_file_info fi;
    char filename[256];
    char comment[256];

    int rc = zip.openZIP(path.c_str(), zipOpen, zipClose, zipRead, zipSeek);

    if (rc != UNZ_OK)
    {
        Serial.println("error opening zip");
        return;
    }

    rc = zip.gotoFirstFile();

    uint8_t buffer[1024];

    while (rc == UNZ_OK)
    {
        watchdog_update();

        rc = zip.getFileInfo(&fi, filename, 256, NULL, 0, NULL, 0);

        String filePath = "/" + String(filename);

        if (LittleFS.exists(filePath))
        {
            LittleFS.remove(filePath);
        }

        zip.openCurrentFile();

        auto file = LittleFS.open(filePath, "w+b");

        int readBytes = 0;
        do
        {
            readBytes = zip.readCurrentFile(buffer, 1024);
            file.write(buffer, readBytes);
        } while (readBytes > 0);

        file.close();

        rc = zip.gotoNextFile();
    }

    zip.closeZIP();

    LittleFS.remove(path);

    sleep_ms(1000);

    Serial.println("update end");
}

void handlePostFirmware()
{
    fs::File zipFile = LittleFS.open(firmwareZipFilePath, "r");

    bool md5Mismatch = false;

    MD5Builder md5Builder;
    md5Builder.begin();
    md5Builder.addStream(zipFile, zipFile.size());
    md5Builder.calculate();

    Serial.println(md5Builder.toString());
    Serial.println(server.arg("md5"));

    md5Mismatch = md5Builder.toString() != server.arg("md5");

    zipFile.close();

    if (!md5Mismatch)
    {
        unzipFirmware(firmwareZipFilePath);

        fs::File md5File = LittleFS.open("/fw.bin.md5", "r");
        String md5 = md5File.readString();
        md5.trim();
        md5File.close();

        fs::File firmwareFile = LittleFS.open("/fw.bin", "r");
        if (Update.begin(firmwareFile.size()))
        {
            Update.setMD5(md5.c_str());
            Update.writeStream(firmwareFile);

            Update.end();
        }
    }

    if (Update.hasError())
    {
        server.send(500, "text/plain");
        auto client = server.client();
        Update.printError(client);
    }
    else if (md5Mismatch)
    {
        server.send(500, "text/plain", "MD5 mismatch");
    }
    else
    {
        handleStaticFile();
    }

    delay(1000);

    rp2040.restart();
}

fs::File fsUploadFile;
int uploadSize = 0;

void handleFileUpload()
{
    watchdog_update();

    ethernetHTTPUpload &upload = server.upload();

    String filename = upload.filename;

    if (!filename.startsWith("/"))
    {
        filename = "/" + filename;
    }

    if (!upload.filename.endsWith(".zip"))
    {
        return;
    }

    if (upload.status == UPLOAD_FILE_START)
    {
        fsUploadFile = LittleFS.open(filename, "w");
        uploadSize = 0;
    }
    else if (upload.status == UPLOAD_FILE_WRITE)
    {
        if (fsUploadFile)
        {
            size_t n = fsUploadFile.write(upload.buf, upload.currentSize);

            uploadSize += n;
        }
    }
    else if (upload.status == UPLOAD_FILE_END)
    {
        if (fsUploadFile)
        {
            fsUploadFile.close();
        }

        firmwareZipFilePath = filename;
    }
}

void blinkErrorCode(int blinks)
{
    while (true)
    {
        for (int i = 0; i < blinks; i++)
        {
            digitalWrite(LED_BUILTIN, HIGH);
            sleep_ms(200);
            digitalWrite(LED_BUILTIN, LOW);
            sleep_ms(200);
        }

        sleep_ms(5000);
    }
}

void setup()
{
    sleep_ms(5000);

    analogWriteFreq(5000); // 5 kHz
    analogWriteResolution(8);

    Serial.begin(9600);

    hostname = generateHostname("dragon-light-");
    mac = generateMacAddress(BOARD_ID);
    macAddress = getMac(BOARD_ID);

    Ethernet.setHostname(hostname.c_str());
    Ethernet.init(USE_THIS_SS_PIN);
    if (Ethernet.begin(mac, 10000, 4000) == 0)
    {
        Serial.println("Failed to configure Ethernet using DHCP");
        blinkErrorCode(5);
    }

    Serial.print(F("Connected! IP address: "));
    Serial.println(Ethernet.localIP());
    Serial.print(F("MAC Address: "));
    Serial.println(macAddress);

    if (!LittleFS.begin())
    {
        Serial.println("Failed to initialize file system");
        blinkErrorCode(3);
    }

#ifdef MDNS_FEATURE
    mdns.begin(Ethernet.localIP(), hostname.c_str());
    mdns.addServiceRecord("DragonLight._http",
                          80,
                          MDNSServiceTCP);
#endif

    Serial.print(F("Serial Number: "));
    Serial.println(getBoardSerial());
    Serial.print(F("Hostname: "));
    Serial.println(hostname);
    Serial.print(F("Web Server: "));
    Serial.print("http://");
    Serial.println(hostname);

    server.on("/setup", HTTP_GET, handleStaticFile);
    server.on("/setup/v1/covercalibrator/0/setup", HTTP_GET, handleStaticFile);

    server.on("/management/apiversions", HTTP_GET, handleAlpacaManagementApiVersions);
    server.on("/management/v1/description", HTTP_GET, handleAlpacaManagementDescription);
    server.on("/management/v1/configureddevices", HTTP_GET, handleAlpacaManagementConfiguredDevices);

    server.on("/api/v1/covercalibrator/0/action", HTTP_PUT, handleNotImplemented);
    server.on("/api/v1/covercalibrator/0/commandblind", HTTP_PUT, handleNotImplemented);
    server.on("/api/v1/covercalibrator/0/commandbool", HTTP_PUT, handleNotImplemented);
    server.on("/api/v1/covercalibrator/0/commandstring", HTTP_PUT, handleNotImplemented);

    server.on("/api/v1/covercalibrator/0/connected", HTTP_GET, handleAlpacaGetConnected);
    server.on("/api/v1/covercalibrator/0/connected", HTTP_PUT, handleAlpacaPutConnected);
    server.on("/api/v1/covercalibrator/0/description", HTTP_GET, handleAlpacaGetDescription);
    server.on("/api/v1/covercalibrator/0/driverinfo", HTTP_GET, handleAlpacaGetDescription);
    server.on("/api/v1/covercalibrator/0/driverversion", HTTP_GET, handleAlpacaGetVersion);
    server.on("/api/v1/covercalibrator/0/interfaceversion", HTTP_GET, handleAlpacaGetInterfaceVersion);
    server.on("/api/v1/covercalibrator/0/name", HTTP_GET, handleAlpacaGetName);
    server.on("/api/v1/covercalibrator/0/supportedactions", HTTP_GET, handleAlpacaGetSupportedActions);
    server.on("/api/v1/covercalibrator/0/brightness", HTTP_GET, handleAlpacaGetBrightness);
    server.on("/api/v1/covercalibrator/0/calibratorstate", HTTP_GET, handleAlpacaGetLightState);
    server.on("/api/v1/covercalibrator/0/coverstate", HTTP_GET, handleAlpacaGetCoverState);
    server.on("/api/v1/covercalibrator/0/maxbrightness", HTTP_GET, handleAlpacaGetMaxBrightness);
    server.on("/api/v1/covercalibrator/0/calibratoroff", HTTP_PUT, handleAlpacaPutLightOff);
    server.on("/api/v1/covercalibrator/0/calibratoron", HTTP_PUT, handleAlpacaPutLightOn);
    server.on("/api/v1/covercalibrator/0/closecover", HTTP_PUT, handleNotImplemented);
    server.on("/api/v1/covercalibrator/0/haltcover", HTTP_PUT, handleNotImplemented);
    server.on("/api/v1/covercalibrator/0/opencover", HTTP_PUT, handleNotImplemented);

    server.on("/status", HTTP_GET, handleGetStatus);
    server.on("/update", HTTP_POST, handlePostUpdate);
    server.on("/firmware", HTTP_POST, handlePostFirmware, handleFileUpload);

    server.onNotFound(handleStaticFile);

    server.begin();

    alpacaDiscovery.begin(32227);

    ddaDiscovery.begin(65534);

    auto dir = LittleFS.openDir("/");

    while (dir.next())
    {
        Serial.print(dir.fileName());
        Serial.print(" ");
        Serial.println(dir.fileSize());
    }

    Wire.setSDA(0);
    Wire.setSCL(1);

    Wire.begin();
    sht30.begin(0x44);
    Wire.setClock(100000);

    fs::FSInfo fs_info;
    LittleFS.info(fs_info);

    Serial.print("Total space: ");
    Serial.println(fs_info.totalBytes);
    Serial.print("Used space: ");
    Serial.println(fs_info.usedBytes);

    watchdog_enable(0x7fffff, false);
}

void checkForAlpacaDiscovery()
{
    int packetSize = alpacaDiscovery.parsePacket();
    if (packetSize > 0)
    {
        char message[64];
        memset(message, 0, sizeof(message));

        int ret = alpacaDiscovery.read(message, sizeof(message));

        if (strncmp("alpacadiscovery1", message, 16) == 0)
        {
            alpacaDiscovery.beginPacket(alpacaDiscovery.remoteIP(), alpacaDiscovery.remotePort());

            alpacaDiscovery.print("{\"alpacaport\":80}");

            alpacaDiscovery.endPacket();
        }
    }
}

void checkForDDADiscovery()
{
    int packetSize = ddaDiscovery.parsePacket();
    if (packetSize > 0)
    {
        char message[16];
        memset(message, 0, sizeof(message));

        int ret = ddaDiscovery.read(message, sizeof(message));

        if (strncmp("darkdragons", message, 11) == 0)
        {
            ddaDiscovery.beginPacket(ddaDiscovery.remoteIP(), ddaDiscovery.remotePort());
            ddaDiscovery.print("dragonlight");
            ddaDiscovery.endPacket();
        }
    }
}

unsigned long lastTempHumidityCheck = 0;

void checkTempHumidity()
{
    if (millis() - lastTempHumidityCheck > 1000)
    {
        lastTempHumidityCheck = millis();

        sht30.read();

        temperature = sht30.getTemperature();
        humidity = sht30.getHumidity();
        dewPoint = temperature - ((100 - humidity) / 5);

        Serial.print("Temp: ");
        Serial.print(temperature);
        Serial.print(" Humidity: ");
        Serial.print(humidity);
        Serial.print(" Dew Point: ");
        Serial.println(dewPoint);
    }
}

void loop()
{
    watchdog_update();

    Ethernet.maintain();
#ifdef MDNS_FEATURE
    mdns.run();
#endif

    checkTempHumidity();

    checkForAlpacaDiscovery();
    checkForDDADiscovery();
    server.handleClient();
}
