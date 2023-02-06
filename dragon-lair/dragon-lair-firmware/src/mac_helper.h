#include <Arduino.h>
#include <pico/unique_id.h>

byte *generateMacAddress()
{
    static byte mac[6];
    pico_unique_board_id_t id;
    pico_get_unique_board_id(&id);

    mac[0] = 0x02; // Self Defined Mac
    mac[1] = 0xDC; // Dragon lair Controller
    mac[2] = id.id[4];
    mac[3] = id.id[5];
    mac[4] = id.id[6];
    mac[5] = id.id[7];

    return mac;
}

String getMac()
{
    byte *mac = generateMacAddress();

    char macStr[18];
    memset(&macStr, 0, 18);

    sprintf(macStr, "%02X:%02X:%02X:%02X:%02X:%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

    return String(macStr);
}

String getBoardSerial()
{
    static String serial;

    if (serial.length() > 0)
    {
        return serial;
    }

    char buffer[2 * PICO_UNIQUE_BOARD_ID_SIZE_BYTES + 1];
    pico_get_unique_board_id_string(buffer, sizeof(buffer));

    serial = String(buffer);

    return serial;
}

String generateHostname()
{
    return "dragon-lair-" + getBoardSerial();
}
