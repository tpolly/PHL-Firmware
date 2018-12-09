#include <BLE_API.h>

#include <Adafruit_TLC59711.h>

#define LED_BUILTIN 13

// SPI

#define dataPin   A4
#define clockPin  A3

#define NUM_TLC59711 2  // number of chained TLCs
Adafruit_TLC59711 tlc = Adafruit_TLC59711(NUM_TLC59711, clockPin, dataPin);

BLE           ble;
Ticker        ticker;

static uint8_t service1_uuid[]    = {0x71, 0x3D, 0, 0, 0x50, 0x3E, 0x4C, 0x75, 0xBA, 0x94, 0x31, 0x48, 0xF1, 0x8D, 0x94, 0x1E};
static uint8_t service1_chars1[]  = {0x71, 0x3D, 0, 2, 0x50, 0x3E, 0x4C, 0x75, 0xBA, 0x94, 0x31, 0x48, 0xF1, 0x8D, 0x94, 0x1E};
static uint8_t service1_chars2[]  = {0x71, 0x3D, 0, 3, 0x50, 0x3E, 0x4C, 0x75, 0xBA, 0x94, 0x31, 0x48, 0xF1, 0x8D, 0x94, 0x1E};
static uint8_t service1_chars3[]  = {0x71, 0x3D, 0, 4, 0x50, 0x3E, 0x4C, 0x75, 0xBA, 0x94, 0x31, 0x48, 0xF1, 0x8D, 0x94, 0x1E};

UUID service_uuid(0x180D);
UUID chars_uuid1(0x2A37);
UUID chars_uuid2(service1_chars2);
UUID chars_uuid3(service1_chars3);

static uint8_t device_is_hrm = 0;

static uint8_t characteristic_is_found = 0;
static uint8_t descriptor_is_found = 0;
static DiscoveredCharacteristic            chars_hrm;
static DiscoveredCharacteristicDescriptor  desc_of_chars_hrm(NULL, GattAttribute::INVALID_HANDLE, GattAttribute::INVALID_HANDLE, UUID::ShortUUIDBytes_t(0));

static void scanCallBack(const Gap::AdvertisementCallbackParams_t *params);
static void discoveredServiceCallBack(const DiscoveredService *service);
static void discoveredCharacteristicCallBack(const DiscoveredCharacteristic *chars);
static void discoveryTerminationCallBack(Gap::Handle_t connectionHandle);
static void discoveredCharsDescriptorCallBack(const CharacteristicDescriptorDiscovery::DiscoveryCallbackParams_t *params);
static void discoveredDescTerminationCallBack(const CharacteristicDescriptorDiscovery::TerminationCallbackParams_t *params) ;

uint32_t ble_advdata_parser(uint8_t type, uint8_t advdata_len, uint8_t *p_advdata, uint8_t *len, uint8_t *p_field_data) {
  uint8_t index = 0;
  uint8_t field_length, field_type;

  while (index < advdata_len) {
    field_length = p_advdata[index];
    field_type   = p_advdata[index + 1];
    if (field_type == type) {
      memcpy(p_field_data, &p_advdata[index + 2], (field_length - 1));
      *len = field_length - 1;
      return NRF_SUCCESS;
    }
    index += field_length + 1;
  }
  return NRF_ERROR_NOT_FOUND;
}

void startDiscovery(uint16_t handle) {
  if (device_is_hrm)
    ble.gattClient().launchServiceDiscovery(handle, discoveredServiceCallBack, discoveredCharacteristicCallBack, service_uuid, chars_uuid1);
}

static void scanCallBack(const Gap::AdvertisementCallbackParams_t *params) {
  uint8_t index;

  Serial.println("Scan CallBack ");
  Serial.print("PeerAddress: ");
  for (index = 0; index < 6; index++) {
    Serial.print(params->peerAddr[index], HEX);
    Serial.print(" ");
  }
  Serial.println(" ");

  uint8_t len;
  uint8_t adv_name[31];
  if ( NRF_SUCCESS == ble_advdata_parser(BLE_GAP_AD_TYPE_COMPLETE_LOCAL_NAME, params->advertisingDataLen, (uint8_t *)params->advertisingData, &len, adv_name) ) {
    if (memcmp("PHLTX", adv_name, 5) == 0x00) {
      Serial.println("Got device, stop scan ");
      ble.stopScan();
      device_is_hrm = 1;
      ble.connect(params->peerAddr, BLEProtocol::AddressType::RANDOM_STATIC, NULL, NULL);
    }
  }
  Serial.println(" ");
}

void connectionCallBack( const Gap::ConnectionCallbackParams_t *params ) {
  startDiscovery(params->handle);
}

void disconnectionCallBack(const Gap::DisconnectionCallbackParams_t *params) {
  Serial.println("Disconnected, start to scanning");
  device_is_hrm = 0;
  characteristic_is_found = 0;
  descriptor_is_found = 0;
  ble.startScan(scanCallBack);
}

static void discoveredServiceCallBack(const DiscoveredService *service) {
}

static void discoveredCharacteristicCallBack(const DiscoveredCharacteristic *chars) {
  if ((chars->getUUID().shortOrLong() == UUID::UUID_TYPE_SHORT) && (chars->getUUID().getShortUUID() == 0x2A37)) {
    characteristic_is_found = 1;
    chars_hrm = *chars;
  }
}

static void discoveryTerminationCallBack(Gap::Handle_t connectionHandle) {
  if (characteristic_is_found == 1) {
    ble.gattClient().discoverCharacteristicDescriptors(chars_hrm, discoveredCharsDescriptorCallBack, discoveredDescTerminationCallBack);
  }
}

static void discoveredCharsDescriptorCallBack(const CharacteristicDescriptorDiscovery::DiscoveryCallbackParams_t *params) {
  if (params->descriptor.getUUID().getShortUUID() == 0x2902) {
    descriptor_is_found = 1;
    desc_of_chars_hrm = params->descriptor;
  }
}

static void discoveredDescTerminationCallBack(const CharacteristicDescriptorDiscovery::TerminationCallbackParams_t *params) {
  if (descriptor_is_found) {
    Serial.println("Open PHLTX notify");
    uint16_t value = 0x0001;
    ble.gattClient().write(GattClient::GATT_OP_WRITE_REQ, chars_hrm.getConnectionHandle(), desc_of_chars_hrm.getAttributeHandle(), 2, (uint8_t *)&value);
    tlcTest();
  }
}

boolean nextPwmState;
uint8_t delays[20] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
uint8_t *state = delays;
unsigned long previousMillis = 0;

void hvxCallBack(const GattHVXCallbackParams *params) {
  Serial.println("GattClient notify call back ");
  for (unsigned char index = 0; index < params->len; index++) {
    Serial.print(params->data[index], DEC);
    Serial.print(" ");
  }
  Serial.println(" ");

  for (int i = 0; i < 20; i++)
    if (i < params->len)
      delays[i] = params->data[i];
    else
      delays[i] = 0;

  Serial.println("new delays array:");
  for (int i = 0; i < 20; i++) {
    Serial.print(delays[i]);
    Serial.print(" ");
  }
  Serial.println(" ");

  nextPwmState = 1;
  state = delays; // point to beginning

  switchTlcs();

  previousMillis = millis();
  Serial.print("Next wait time: ");
  Serial.println(*state);
  Serial.print("Previous Millis:");
  Serial.println(previousMillis);
}

void switchTlcs() {
  Serial.print("Set TLCs to ");
  uint16_t a = nextPwmState * 0xFFFF;
  uint16_t b = nextPwmState * 0xFF;
  tlc.setPWM(0, a);
  tlc.setPWM(1, a);
  tlc.setPWM(2, a);
  tlc.setPWM(3, b);
  tlc.setPWM(12, a);
  tlc.setPWM(13, a);
  tlc.setPWM(14, a);
  tlc.setPWM(15, b);
  tlc.write();
  nextPwmState = !nextPwmState;
  Serial.println("Setting TLCs done");
}

int tickerLed = 0;

void tickerTask() {
  //tlc.setPWM(4, 0xFF * tickerLed);
  //tlc.setPWM(16, 0xFF * tickerLed);
  //tlc.write();
  if (tickerLed)
    digitalWrite(LED_BUILTIN, HIGH);
  else
    digitalWrite(LED_BUILTIN, LOW);
  tickerLed = !tickerLed;
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  Serial.println("BLE PHL Receiver ");

  ticker.attach(tickerTask, 1);

  pinMode(dataPin, OUTPUT);
  pinMode(clockPin, OUTPUT);

  tlc.begin();
  tlc.write();

  // initialize digital pin LED_BUILTIN as an output.
  pinMode(LED_BUILTIN, OUTPUT);

  ble.init();
  ble.onConnection(connectionCallBack);
  ble.onDisconnection(disconnectionCallBack);
  ble.gattClient().onServiceDiscoveryTermination(discoveryTerminationCallBack);
  ble.gattClient().onHVX(hvxCallBack);
  ble.setScanParams(1000, 200, 0, false);
  ble.startScan(scanCallBack);
}

void tlcTest() {
  Serial.println("On");
  tlc.setPWM(0, 0xFFFF);
  tlc.setPWM(3, 0xFF);
  tlc.setPWM(12, 0xFFFF);
  tlc.setPWM(15, 0xFF);
  tlc.write();
  delay(1000);
  Serial.println("Off");
  tlc.setPWM(0, 0);
  tlc.setPWM(3, 0);
  tlc.setPWM(12, 0);
  tlc.setPWM(15, 0);
  tlc.write();
  delay(1000);
}

void loop() {
  // check delays array and do something if necessary
  if (*state != 0) {
    unsigned long currentMillis = millis();
    if (currentMillis - previousMillis >= *state * 5) {
      Serial.print("Passed wait time: ");
      Serial.println(*state);
      switchTlcs();
      state++;
      if (*state == 0) {
        Serial.println("Done");
      } else {
        Serial.print("Next wait time: ");
        Serial.println(*state);
      }
      previousMillis = currentMillis;
    }
  }

  //ble.waitForEvent();
}

