#include <Arduino.h>
#include <Ethernet_Generic.h>

EthernetServer server(80);

struct request_t
{
    String method;
    String path;
    String version;
    long contentLength;
    String connection;
    String contentType;
    String ifNoneMatch;
};

struct response_t
{
    String version;
    int statusCode;
    String statusMessage;
    long contentLength;
    String connection;
    String contentType;
    String contentEncoding;
    String etag;
};

typedef void (*request_handler_t)(EthernetClient &client, const request_t req);

int timedRead(EthernetClient &client)
{
    int c;
    unsigned long startMillis = millis();
    do
    {
        c = client.read();
        if (c >= 0)
            return c;
    } while (millis() - startMillis < 1000);
    return -1; // -1 indicates timeout
}

String readStringUntilOneOf(EthernetClient &client, String delimiters)
{
    String ret;
    int c = timedRead(client);
    while (c >= 0 && delimiters.indexOf(c) == -1)
    {
        ret += (char)c;
        c = timedRead(client);
    }
    return ret;
}

void readStartLine(EthernetClient &client, request_t &request)
{
    String line = client.readStringUntil('\r');
    client.read(); // read the \n

    int firstSpace = line.indexOf(' ');
    int secondSpace = line.indexOf(' ', firstSpace + 1);

    request.method = line.substring(0, firstSpace);
    request.path = line.substring(firstSpace + 1, secondSpace);
    request.version = line.substring(secondSpace + 1);
}

bool readHeader(EthernetClient &client, request_t &request)
{
    String name = readStringUntilOneOf(client, ":\r");
    if (name == "")
    {
        client.read(); // read the \n
        return false;
    }

    client.read(); // read the :space:

    name.toLowerCase();

    if (name == "content-length")
    {
        request.contentLength = client.readStringUntil('\r').toInt();
        client.read(); // read the \n
    }
    else if (name == "connection")
    {
        request.connection = client.readStringUntil('\r');
        client.read(); // read the \n
    }
    else if (name == "content-type")
    {
        request.contentType = client.readStringUntil('\r');
        client.read(); // read the \n
    }
    else if (name == "if-none-match")
    {
        request.ifNoneMatch = client.readStringUntil('\r');
        client.read(); // read the \n
    }
    else
    {
        client.find('\n');
    }

    return true;
}

void readHeaders(EthernetClient &client, request_t &request)
{
    while (readHeader(client, request))
    {
    }
}

void sendHeaders(Stream *stream, const response_t &res, bool debug = false)
{
#ifdef DEBUG
    if (!debug)
    {
        Serial.println("response: -------------------------");
        sendHeaders(&Serial, res, true);
    }
#endif

    stream->print(res.version);
    stream->print(' ');
    stream->print(res.statusCode);
    stream->print(' ');
    stream->println(res.statusMessage);

    if (res.contentLength >= 0)
    {
        stream->print("Content-Length: ");
        stream->println(res.contentLength);
    }

    if (res.connection.length() > 0)
    {
        stream->print("Connection: ");
        stream->println(res.connection);
    }

    if (res.contentType.length() > 0)
    {
        stream->print("Content-Type: ");
        stream->println(res.contentType);
    }

    if (res.etag.length() > 0)
    {
        stream->print("ETag: ");
        stream->println(res.etag);
    }

    if (res.contentEncoding.length() > 0)
    {
        stream->print("Content-Encoding: ");
        stream->println(res.contentEncoding);
    }

    stream->println();
}

const response_t notFound = {
    "HTTP/1.1",
    404,
    "Not Found",
    0,
    "close",
    "text/plain",
    "",
    ""};

const response_t notModified = {
    "HTTP/1.1",
    304,
    "Not Modified",
    0,
    "close",
    "text/plain",
    "",
    ""};

void handleClient(request_handler_t handler)
{
    EthernetClient client = server.accept();
    if (client)
    {
        while (client.connected())
        {
#ifdef DEBUG
            Serial.println("Client connected");
#endif
            request_t req;
            req.contentLength = -1;

            if (client.available())
            {
#ifdef DEBUG
                Serial.println("request: -------------------------");
#endif
                readStartLine(client, req);

#ifdef DEBUG
                Serial.print("Method: ");
                Serial.println(req.method);

                Serial.print("Path: ");
                Serial.println(req.path);

                Serial.print("Version: ");
                Serial.println(req.version);
#endif

                readHeaders(client, req);

#ifdef DEBUG
                Serial.print("Content-Length: ");
                Serial.println(req.contentLength);

                Serial.print("Connection: ");
                Serial.println(req.connection);

                Serial.print("Content-Type: ");
                Serial.println(req.contentType);

                Serial.print("If-None-Match: ");
                Serial.println(req.ifNoneMatch);
#endif

                handler(client, req);

                // sleep_ms(100);

                // client.flush();
                // client.stop();
            }
#ifdef DEBUG
            else
            {
                Serial.println("Client not available");
            }
#endif
        }

        client.stop();
    }
}