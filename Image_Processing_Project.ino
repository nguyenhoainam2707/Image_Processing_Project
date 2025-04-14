#include <Servo.h>
#include <math.h>

#define S1_EN_PIN A1
#define S1_STEP_PIN 2
#define S1_DIR_PIN 3

#define S2_EN_PIN 6
#define S2_STEP_PIN 5
#define S2_DIR_PIN 4

#define S3_EN_PIN 9
#define S3_STEP_PIN 8
#define S3_DIR_PIN 7

#define SERVO_PIN 10

#define S1_STOP_PIN 11
#define S2_STOP_PIN 12
#define S3_STOP_PIN 13

#define a 54.12
#define b 140.00
#define c 140.00
#define d 21.70

Servo myservo;

bool s1_stop = true;
float s1_pos = -2;
float s1_angel = 0;
uint16_t s1_num_steps = 0;
// uint16_t s1_delay_us = 500;

bool s2_stop = true;
float s2_pos = 135;
float s2_angel = 0;
uint16_t s2_num_steps = 0;
// uint16_t s2_delay_us = 1000;

bool s3_stop = true;
float s3_pos = 0;
float s3_angel = 0;
uint16_t s3_num_steps = 0;
uint16_t s3_delay_us = 250;

bool color = false;
float x, y, z;
float pre_alpha = 67;

void go_home() {
  digitalWrite(S1_EN_PIN, LOW);
  digitalWrite(S2_EN_PIN, LOW);
  digitalWrite(S3_EN_PIN, LOW);
  digitalWrite(S1_DIR_PIN, HIGH);
  digitalWrite(S2_DIR_PIN, HIGH);
  digitalWrite(S3_DIR_PIN, HIGH);
  while (s1_stop || s2_stop || s3_stop) {
    s1_stop = digitalRead(S1_STOP_PIN);
    s2_stop = digitalRead(S2_STOP_PIN);
    s3_stop = digitalRead(S3_STOP_PIN);

    if (s1_stop) digitalWrite(S1_STEP_PIN, HIGH);
    if (s2_stop) digitalWrite(S2_STEP_PIN, HIGH);
    if (s3_stop) digitalWrite(S3_STEP_PIN, HIGH);
    delayMicroseconds(s3_delay_us);
    if (s3_stop) digitalWrite(S3_STEP_PIN, LOW);
    delayMicroseconds(s3_delay_us);

    if (s1_stop) digitalWrite(S1_STEP_PIN, LOW);
    if (s2_stop) digitalWrite(S2_STEP_PIN, LOW);
    if (s3_stop) digitalWrite(S3_STEP_PIN, HIGH);
    delayMicroseconds(s3_delay_us);
    if (s3_stop) digitalWrite(S3_STEP_PIN, LOW);
    delayMicroseconds(s3_delay_us);
  }
  myservo.write(180);
  s1_stop = true;
  s2_stop = true;
  s3_stop = true;
  s1_pos = -2;
  s2_pos = 135;
  s3_pos = 0;
  Serial.println("Go home: done!");
}

void go_to_pos(float x, float y, float z, uint8_t servo_angel) {
  //Tính động học ngược
  float k = pow((sqrt(x * x + y * y) - a), 2) + pow((z + d), 2);
  float phi = degrees(atan((z + d) / (sqrt(x * x + y * y) - a)) + acos((c * c + k - b * b) / (2 * c * sqrt(k))));
  float alpha = degrees(acos((c * c + b * b - k) / (2 * c * b)));
  s1_angel = 90 + degrees(atan2(x, y)) - s1_pos;
  s1_pos = 90 + degrees(atan2(x, y));
  s2_angel = phi - s2_pos;
  s2_pos = phi;
  s3_angel = pre_alpha - alpha - s2_angel;
  s3_pos += s3_angel;
  pre_alpha = alpha;
  //Di chuyển tới vị trí
  if (s1_angel < 0) digitalWrite(S1_DIR_PIN, HIGH);
  else digitalWrite(S1_DIR_PIN, LOW);
  if (s2_angel < 0) digitalWrite(S2_DIR_PIN, LOW);
  else digitalWrite(S2_DIR_PIN, HIGH);
  if (s3_angel < 0) digitalWrite(S3_DIR_PIN, HIGH);
  else digitalWrite(S3_DIR_PIN, LOW);
  s1_num_steps = round(abs(s1_angel) * 40);
  s2_num_steps = round(abs(s2_angel) * 40);
  s3_num_steps = round(abs(s3_angel) * 80);

  while (s1_num_steps || s2_num_steps || s3_num_steps) {
    if (s1_num_steps) digitalWrite(S1_STEP_PIN, HIGH);
    if (s2_num_steps) digitalWrite(S2_STEP_PIN, HIGH);
    if (s3_num_steps) digitalWrite(S3_STEP_PIN, HIGH);
    delayMicroseconds(s3_delay_us);
    if (s3_num_steps) digitalWrite(S3_STEP_PIN, LOW);
    delayMicroseconds(s3_delay_us);
    if (s3_num_steps) s3_num_steps--;

    if (s1_num_steps) digitalWrite(S1_STEP_PIN, LOW);
    if (s2_num_steps) digitalWrite(S2_STEP_PIN, LOW);
    if (s3_num_steps) digitalWrite(S3_STEP_PIN, HIGH);
    delayMicroseconds(s3_delay_us);
    if (s3_num_steps) digitalWrite(S3_STEP_PIN, LOW);
    delayMicroseconds(s3_delay_us);

    if (s1_num_steps) s1_num_steps--;
    if (s2_num_steps) s2_num_steps--;
    if (s3_num_steps) s3_num_steps--;
  }
  myservo.write(servo_angel);
}

void pick_and_drop() {
  while (!Serial.available()) {}
  x = Serial.parseFloat();
  Serial.print("X: ");
  Serial.println(x);
  y = Serial.parseFloat();
  Serial.print("Y: ");
  Serial.println(y);
  z = Serial.parseFloat();
  Serial.print("Z: ");
  Serial.println(z);
  color = bool(Serial.parseFloat());
  if (color) Serial.println("Color: GREEN");
  else Serial.println("Color: YELLOW");
  Serial.readStringUntil('\n');

  Serial.println("X: " + String(x, 2) + ", Y: " + String(y, 2) + ", Z: " + String(0, 2));
  go_to_pos(x, y, 0, 180);
  Serial.println("s1_angel: " + String(s1_angel, 4));
  Serial.println("s2_angel: " + String(s2_angel, 4));
  Serial.println("s3_angel: " + String(s3_angel, 4));

  Serial.println("X: " + String(x, 2) + ", Y: " + String(y, 2) + ", Z: " + String(z, 2));
  go_to_pos(x, y, z, 95);
  Serial.println("s1_angel: " + String(s1_angel, 4));
  Serial.println("s2_angel: " + String(s2_angel, 4));
  Serial.println("s3_angel: " + String(s3_angel, 4));

  delay(1000);

  Serial.println("X: " + String(x, 2) + ", Y: " + String(y, 2) + ", Z: " + String(0, 2));
  go_to_pos(x, y, 0, 95);
  Serial.println("s1_angel: " + String(s1_angel, 4));
  Serial.println("s2_angel: " + String(s2_angel, 4));
  Serial.println("s3_angel: " + String(s3_angel, 4));

  if (color) {
    Serial.println("X: 288, Y: -60, Z: -50");
    go_to_pos(288, -60, -50, 180);
    Serial.println("s1_angel: " + String(s1_angel, 4));
    Serial.println("s2_angel: " + String(s2_angel, 4));
    Serial.println("s3_angel: " + String(s3_angel, 4));
  } else {
    Serial.println("X: 180, Y: -60, Z: -50");
    go_to_pos(180, -60, -50, 180);
    Serial.println("s1_angel: " + String(s1_angel, 4));
    Serial.println("s2_angel: " + String(s2_angel, 4));
    Serial.println("s3_angel: " + String(s3_angel, 4));
  }

  Serial.println("Done!");
}

void draw_heart(int num_point, float heart_size, uint8_t servo_angel, float x_origin, float y_origin, float z_origin) {
  go_to_pos(x_origin, y_origin, -50, servo_angel);
  s3_delay_us = 600;
  double delta = 2 * PI / num_point;
  for (float i = 0.0; i <= PI; i += delta) {
    x = heart_size * 16 * pow(sin(i), 3) + x_origin;
    y = heart_size * (13 * cos(i) - 5 * cos(2 * i) - 2 * cos(3 * i) - cos(4 * i)) + y_origin;
    z = z_origin;
    go_to_pos(x, y, z, servo_angel);
    // Serial.println("X: "+String(x)+" ,Y: "+String(y)+" ,Z: "+String(z));
  }
  go_to_pos(x_origin, y_origin, 0, servo_angel);
  for (float i = 0.0; i >= -PI; i -= delta) {
    x = heart_size * 16 * pow(sin(i), 3) + x_origin;
    y = heart_size * (13 * cos(i) - 5 * cos(2 * i) - 2 * cos(3 * i) - cos(4 * i)) + y_origin;
    z = z_origin;
    go_to_pos(x, y, z, servo_angel);
    // Serial.println("X: "+String(x)+" ,Y: "+String(y)+" ,Z: "+String(z));
  }
  Serial.println("Draw heart: Done!");
  s3_delay_us = 250;
  go_to_pos(x, y, 100, servo_angel);
  go_home();
}

void draw_line(int num_point, uint8_t servo_angel, float x1, float y1, float z1, float x2, float y2, float z2) {
  // go_to_pos(x1, y1, 0, servo_angel);
  s3_delay_us = 500;
  float line_long = sqrt((x2 - x1) * (x2 - x1) + (y2 - y1) * (y2 - y1) + (z2 - z1) * (z2 - z1));
  float delta = line_long / num_point;
  for (int i = 0; i < num_point; i++) {
    x = x1 + i * (x2 - x1) / (num_point - 1);
    y = y1 + i * (y2 - y1) / (num_point - 1);
    z = z1 + i * (z2 - z1) / (num_point - 1);
    go_to_pos(x, y, z, servo_angel);
    // Serial.println("X: "+String(x)+" ,Y: "+String(y)+" ,Z: "+String(z));
  }
  Serial.println("Draw line: Done!");
  s3_delay_us = 250;
  // go_to_pos(x, y, 0, servo_angel);
  go_home();
}

void setup() {
  Serial.begin(9600);

  pinMode(S1_EN_PIN, OUTPUT);
  pinMode(S1_STEP_PIN, OUTPUT);
  pinMode(S1_DIR_PIN, OUTPUT);
  pinMode(S2_EN_PIN, OUTPUT);
  pinMode(S2_STEP_PIN, OUTPUT);
  pinMode(S2_DIR_PIN, OUTPUT);
  pinMode(S3_EN_PIN, OUTPUT);
  pinMode(S3_STEP_PIN, OUTPUT);
  pinMode(S3_DIR_PIN, OUTPUT);
  pinMode(S1_STOP_PIN, INPUT_PULLUP);
  pinMode(S2_STOP_PIN, INPUT_PULLUP);
  pinMode(S3_STOP_PIN, INPUT_PULLUP);

  myservo.attach(SERVO_PIN, 550, 2420);
  myservo.write(180);
  go_home();
  delay(1000);
  myservo.write(115);
  delay(1500);
}

void loop() {
  // pick_and_drop();
  draw_heart(1000, 2, 100, 0, 230, -95);
  // draw_line(3000, 115, 250, 50, -90, 0, 200, 50);
}














//
