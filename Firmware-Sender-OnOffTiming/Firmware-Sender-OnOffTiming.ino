#include <nRF5x_BLE_API.h>
#define DEVICE_NAME       "PHLTX"
#define DATALENGTH        20 // bytes
#define LEDPIN            D13

BLE                       ble;

static uint8_t data[DATALENGTH]  = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
static const uint8_t location  = 0x03;

static const uint16_t uuid16_list[] = {GattService::UUID_HEART_RATE_SERVICE};

// Create characteristic and service
GattCharacteristic   hrmRate(GattCharacteristic::UUID_HEART_RATE_MEASUREMENT_CHAR, data, sizeof(data), sizeof(data), GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_NOTIFY);
GattCharacteristic   hrmLocation(GattCharacteristic::UUID_BODY_SENSOR_LOCATION_CHAR, (uint8_t *)&location, sizeof(location), sizeof(location), GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_READ);
GattCharacteristic   *hrmChars[] = {&hrmRate, &hrmLocation, };
GattService          hrmService(GattService::UUID_HEART_RATE_SERVICE, hrmChars, sizeof(hrmChars) / sizeof(GattCharacteristic *));

void disconnectionCallback(const Gap::DisconnectionCallbackParams_t *params) {
  //Serial.println("Disconnected - restart advertising");
  digitalWrite(LEDPIN, 0);
  ble.startAdvertising();
}

void connectionCallback(const Gap::ConnectionCallbackParams_t *params) {
  digitalWrite(LEDPIN, 1);
}

int dataIndex;

void setup() {
  dataIndex = 0;
  pinMode(LEDPIN, OUTPUT);

  Serial.begin(9600);
  while (!Serial) {
    delay(100); // wait for USB serial port to be available
  }

  // Init ble
  ble.init();
  ble.onConnection(connectionCallback);
  ble.onDisconnection(disconnectionCallback);

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
  if (Serial.available()) {
    int inChar = Serial.read();
    if (inChar == 0) {
      //for (int j = 0; j < DATALENGTH; j++) {
      //  Serial.print(data[j], HEX);
      //  Serial.print(" ");
      //}

      if (ble.getGapState().connected) {
        ble.updateCharacteristicValue(hrmRate.getValueAttribute().getHandle(), data, sizeof(data));
        Serial.println("OK");
      } else {
        Serial.println("Not sending - No Client connected");
      }

      for (int j = 0; j < DATALENGTH; j++)
        data[j] = 0;

      dataIndex = 0;

    } else {
      data[dataIndex] = (uint8_t)inChar;
      dataIndex++;
    }
  }

  ble.waitForEvent();
}

