/*******************************************************************************
 Copyright(c) 2021 Rick Bassham. All rights reserved.

 This library is free software; you can redistribute it and/or
 modify it under the terms of the GNU Library General Public
 License version 2 as published by the Free Software Foundation.
 .
 This library is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 Library General Public License for more details.
 .
 You should have received a copy of the GNU Library General Public License
 along with this library; see the file COPYING.LIB.  If not, write to
 the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 Boston, MA 02110-1301, USA.
*******************************************************************************/

#include "dragonlight.h"
#include "config.h"

#include <libindi/connectionplugins/connectiontcp.h>
#include <libindi/indicom.h>

#include <cmath>
#include <cstring>
#include <ctime>
#include <memory>
#include <termios.h>
#include <deque>

#if defined(_WIN32) || defined(__USE_W32_SOCKETS)
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>
#ifdef _MSC_VER
#pragma comment(lib, "IPHLPAPI.lib")
#pragma comment(lib, "Ws2_32.lib")
#endif
#else
#if defined(__FreeBSD__) || defined(__NetBSD__) || defined(__OpenBSD__) ||                     \
        defined(__bsdi__) || defined(__DragonFly__)
#include <sys/socket.h>
#include <netinet/in.h>
#endif
#include <arpa/inet.h>
#include <ifaddrs.h>
#endif

#include "jsonRequest.h"

using namespace INDI;

// 0x23 is the stop char '#'
#define DRIVER_STOP_CHAR char(0x23)
// Wait up to a maximum of 3 seconds for serial input
#define DRIVER_TIMEOUT 3
// Maximum buffer for sending/receving.
#define DRIVER_LEN 64


static class Loader
{
    std::deque<std::unique_ptr<DragonLight>> lights;
public:
    Loader()
    {
        IDLog("Loading DragonLight driver\n");
        discover();
    }

    void discover()
    {
        IDLog("Sending discovery packet\n");

        sockaddr_in si_me, si_other;
        int s;

        s=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
        if (s==-1)
        {
            IDLog("Error creating socket\n");
            return;
        }

        int port=65534;

        int broadcast=1;
        setsockopt(s, SOL_SOCKET, SO_BROADCAST,
                   &broadcast, sizeof(broadcast));

        struct timeval tv;
        tv.tv_sec = 1;
        tv.tv_usec = 0;
        setsockopt(s, SOL_SOCKET, SO_RCVTIMEO,&tv,sizeof(tv));

        struct sockaddr_in Recv_addr;
        struct sockaddr_in Sender_addr;

        socklen_t len = sizeof(struct sockaddr_in);

        char recvbuff[64] = "";
        int recvbufflen = 64;

        Recv_addr.sin_family       = AF_INET;
        Recv_addr.sin_port         = htons(65534);
        Recv_addr.sin_addr.s_addr  = INADDR_BROADCAST;

        char sendMSG[12] = "darkdragons";

        sendto(s,sendMSG,strlen(sendMSG)+1,0,(sockaddr *)&Recv_addr,sizeof(Recv_addr));

        while (true)
        {
            int n = recvfrom(s,recvbuff,recvbufflen,0,(sockaddr *)&Recv_addr,&len);
            if (n < 0)
                break;

            IDLog("Received: %s\n", recvbuff);

            if (strncasecmp(recvbuff, "dragonlight", 11) == 0)
            {
                char str[INET_ADDRSTRLEN];
                inet_ntop(AF_INET, &(Recv_addr.sin_addr), str, INET_ADDRSTRLEN);

                IDLog("IP: %s\n", str);

                lights.push_back(std::unique_ptr<DragonLight>(new DragonLight(std::string(str))));
            }
        }

        IDLog("discovery complete\n");
    }
} loader;

DragonLight::DragonLight(std::string ipAddress) : LightBoxInterface(this, true)
{
    setVersion(VERSION_MAJOR, VERSION_MINOR);
    _ipAddress = ipAddress;
}

void DragonLight::ISGetProperties(const char *dev)
{
    INDI::DefaultDevice::ISGetProperties(dev);

    // Get Light box properties
    isGetLightBoxProperties(dev);
}

bool DragonLight::initProperties()
{
    INDI::DefaultDevice::initProperties();
    setDriverInterface(AUX_INTERFACE | LIGHTBOX_INTERFACE);

    descriptionTP[Description::MANUFACTURER].fill("MANUFACTURER", "Manufacturer", nullptr);
    descriptionTP[Description::MANUFACTURER_VERSION].fill("MANUFACTURER_VERSION", "Version", nullptr);
    descriptionTP[Description::LOCATION].fill("LOCATION", "Location", nullptr);
    descriptionTP[Description::SERVER_NAME].fill("SERVER_NAME", "Server Name", nullptr);

    descriptionTP.fill(getDeviceName(), "DESCRIPTION_INFO", "Description", INFO_TAB, IP_RO, 60, IPS_IDLE);

    registerProperty(descriptionTP);

    deviceTP[Device::DEVICE_NAME].fill("DEVICE_NAME", "Device Name", nullptr);
    deviceTP[Device::DEVICE_TYPE].fill("DEVICE_TYPE", "Device Type", nullptr);
    deviceTP[Device::DEVICE_NUMBER].fill("DEVICE_NUMBER", "Device Number", nullptr);
    deviceTP[Device::UNIQUE_ID].fill("UNIQUE_ID", "Unique ID", nullptr);
    deviceTP[Device::DESCRIPTION].fill("DESCRIPTION", "Description", nullptr);
    deviceTP[Device::DEVICE_VERSION].fill("DEVICE_VERSION", "Device Version", nullptr);

    deviceTP.fill(getDeviceName(), "DEVICE_INFO", "Device", INFO_TAB, IP_RO, 60, IPS_IDLE);

    registerProperty(deviceTP);
    initLightBoxProperties(getDeviceName(), MAIN_CONTROL_TAB);
    addAuxControls();

    return true;
}

bool DragonLight::updateProperties()
{
    INDI::DefaultDevice::updateProperties();

    if (isConnected())
    {
        defineProperty(&LightSP);
        defineProperty(&LightIntensityNP);
        defineProperty(&descriptionTP);
        defineProperty(&deviceTP);
    }
    else
    {
        deleteProperty(LightSP.name);
        deleteProperty(LightIntensityNP.name);
        deleteProperty(descriptionTP.getName());
        deleteProperty(deviceTP.getName());
    }

    updateLightBoxProperties();

    return true;
}

const char *DragonLight::getDefaultName()
{
    return (const char *)"Dragon Light";
}

bool DragonLight::updateManagementDescription()
{
    char url[128];
    memset(url, 0, 128);
    sprintf(url, "http://%s/management/v1/description", _ipAddress.c_str());

    auto result = get_json(url);

    if (result == nullptr)
    {
        IDLog("Failed to connect to %s\n", _ipAddress.c_str());
        return false;
    }

    descriptionTP[Description::MANUFACTURER].setText(result["Value"]["Manufacturer"].get<std::string>());
    descriptionTP[Description::MANUFACTURER_VERSION].setText(result["Value"]["ManufacturerVersion"].get<std::string>());
    descriptionTP[Description::LOCATION].setText(result["Value"]["Location"].get<std::string>());
    descriptionTP[Description::SERVER_NAME].setText(result["Value"]["ServerName"].get<std::string>());

    IDLog("Manufacturer: %s\n", result["Value"]["Manufacturer"].get<std::string>());

    descriptionTP.apply();

    return true;
}

bool DragonLight::updateManagementConfiguredDevices()
{
    char url[128];
    memset(url, 0, 128);
    sprintf(url, "http://%s/management/v1/configureddevices", _ipAddress.c_str());

    auto result = get_json(url);

    if (result == nullptr)
    {
        IDLog("Failed to connect to %s\n", _ipAddress.c_str());
        return false;
    }

    auto device = result["Value"][0];

    int deviceNumber = device["DeviceNumber"].get<int>();

    deviceTP[Device::DEVICE_NAME].setText(device["DeviceName"].get<std::string>());
    deviceTP[Device::DEVICE_TYPE].setText(device["DeviceType"].get<std::string>());
    deviceTP[Device::DEVICE_NUMBER].setText(std::to_string(deviceNumber));
    deviceTP[Device::UNIQUE_ID].setText(device["UniqueID"].get<std::string>());

    deviceTP.apply();

    return true;
}

bool DragonLight::updateDeviceDescription()
{
    char url[128];
    memset(url, 0, 128);
    sprintf(url, "http://%s/api/v1/covercalibrator/0/description", _ipAddress.c_str());

    auto result = get_json(url);

    if (result == nullptr)
    {
        IDLog("Failed to connect to %s\n", _ipAddress.c_str());
        return false;
    }

    deviceTP[Device::DESCRIPTION].setText(result["Value"].get<std::string>());

    deviceTP.apply();

    return true;
}

bool DragonLight::updateDeviceVersion()
{
    char url[128];
    memset(url, 0, 128);
    sprintf(url, "http://%s/api/v1/covercalibrator/0/driverversion", _ipAddress.c_str());

    auto result = get_json(url);

    if (result == nullptr)
    {
        IDLog("Failed to connect to %s\n", _ipAddress.c_str());
        return false;
    }

    deviceTP[Device::DEVICE_VERSION].setText(result["Value"].get<std::string>());

    deviceTP.apply();

    return true;
}

bool DragonLight::Connect()
{
    if (!updateManagementDescription())
        return false;

    if (!updateManagementConfiguredDevices())
        return false;

    if (!updateDeviceDescription())
        return false;

    if (!updateDeviceVersion())
        return false;

    return true;
}

bool DragonLight::Disconnect()
{
    return true;
}

bool DragonLight::turnOn(uint8_t brightness)
{
    char url[128];
    memset(url, 0, 128);
    sprintf(url, "http://%s/api/v1/covercalibrator/0/calibratoron", _ipAddress.c_str());

    auto body = std::map<std::string, std::string>();
    body["Brightness"] = std::to_string(brightness);

    auto result = put_json(url, body);

    if (result == nullptr)
    {
        IDLog("Failed to connect to %s\n", _ipAddress.c_str());
        return false;
    }

    return true;
}

bool DragonLight::turnOff()
{
    char url[128];
    memset(url, 0, 128);
    sprintf(url, "http://%s/api/v1/covercalibrator/0/calibratoroff", _ipAddress.c_str());

    auto body = std::map<std::string, std::string>();
    auto result = put_json(url, body);

    if (result == nullptr)
    {
        IDLog("Failed to connect to %s\n", _ipAddress.c_str());
        return false;
    }

    return true;
}

bool DragonLight::EnableLightBox(bool enable)
{
    if (enable)
    {
        uint8_t value = (uint8_t)LightIntensityN[0].value;
        return turnOn(value);
    }
    else
    {
        return turnOff();
    }
}

bool DragonLight::SetLightBoxBrightness(uint16_t value)
{
    uint16_t original = (uint16_t)LightIntensityN[0].value;

    if (value == original)
        return true;

    LightIntensityN[0].value = value;
    IDSetNumber(&LightIntensityNP, nullptr);

    if (LightS[FLAT_LIGHT_ON].s == ISS_ON)
    {
        return turnOn(value);
    }

    return true;
}

bool DragonLight::ISNewNumber(const char *dev, const char *name, double values[], char *names[], int n)
{
    if (processLightBoxNumber(dev, name, values, names, n))
    {
        return true;
    }

    return INDI::DefaultDevice::ISNewNumber(dev, name, values, names, n);
}

bool DragonLight::ISNewSwitch(const char *dev, const char *name, ISState *states, char *names[], int n)
{
    if (processLightBoxSwitch(dev, name, states, names, n))
    {
        return true;
    }

    return INDI::DefaultDevice::ISNewSwitch(dev, name, states, names, n);
}

bool DragonLight::ISNewText(const char *dev, const char *name, char *texts[], char *names[], int n)
{
    if (processLightBoxText(dev, name, texts, names, n))
    {
        return true;
    }

    return INDI::DefaultDevice::ISNewText(dev, name, texts, names, n);
}

bool DragonLight::ISSnoopDevice(XMLEle *root)
{
    snoopLightBox(root);

    return INDI::DefaultDevice::ISSnoopDevice(root);
}

bool DragonLight::saveConfigItems(FILE *fp)
{
    INDI::DefaultDevice::saveConfigItems(fp);

    return saveLightBoxConfigItems(fp);
}

bool DragonLight::checkStatus()
{
    return false;
}

void DragonLight::TimerHit()
{
    if (!isConnected())
        return;

    checkStatus();

    SetTimer(getCurrentPollingPeriod());
}
