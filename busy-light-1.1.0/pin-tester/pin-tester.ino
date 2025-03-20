#define GREEN_BUTTON D5
#define GREEN_BUTTON_LIGHT D7
#define GREEN_LIGHT D8

#define YELLOW_BUTTON D4
#define YELLOW_BUTTON_LIGHT D0
#define YELLOW_LIGHT D6

#define RED_BUTTON D1
#define RED_BUTTON_LIGHT D2
#define RED_LIGHT D3

int counter = 0;
int lightCount = 0;

void setup() {
  Serial.begin(9600);
  Serial.println("==============");
  Serial.println("===NEW BOOT===");
  Serial.println("==============");

  pinMode(GREEN_BUTTON_LIGHT, OUTPUT);
  digitalWrite(GREEN_BUTTON_LIGHT, LOW);

  pinMode(GREEN_LIGHT, OUTPUT);
  digitalWrite(GREEN_LIGHT, LOW);

  pinMode(YELLOW_BUTTON_LIGHT, OUTPUT);
  digitalWrite(YELLOW_BUTTON_LIGHT, LOW);

  pinMode(YELLOW_LIGHT, OUTPUT);
  digitalWrite(YELLOW_LIGHT, LOW);

  pinMode(RED_BUTTON_LIGHT, OUTPUT);
  digitalWrite(RED_BUTTON_LIGHT, LOW);

  pinMode(RED_LIGHT, OUTPUT);
  digitalWrite(RED_LIGHT, LOW);

  pinMode(GREEN_BUTTON, INPUT_PULLUP);
  pinMode(YELLOW_BUTTON, INPUT_PULLUP);
  pinMode(RED_BUTTON, INPUT_PULLUP);
}

void loop() {
  if (!digitalRead(GREEN_BUTTON))
    Serial.println("GREEN_BUTTON");
  if (!digitalRead(YELLOW_BUTTON))
    Serial.println("YELLOW_BUTTON");
  if (!digitalRead(RED_BUTTON))
    Serial.println("RED_BUTTON");
  
  if (counter > 6) {
    lightCount++;
    if (lightCount > 6)
      lightCount = 0;
    digitalWrite(GREEN_LIGHT, lightCount == 1);
    digitalWrite(YELLOW_LIGHT, lightCount == 2);
    digitalWrite(RED_LIGHT, lightCount == 3);
    digitalWrite(GREEN_BUTTON_LIGHT, lightCount == 4);
    digitalWrite(YELLOW_BUTTON_LIGHT, lightCount == 5);
    digitalWrite(RED_BUTTON_LIGHT, lightCount == 6);
    counter = 0;
  } else {
    counter += 1;
  }

  delay(50);
}