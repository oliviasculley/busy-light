#include <espnow.h>
#include <ESP8266WiFi.h>

#define GREEN_BUTTON D5
#define GREEN_BUTTON_LIGHT D7
#define GREEN_LIGHT D8

#define YELLOW_BUTTON D4
#define YELLOW_BUTTON_LIGHT D0
#define YELLOW_LIGHT D6

#define RED_BUTTON D1
#define RED_BUTTON_LIGHT D2
#define RED_LIGHT D3

const uint8_t BUTTON_LIGHTS[] = {
  RED_BUTTON_LIGHT,
  YELLOW_BUTTON_LIGHT,
  GREEN_BUTTON_LIGHT
};
const uint8_t STATUS_LIGHTS[] = { RED_LIGHT, YELLOW_LIGHT, GREEN_LIGHT };
const uint8_t BUTTONS[] = { RED_BUTTON, YELLOW_BUTTON, GREEN_BUTTON };

enum Status { NONE, NO_CALL, CAUTION, AUDIO_CALL, VIDEO_CALL };
Status selfStatus = NONE;
Status otherStatus = NONE;

// Toggle me to flash corresponding boxes
#if true
uint8_t BROADCAST_ADDRESS[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
#else
uint8_t BROADCAST_ADDRESS[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
#endif

unsigned char* ESPNOW_KEY = (unsigned char*) "your-16-char-encryption-key-here";
struct Broadcast { uint8_t status; bool confirm; };
Broadcast sendingStatus;
Broadcast sendingConfirmation;
bool receive = true;
int rebroadcastCounter = 0;
#define REBROADCAST_INTERVAL 100
#define HOLD_THRESHOLD 10
int resetHeldCounter = 0;
int heldCounter = 0;

void setup() {
  Serial.begin(9600);
  Serial.println("==============");
  Serial.println("===NEW BOOT===");
  Serial.println("==============");

  for(uint8_t buttonLight : BUTTON_LIGHTS) {
    pinMode(buttonLight, OUTPUT);
    digitalWrite(buttonLight, LOW);
  }
  for(uint8_t statusLight : STATUS_LIGHTS) {
    pinMode(statusLight, OUTPUT);
    digitalWrite(statusLight, LOW);
  }
  for(uint8_t button : BUTTONS) {
    pinMode(button, INPUT_PULLUP);
  }

  sendingStatus.status = NONE;
  sendingStatus.confirm = false;

  sendingConfirmation.status = NONE;
  sendingConfirmation.confirm = true;

  WiFi.mode(WIFI_STA);
  Serial.print("ESP Board MAC Address:  ");
  Serial.println(WiFi.macAddress());

  if (esp_now_init() != 0) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  esp_now_set_self_role(ESP_NOW_ROLE_COMBO);
  esp_now_add_peer(
    BROADCAST_ADDRESS,
    ESP_NOW_ROLE_COMBO,
    1,
    ESPNOW_KEY,
    sizeof(ESPNOW_KEY)
  );
  esp_now_register_recv_cb(esp_now_recv_cb_t(readStatus));
}

void loop() {
  bool greenPressed = !digitalRead(GREEN_BUTTON);
  bool yellowPressed = !digitalRead(YELLOW_BUTTON);
  bool redPressed = !digitalRead(RED_BUTTON);
  
  if (resetHeldCounter > HOLD_THRESHOLD && greenPressed)
    sendingStatus.status = NONE;
  else if (greenPressed) {
    sendingStatus.status = NO_CALL;
    resetHeldCounter++;
  } else
    resetHeldCounter = 0;

  if (yellowPressed)
    sendingStatus.status = CAUTION;

  if (heldCounter > HOLD_THRESHOLD && redPressed) {
    sendingStatus.status = VIDEO_CALL;
  } else if (redPressed) {
    heldCounter++;
    sendingStatus.status = AUDIO_CALL;
  } else
    heldCounter = 0;

  sendSelfStatus();
  delay(50);
}

void sendSelfStatus() {
  rebroadcastCounter++;
  if (rebroadcastCounter < REBROADCAST_INTERVAL && sendingStatus.status == selfStatus) return;
  rebroadcastCounter = 0;

  Serial.print("Sending self status ");
  Serial.println(getStatusStr((Status) sendingStatus.status));

  esp_now_send(
    BROADCAST_ADDRESS,
    (uint8_t *) &sendingStatus,
    sizeof(sendingStatus)
  );
}

void readStatus(uint8_t * mac, uint8_t * incomingData, uint8_t len) {
  if (((Broadcast*) incomingData)->confirm) {
    selfStatus = (Status) ((Broadcast*) incomingData)->status;
    Serial.print("Set self status ");
    Serial.println(getStatusStr(selfStatus));
    setButtonLights(selfStatus);
  } else {
    sendingConfirmation.status = ((Broadcast*) incomingData)->status;
    Serial.print("Sending other status ");
    Serial.println(getStatusStr((Status) sendingConfirmation.status));

    uint8_t result = esp_now_send(
      BROADCAST_ADDRESS,
      (uint8_t *) &sendingConfirmation,
      sizeof(sendingConfirmation)
    );
    if (result != 0) return;

    otherStatus = (Status) ((Broadcast*) incomingData)->status;
    
    Serial.print("Set other status ");
    Serial.println(getStatusStr(otherStatus));
    setStatusLights(otherStatus);
  }
}

// helpers

const char* getStatusStr(Status status) {
  switch (status) {
    case NO_CALL:
      return "No Call";
    case CAUTION:
      return "Caution";
    case AUDIO_CALL:
      return "Audio Call";
    case VIDEO_CALL:
      return "Video Call";
    default:
      return "None";
  }
}

void setButtonLights(Status status) {
  for (uint8_t light : BUTTON_LIGHTS)
    digitalWrite(light, LOW);

  switch (status) {
    case NO_CALL:
      digitalWrite(GREEN_BUTTON_LIGHT, HIGH);
      break;
    case CAUTION:
      digitalWrite(YELLOW_BUTTON_LIGHT, HIGH);
      break;
    case AUDIO_CALL:
      digitalWrite(RED_BUTTON_LIGHT, HIGH);
      break;
    case VIDEO_CALL:
      digitalWrite(YELLOW_BUTTON_LIGHT, HIGH);
      digitalWrite(RED_BUTTON_LIGHT, HIGH);
      break;
    default:
      break;
  }
}

void setStatusLights(Status status) {
  for (uint8_t light : STATUS_LIGHTS)
    digitalWrite(light, LOW);

  switch (status) {
    case NO_CALL:
      digitalWrite(GREEN_LIGHT, HIGH);
      break;
    case CAUTION:
      digitalWrite(YELLOW_LIGHT, HIGH);
      break;
    case AUDIO_CALL:
      digitalWrite(RED_LIGHT, HIGH);
      break;
    case VIDEO_CALL:
      digitalWrite(YELLOW_LIGHT, HIGH);
      digitalWrite(RED_LIGHT, HIGH);
      break;
    default:
      break;
  }
}
