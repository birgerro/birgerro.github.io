#include <Servo.h>

const int UPDATE_INTERVAL = 60; // ms, multiple of 20ms
const int VALUE_MAX = 1000;
const int MIN_RANGE = 900;   // in microseconds
const int MAX_RANGE = 2000;  // in microseconds

int gauge_pins[] = { 9, 10, 11 };
const int NUM_GAUGES = sizeof gauge_pins / sizeof *gauge_pins;

float gaussian(float mean, float sdev) {
  float x1, x2, rad, c;
  
  do {
    x1 = 2 * random(1000)/1000.0 - 1;
    x2 = 2 * random(1000)/1000.0 - 1;
    rad = x1 * x1 + x2 * x2;
  } while ( rad >= 1 || rad == 0 );
  
  c = sqrt( -2 * log(rad) / rad );
  
  return mean + (x1 * c) * sdev;
}

class RandomGauge {
  public:
  RandomGauge();
  void set_pin(int pin);
  void set_range(int min, int max);
  void update();
  
  private:
  void move_needle(int value);
  
  Servo _servo;
  unsigned long _next_update;
  unsigned long _next_setpoint;
  unsigned int _setpoint;
  unsigned int _time_avg;  // feks 15000
  unsigned int _time_sdev; // feks  4000
  float _gain;
  int _value;
  int _noise_sdev;
  int _servo_min;
  int _servo_max;
};

RandomGauge::RandomGauge() {
  _next_update = 0;
  _next_setpoint = 0;
  _time_avg = 15000;
  _time_sdev = 4000;
  _noise_sdev = 5;
  _gain = 0.025;
  _value = VALUE_MAX / 2;
  _servo_min = 0;
  _servo_max = 180;
}

void RandomGauge::set_pin(int pin) {
  _servo.attach(pin);
}

void RandomGauge::set_range(int min, int max) {
  _servo_min = min;
  _servo_max = max;
}

void RandomGauge::update() {
  unsigned long time = millis();
  if ( time < _next_update ) {
    return;
  }
  if ( time > _next_setpoint ) {
    _setpoint = random(0, VALUE_MAX);
    _next_setpoint = time + gaussian(_time_avg, _time_sdev);
  }
  
  int error = _setpoint - _value;
  _value += round( _gain*error + gaussian(0, _noise_sdev) );
  move_needle(_value);
  _next_update = time + UPDATE_INTERVAL;
}

void RandomGauge::move_needle(int value) {
  value = constrain(value, 0, VALUE_MAX);
  int us = map(value, 0, VALUE_MAX, _servo_min, _servo_max);
  _servo.writeMicroseconds(us);
}

RandomGauge all_gauges[NUM_GAUGES];

void setup() {
  randomSeed(analogRead(A0));
  for ( int i=0; i<NUM_GAUGES; i++ ){
    all_gauges[i].set_pin(gauge_pins[i]);
    all_gauges[i].set_range(MIN_RANGE, MAX_RANGE);
  }
}

void loop() {
  for ( int i=0; i<NUM_GAUGES; i++ ){
    all_gauges[i].update();
  }
}

