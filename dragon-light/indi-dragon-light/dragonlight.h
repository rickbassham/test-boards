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

#include <libindi/defaultdevice.h>
#include <libindi/indilightboxinterface.h>

namespace INDI
{
/**
 * @brief The DragonLight class interfaces with the Dark Dragons Astronomy
 * Dragon Light Flat Panel Controller.
 *
 * @author Rick Bassham
 */
class DragonLight : public INDI::DefaultDevice, public INDI::LightBoxInterface
{
public:
    DragonLight(std::string ipAddress);
    virtual ~DragonLight() = default;

    void ISGetProperties(const char *dev) override;

    virtual bool ISNewNumber(const char *dev, const char *name, double values[], char *names[], int n) override;
    virtual bool ISNewSwitch(const char *dev, const char *name, ISState *states, char *names[], int n) override;
    virtual bool ISNewText(const char *dev, const char *name, char *texts[], char *names[], int n) override;
    virtual bool ISSnoopDevice(XMLEle *root) override;

protected:
    virtual bool initProperties() override;
    virtual bool updateProperties() override;
    const char *getDefaultName() override;
    virtual bool saveConfigItems(FILE *fp) override;

    virtual bool Connect() override;
    virtual bool Disconnect() override;
    void TimerHit() override;
    virtual bool SetLightBoxBrightness(uint16_t value) override;
    virtual bool EnableLightBox(bool enable) override;

private:
    bool checkStatus();
    bool turnOn(uint8_t brightness);
    bool turnOff();

    bool updateManagementDescription();
    bool updateManagementConfiguredDevices();
    bool updateDeviceDescription();
    bool updateDeviceVersion();

    enum Description
    {
        MANUFACTURER,
        MANUFACTURER_VERSION,
        LOCATION,
        SERVER_NAME,
        DESCRIPTION_LEN,
    };
    INDI::PropertyText descriptionTP{Description::DESCRIPTION_LEN};

    enum Device
    {
        DEVICE_NAME,
        DEVICE_TYPE,
        DEVICE_NUMBER,
        UNIQUE_ID,
        DESCRIPTION,
        DEVICE_VERSION,
        DEVICE_LEN,
    };
    INDI::PropertyText deviceTP{Device::DEVICE_LEN};

private:
    std::string _ipAddress;
}; // class DragonLight

}; // namespace INDI
