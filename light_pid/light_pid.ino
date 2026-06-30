#include <PID_v1_bc.h>
#include <rgb_lcd.h>
#include <Wire.h>

/* ================= PINOS ================= */

#define LED_PWM 3
#define LDR_PIN A0

/* ================= LCD ================= */

rgb_lcd lcd;

/* ================= PID ================= */

/*
  Input  -> valor lido no LDR
  Output -> PWM aplicado no LED
  Setpoint -> luminosidade desejada
*/

double Input;
double Output;
double Setpoint;

/* Ganhos PID */

double Kp = 8.9;
double Ki = 11.8;
double Kd = 0.104;

/* Controlador PID */

PID controladorPID(
  &Input,
  &Output,
  &Setpoint,
  Kp,
  Ki,
  Kd,
  DIRECT
);

/* ================= SETUP ================= */

void setup() {

  /* ---------- Serial (para o Gráfico) ---------- */

  Serial.begin(9600);

  /* ---------- LED ---------- */

  pinMode(LED_PWM, OUTPUT);

  /* ---------- LDR ---------- */

  pinMode(LDR_PIN, INPUT);

  /* ---------- LCD ---------- */

  lcd.begin(16, 2);

  /* Cor do LCD (R, G, B) */

  lcd.setRGB(0, 255, 0);

  lcd.setCursor(0, 0);
  lcd.print("Sistema PID");

  lcd.setCursor(0, 1);
  lcd.print("Inicializando");

  delay(2000);

  lcd.clear();

  /* ---------- SETPOINT ---------- */

  Setpoint = 30.5;

  /* ---------- LIMITES PID ---------- */

  controladorPID.SetOutputLimits(0, 255);

  /* ---------- ATIVA PID ---------- */

  controladorPID.SetMode(AUTOMATIC);
}

/* ================= LOOP ================= */

void loop() {

  /* ---------- LEITURA LDR ---------- */

  Input = analogRead(LDR_PIN);
  Input = Input/4;

  /* ---------- PID ---------- */

  controladorPID.Compute();

  /* ---------- PWM LED ---------- */

  analogWrite(LED_PWM, (int)Output);

  /* ---------- LCD ---------- */

  lcd.setCursor(0, 0);
  lcd.print("L:");
  lcd.print((int)Input);
  lcd.print("    ");

  lcd.setCursor(0, 1);
  lcd.print("PWM:");
  lcd.print((int)Output);
  lcd.print("   ");

  /* ---------- Envio de dados para o Gráfico ---------- */
  Serial.print("Luminosidade:");
  Serial.print(Input);
  Serial.print(",");
  Serial.print("PWM:");
  Serial.print(Output);
  Serial.print(",");
  Serial.print("Setpoint:");
  Serial.println(Setpoint);

  delay(50);
}
