#include <Arduino.h>
#include "MIDIUSB.h"
#include "pitchToNote.h"

const float flowK = 10; // коэф дуновения к нотам (выбран мной рандомно, >0)
const uint16_t flowMiss = 10; // допустимая погрешность дуновения (выбран мной рандомно, настраивается для удобства, >0)
const uint16_t pitchChangeK = 0.1f; // коэф изменения ноты в зависимости от силы дуновения (выбран мной рандомно >0)
const float flowVelK = 0.124f; // коэф громкости в зависимости от дуновения (127/1023)

const uint8_t PIN_A = A0; // пин резистора / датчика
const uint8_t PIN_B1 = 2; // пины кнопок
const uint8_t PIN_B2 = 3;
const uint8_t PIN_B3 = 4;
// больше кнопок - больше объявлений

void noteOn(uint8_t channel, uint8_t pitch, uint8_t velocity) {
  midiEventPacket_t noteOn = {0x09, 0x90 | channel, pitch, velocity};
  MidiUSB.sendMIDI(noteOn);
}

void noteOff(uint8_t channel, uint8_t pitch, uint8_t velocity) {
  midiEventPacket_t noteOff = {0x08, 0x80 | channel, pitch, velocity};
  MidiUSB.sendMIDI(noteOff);
}

void setup() {
  // соединения кнопок следующее: pin - кнопка - GND, что намного удобнее, чем использовать дополнительные резисторы
  pinMode(PIN_B1, INPUT_PULLUP);
  pinMode(PIN_B2, INPUT_PULLUP);
  pinMode(PIN_B3, INPUT_PULLUP);
  // больше кнопок - больше строк
}

uint8_t pitchOld = 0;
uint8_t velOld = 0;

void loop() {
  uint16_t flow = analogRead(PIN_A); // считываем значение потенциометра

  uint16_t pitch = 0;
  uint8_t vel = 0;

  // тут математика нахождения основной ноты по кнопкам, скорее всего тут просто правдоподобная чушь, но чисто как mvp
  uint8_t noteCount = 0;
  if (!digitalRead(PIN_B1)) { // если нажата кнопка 1
    pitch = pitchC4; // from pitchToNote.h
    noteCount++;
  }
  if (!digitalRead(PIN_B2)) { // если нажата кнопка 2
    pitch = pitchD4; // from pitchToNote.h
    noteCount++;
  }
  if (!digitalRead(PIN_B3)) { // если нажата кнопка 3
    pitch = pitchE4; // from pitchToNote.h
    noteCount++;
  }
  // больше кнопок - больше тут if-ов
  pitch /= noteCount; // средне арифметическое, но по факту надо углубляться в то, как ноты образуются в зависимости от закрытия отверстий

  // тут математика примешивания аналогового значения в ноту с потенциометра / датчика давления воздуха, опять же правдоподобная чушь
  if ((pitch * flowK) > (flow + flowMiss)) { // если не попали силой воздуха в ноту
    uint16_t pitchChange = ((flow - flowMiss) / flowK) * pitchChangeK; // считаем на сколько надо поменять ноту
    pitch += pitchChangeK; // вычисляем новую ноту
    if (pitch > 108) // если больше максимума - максимум
      pitch = 108;
  }
  if ((pitch * flowK) < (flow - flowMiss)) { // если не попали силой воздуха в ноту
    uint16_t pitchChange = ((flow + flowMiss) / flowK) * pitchChangeK; // считаем на сколько надо поменять ноту
    pitch -= pitchChangeK; // вычисляем новую ноту
    if (pitch < 0) // если меньше минимума - минимум
      pitch = 0;
  }

  // громкость
  vel = (uint8_t)((float)flow * flowVelK); // громкость от силы дуновения

  // если значение изменилось - отсылаем новые данные на устройство
  if (pitchOld != pitch || velOld != vel) {
    noteOff(0, pitchOld, velOld); // выключаем старую ноту
    noteOn(0, pitch, vel); // включаем новую
    MidiUSB.flush(); // ждём отсылки
    pitchOld = pitch; // записываем новые значения в старые для след цикла
    velOld = vel;
  }
  delay(1);
}
