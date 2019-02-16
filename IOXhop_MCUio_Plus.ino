/* File              : IOXhop_MCUio_Plus.cpp
   Codeing By IOXhop : www.ioxhop.com
   Sonthaya Nongnuch : www.fb.me/maxthai */

#include <Wire.h>
#include <Servo.h>
#include <avr/wdt.h>

void(*resetFunc) (void) = 0;

byte devAddr = 0x08;
byte RequestAddr = 0x0;

int pwm_to_pin[] = { 3, 5, 6, 9, 10, 11 };
int servo_to_pin[] = { 2, 4, 7, 8, 12, 13, A0, A1, A2, A3 };
byte buff_registers[62];

Servo myServo[10];

void setup() {
  Wire.begin(devAddr);
  Wire.onReceive([](int numByte) {
    if (Wire.available() == 1) {
      RequestAddr = Wire.read();
      return;
    }

    while (Wire.available() > 1) {
      byte addr = Wire.read();
      byte value = Wire.read();
      if (addr >= 1 && addr <= 20) {
        pinMode(addr - 1, (value & 0x01) == 0x01 ? OUTPUT : INPUT);
      } else if (addr >= 21 && addr <= 40) {
        digitalWrite(addr - 21, (value & 0x01) == 0x01 ? HIGH : LOW);
      } else if (addr >= 53 && addr <= 58) {
        analogWrite(pwm_to_pin[addr - 53], (int)value);
      } else if (addr >= 59 && addr <= 61) {
        buff_registers[addr] = value;
        if (addr == 61) {
          if ((value & 0x10) != 0) {
            tone(value & 0x0F, buff_registers[59] << 8 | buff_registers[60]);
          } else {
            noTone(value & 0x0F);
          }
        }
      } else if (addr >= 62 && addr <= 71) {
        Servo &servo = myServo[addr - 62];
        if (!servo.attached()) {
          servo.attach(servo_to_pin[addr - 62]);
        }
        servo.write(value);
      } else if (addr == 0xFF) {
        wdt_disable();
        wdt_enable(WDTO_15MS);
        while(1) ;
      }
    }
  });
  Wire.onRequest([]() {
    byte addr = RequestAddr;
    if (addr >= 21 && addr <= 40) {
      Wire.write(digitalRead(addr - 21) == HIGH ? 0x01 : 0x0);
    } else if (addr >= 41 && addr <= 52) {
      if ((addr % 2) == 1) {
        int tmp_ADC = analogRead((addr - 41) / 2 + 14);
        Wire.write(tmp_ADC & 0xFF);
        Wire.write((tmp_ADC >> 8) & 0x03);
      }
    } else {
      Wire.write(0x0);
    }
  });
}

void loop() {
  delay(500);
}
