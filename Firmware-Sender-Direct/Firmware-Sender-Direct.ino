#include <nRF5x_BLE_API.h>
#define DEVICE_NAME       "PHLTX"
#define DATALENGTH        1 // bytes

BLE                       ble;

static uint8_t data[DATALENGTH]  = {0x00};
static const uint8_t location  = 0x03;

static const uint16_t uuid16_list[] = {GattService::UUID_HEART_RATE_SERVICE};

// Create characteristic and service
GattCharacteristic   hrmRate(GattCharacteristic::UUID_HEART_RATE_MEASUREMENT_CHAR, data, sizeof(data), sizeof(data), GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_NOTIFY);
GattCharacteristic   hrmLocation(GattCharacteristic::UUID_BODY_SENSOR_LOCATION_CHAR, (uint8_t *)&location, sizeof(location), sizeof(location), GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_READ);
GattCharacteristic   *hrmChars[] = {&hrmRate, &hrmLocation, };
GattService          hrmService(GattService::UUID_HEART_RATE_SERVICE, hrmChars, sizeof(hrmChars) / sizeof(GattCharacteristic *));


void disconnectionCallBack(const Gap::DisconnectionCallbackParams_t *params) {
  Serial.println("Disconnected!");
  Serial.println("Restarting the advertising process");
  ble.startAdvertising();
}

int i;

void setup() {
  i = 0;
  // put your setup code here, to run once
  Serial.begin(9600);
  while (!Serial) {
    delay(100); // wait for USB serial port to be available
  }

  Serial.println("Initialize BLE_PHLTX");
  // Init ble
  ble.init();
  ble.onDisconnection(disconnectionCallBack);

  // setup adv_data and srp_data
  ble.accumulateAdvertisingPayload(GapAdvertisingData::BREDR_NOT_SUPPORTED | GapAdvertisingData::LE_GENERAL_DISCOVERABLE);
  ble.accumulateAdvertisingPayload(GapAdvertisingData::COMPLETE_LIST_16BIT_SERVICE_IDS, (uint8_t*)uuid16_list, sizeof(uuid16_list));
  ble.accumulateAdvertisingPayload(GapAdvertisingData::HEART_RATE_SENSOR_HEART_RATE_BELT);
  ble.accumulateAdvertisingPayload(GapAdvertisingData::COMPLETE_LOCAL_NAME, (uint8_t *)DEVICE_NAME, sizeof(DEVICE_NAME));
  // set adv_type
  ble.setAdvertisingType(GapAdvertisingParams::ADV_CONNECTABLE_UNDIRECTED);
  // add service
  ble.addService(hrmService);
  // set device name
  ble.setDeviceName((const uint8_t *)DEVICE_NAME);
  // set tx power,valid values are -40, -20, -16, -12, -8, -4, 0, 4
  ble.setTxPower(4);
  // set adv_interval, 100ms in multiples of 0.625ms.
  ble.setAdvertisingInterval(160);
  // set adv_timeout, in seconds
  ble.setAdvertisingTimeout(0);
  // start advertising
  ble.startAdvertising();
}

void loop() {
  // print the string when a newline arrives:

  if (Serial.available()) {
    int inChar = Serial.read();
    if (ble.getGapState().connected) {
      Serial.print("Sending ");
      Serial.println(inChar, DEC);
      data[0] = inChar;
      ble.updateCharacteristicValue(hrmRate.getValueAttribute().getHandle(), data, sizeof(data));
    }


    /*while (Serial.available()) {
      int inChar = Serial.read();
      Serial.print("InChar: ");
      Serial.print(inChar, HEX);
      Serial.print(" i=");
      Serial.print(i);
      if (inChar == '\n') {
        Serial.println(" Separator");
        for (int j = 0; j < DATALENGTH; j++) {
            Serial.print(data[j], HEX);
            Serial.print(" ");
        }
        uint16_t *pwm;
        pwm = (uint16_t *) data;
        Serial.print("pwm = ");
        Serial.println(*pwm);

        if (ble.getGapState().connected) {
          Serial.println("Sending");
          ble.updateCharacteristicValue(hrmRate.getValueAttribute().getHandle(), data, sizeof(data));
        } else {
          Serial.println("Not sending - not connected");
        }

        for (int j = 0; j < DATALENGTH; j++)
          data[j] = 0;

        i = 0;

      } else {
        data[i] = (uint8_t)inChar;
        Serial.print(" InChar as uint8_t:");
        Serial.println((uint8_t)inChar, HEX);
        i++;
      }
      }*/
  }

  ble.waitForEvent();
}

