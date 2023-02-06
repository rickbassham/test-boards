#include <Arduino.h>

String getMimeType(String path)
{
    String contentType = "text/plain";

    if (path.endsWith(".html") || path.endsWith(".htm"))
    {
        contentType = "text/html";
    }
    else if (path.endsWith(".css"))
    {
        contentType = "text/css";
    }
    else if (path.endsWith(".js"))
    {
        contentType = "application/javascript";
    }
    else if (path.endsWith(".json"))
    {
        contentType = "application/json";
    }
    else if (path.endsWith(".svg"))
    {
        contentType = "image/svg+xml";
    }

    else if (path.endsWith(".png"))
    {
        contentType = "image/png";
    }

    return contentType;
}