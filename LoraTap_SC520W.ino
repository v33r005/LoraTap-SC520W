/*
  Copyright (C) AC SOFTWARE SP. Z O.O.

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; either version 2
  of the License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

/* This example shows ESP82xx/ESP32 based device with simple WebInterface
 * used to configure Wi-Fi parameters and Supla server connection.
 * There is one RollerShutter, one Relay and 3 buttons configured.
 * Two buttons are for roller shutter with Action Trigger.
 * Third button is for controlling the relay and for switching module to
 * config mode.
 * After fresh installation, device will be in config mode. It will have its
 * own Wi-Fi AP configured. You should connect to it with your mobile phone
 * and open http://192.168.4.1 where you can configure the device.
 * Status LED is also configured. Please adjust GPIOs to your HW.
 */

#define STATUS_LED_GPIO 18
#define ROLLER_SHUTTER_UP_RELAY_GPIO1 7
#define ROLLER_SHUTTER_DOWN_RELAY_GPIO1 4
#define ROLLER_SHUTTER_UP_RELAY_GPIO2 5
#define ROLLER_SHUTTER_DOWN_RELAY_GPIO2 1
#define BUTTON_UP_GPIO1 8
#define BUTTON_DOWN_GPIO1 9
#define BUTTON_UP_GPIO2 10
#define BUTTON_DOWN_GPIO2 3
#define BUTTON_CFG_RELAY_GPIO 6

#include <SuplaDevice.h>
#include <supla/network/esp_wifi.h>
#include <supla/control/roller_shutter.h>
#include <supla/control/relay.h>
#include <supla/control/button.h>
#include <supla/control/action_trigger.h>
#include <supla/device/status_led.h>
#include <supla/storage/littlefs_config.h>
#include <supla/network/esp_web_server.h>
#include <supla/network/html/device_info.h>
#include <supla/network/html/protocol_parameters.h>
#include <supla/network/html/status_led_parameters.h>
#include <supla/network/html/wifi_parameters.h>
#include <supla/device/supla_ca_cert.h>
#include <supla/events.h>
#include <supla/network/html/custom_text_parameter.h>

const char DEV_NAME[] = "dev_name";
char devName[30] = {};

// Choose where Supla should store roller shutter data in persistent memory
// We recommend to use external FRAM memory
#include <supla/storage/eeprom.h>
Supla::Eeprom eeprom;
// #include <supla/storage/fram_spi.h>
// Supla::FramSpi fram(STORAGE_OFFSET);

Supla::ESPWifi wifi;
Supla::LittleFsConfig configSupla;

Supla::Device::StatusLed statusLed(STATUS_LED_GPIO, true); // inverted state
Supla::EspWebServer suplaServer;


// HTML www component (they appear in sections according to creation
// sequence)
Supla::Html::DeviceInfo htmlDeviceInfo(&SuplaDevice);
Supla::Html::WifiParameters htmlWifi;
Supla::Html::ProtocolParameters htmlProto;
Supla::Html::StatusLedParameters htmlStatusLed;

void setup() {

  Serial.begin(115200);
  //******************własna nazwa*****************************
  Supla::Storage::Init();
  if (Supla::Storage::ConfigInstance()->getString(DEV_NAME, devName, 30)) {
    SUPLA_LOG_DEBUG("# Param[%s]: %s", DEV_NAME, devName);
  } else {
    Supla::Storage::ConfigInstance()->setString(DEV_NAME, "Supla-SC520W");
  }
//****************************************************************

  // Channels configuration
  // CH 0 - Roller shutter
  auto rs1 = new Supla::Control::RollerShutter(ROLLER_SHUTTER_UP_RELAY_GPIO1, ROLLER_SHUTTER_DOWN_RELAY_GPIO1, true);
  auto rs2 = new Supla::Control::RollerShutter(ROLLER_SHUTTER_UP_RELAY_GPIO2, ROLLER_SHUTTER_DOWN_RELAY_GPIO2, true);

  auto at1 = new Supla::Control::ActionTrigger();
  auto at2 = new Supla::Control::ActionTrigger();
  auto at3 = new Supla::Control::ActionTrigger();
  auto at4 = new Supla::Control::ActionTrigger();

  // Buttons configuration
  auto buttonOpen1 = new Supla::Control::Button(BUTTON_UP_GPIO1, true, true);
  auto buttonClose1 = new Supla::Control::Button(BUTTON_DOWN_GPIO1, true, true);
  auto buttonOpen2 = new Supla::Control::Button(BUTTON_UP_GPIO2, true, true);
  auto buttonClose2 = new Supla::Control::Button(BUTTON_DOWN_GPIO2, true, true);
  auto buttonCfgRelay = new Supla::Control::Button(BUTTON_CFG_RELAY_GPIO, true, true);

  buttonOpen1->addAction(Supla::OPEN_OR_STOP, *rs1, Supla::ON_PRESS);
  buttonOpen1->setHoldTime(1000);
  buttonOpen1->setMulticlickTime(300);

  buttonClose1->addAction(Supla::CLOSE_OR_STOP, *rs1, Supla::ON_PRESS);
  buttonClose1->setHoldTime(1000);
  buttonClose1->setMulticlickTime(300);

  buttonOpen2->addAction(Supla::OPEN_OR_STOP, *rs2, Supla::ON_PRESS);
  buttonOpen2->setHoldTime(1000);
  buttonOpen2->setMulticlickTime(300);

  buttonClose2->addAction(Supla::CLOSE_OR_STOP, *rs2, Supla::ON_PRESS);
  buttonClose2->setHoldTime(1000);
  buttonClose2->setMulticlickTime(300);

  buttonCfgRelay->configureAsConfigButton(&SuplaDevice);


  // Action trigger configuration
  at1->setRelatedChannel(rs1);
  at1->attach(buttonOpen1);

  at2->setRelatedChannel(rs1);
  at2->attach(buttonClose1);

  at3->setRelatedChannel(rs2);
  at3->attach(buttonOpen2);

  at4->setRelatedChannel(rs2);
  at4->attach(buttonClose2);

  new Supla::Html::CustomTextParameter(DEV_NAME, "Device name", 30);
  // configure defualt Supla CA certificate
  SuplaDevice.setName(devName);
  SuplaDevice.setSuplaCACert(suplaCACert);
  SuplaDevice.setSupla3rdPartyCACert(supla3rdCACert);

  SuplaDevice.begin();
}

void loop() {
  SuplaDevice.iterate();
}
