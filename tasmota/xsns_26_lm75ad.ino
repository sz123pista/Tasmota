/*
  xsns_26_lm75ad.ino - Support for I2C LM75AD Temperature Sensor

  Copyright (C) 2021  Andre Thomas and Theo Arends

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifdef USE_I2C
#ifdef USE_LM75AD

/*********************************************************************************************\
 * LM75AD - Temperature
 *
 * Docs at https://www.nxp.com/docs/en/data-sheet/LM75A.pdf
 *
 * I2C Address: 0x48 - 0x4F
\*********************************************************************************************/

#define XSNS_26                 26
#define XSNS_120                120
#define XSNS_121                121
#define XSNS_122                122
#define XSNS_123                123
#define XSNS_124                124
#define XSNS_125                125
#define XSNS_126                126

#define XI2C_20                 20  // See I2CDEVICES.md

#define LM75AD_ADDRESS1					0x48
#define LM75AD_ADDRESS2					0x49
#define LM75AD_ADDRESS3					0x4A
#define LM75AD_ADDRESS4					0x4B
#define LM75AD_ADDRESS5					0x4C
#define LM75AD_ADDRESS6					0x4D
#define LM75AD_ADDRESS7					0x4E
#define LM75AD_ADDRESS8					0x4F

#define LM75_TEMP_REGISTER      0x00
#define LM75_CONF_REGISTER      0x01
#define LM75_THYST_REGISTER     0x02
#define LM75_TOS_REGISTER       0x03

bool lm75ad_type[8] = {0};
uint8_t lm75ad_addresses[] = { LM75AD_ADDRESS1, LM75AD_ADDRESS2, LM75AD_ADDRESS3, LM75AD_ADDRESS4, LM75AD_ADDRESS5, LM75AD_ADDRESS6, LM75AD_ADDRESS7, LM75AD_ADDRESS8 };

void LM75ADDetect(uint32_t idx)
{
  uint8_t lm75ad_address = lm75ad_addresses[idx];
  if (!I2cSetDevice(lm75ad_address)) {
    return; // do not make the next step without a confirmed device on the bus
  }
  uint16_t buffer;
  if (I2cValidRead16(&buffer, lm75ad_address, LM75_THYST_REGISTER)) {
    if (buffer == 0x4B00) {
      lm75ad_type[idx] = true;
      char name_buf[16] = {0};
      snprintf(name_buf, sizeof(name_buf), "LM75AD_%d", idx);
      I2cSetActiveFound(lm75ad_address, name_buf);
    }
  }
}

static const uint8_t MCP980x_RESOLUTION_MASK = 0x60;

float LM75ADGetTemp(uint32_t idx)
{
  int16_t sign = 1;

  uint16_t t = I2cRead16(lm75ad_addresses[idx], LM75_TEMP_REGISTER);

  uint8_t conf = I2cRead8(lm75ad_addresses[idx], LM75_CONF_REGISTER);
  //check the resolution config, make sure it is always 12 bits
  if (conf & MCP980x_RESOLUTION_MASK != MCP980x_RESOLUTION_MASK) {
    conf |= MCP980x_RESOLUTION_MASK;
    I2cWrite8(lm75ad_addresses[idx], LM75_CONF_REGISTER, conf);
  }

  if (t & 0x8000) { // we are getting a negative temperature value
    t = (~t) +0x10;
    sign = -1;
  }
  t = t >> 4; // shift value into place (5 LSB not used)
  return ConvertTemp(sign * t * 0.0625);
}

void LM75ADShow(uint32_t idx, bool json)
{
  float t = LM75ADGetTemp(idx);

  char name_buf[16] = {0};
  snprintf(name_buf, sizeof(name_buf), "LM75AD_%d", idx);

  if (json) {
    ResponseAppend_P(JSON_SNS_F_TEMP, name_buf, Settings->flag2.temperature_resolution, &t);
#ifdef USE_DOMOTICZ
    if (0 == TasmotaGlobal.tele_period) DomoticzFloatSensor(DZ_TEMP, t);
#endif  // USE_DOMOTICZ
#ifdef USE_WEBSERVER
  } else {
    WSContentSend_Temp(name_buf, t);
#endif  // USE_WEBSERVER
  }
}

/*********************************************************************************************\
 * Interface
\*********************************************************************************************/

bool XsnsX(uint32_t idx, uint8_t function)
{
  if (!I2cEnabled(XI2C_20)) { return false; }

  bool result = false;

  if (FUNC_INIT == function) {
    LM75ADDetect(idx);
  }
  else if (lm75ad_type[idx]) {
    switch (function) {
      case FUNC_JSON_APPEND:
        LM75ADShow(idx, 1);
        break;
  #ifdef USE_WEBSERVER
      case FUNC_WEB_SENSOR:
        LM75ADShow(idx, 0);
        break;
  #endif  // USE_WEBSERVER
    }
  }
  return result;
}

bool Xsns26(uint8_t function) {
  return XsnsX(0, function);
}

bool Xsns120(uint8_t function) {
  return XsnsX(1, function);
}

bool Xsns121(uint8_t function) {
  return XsnsX(2, function);
}

bool Xsns122(uint8_t function) {
  return XsnsX(3, function);
}

bool Xsns123(uint8_t function) {
  return XsnsX(4, function);
}

bool Xsns124(uint8_t function) {
  return XsnsX(5, function);
}

bool Xsns125(uint8_t function) {
  return XsnsX(6, function);
}

bool Xsns126(uint8_t function) {
  return XsnsX(7, function);
}

#endif  // USE_LM75AD
#endif  // USE_I2C
