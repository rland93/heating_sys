#define TEMP_SENSOR_PIN A0
#define CAL_INDICATOR_PIN 9
#define TCS_OP_PIN 8
#define HEAT_IND_PIN 10
#define HEAT_CTL_PIN 4


int initial_delay = 8000;
float temp = 0;
float Rsensor = 0.0;

// Heat system
bool heat_on;
const int DUTY_CYC_INTERVAL = 500;
// on at least this proportion of the time
const float CYC_PERIOD_MIN = 0.2;
// Temperature set point
const float SET_POINT = 4.0;

const float TEMP_SCALE_FACTOR = 5.0;


// Op Light
bool op_light = true;

const unsigned int nsamples = 200;
// Kelvin temperature of 0 deg C
const float KELVIN2C = 273.15;
// from 02-N101-1 Datasheet
const int B = 3100;
// deg K
const float T0 = 298.15;
// heater resistance at T0
const float R0 = 130;
//  upper bridge resistance
const float R2 = 330;
// temp calibration
const int CAL_SAMPLES = 10000;
float cal;
const float TCS_SCALE = 1.358;


// calculate temperature from resistance
float calcT(float B, float R) {
  const float logR = log(R0/float(R));
  return -(T0 * B / (T0 * logR + B) - KELVIN2C);
};

float pin2voltage(int pinval) {
  float dV = float(pinval)/1024.0;
  return R2 * (dV/(1-dV));
};


void setup() {
  delay(initial_delay);
  Serial.begin(9600);
  Serial.println("time, T, dT, Calibration Temperature, Duty Factor, Duty Cycle, Pulse Width, Set Point");
  // TCS Op Light
  pinMode(TCS_OP_PIN, OUTPUT);
  // Heat On Light
  pinMode(HEAT_IND_PIN, OUTPUT);
  pinMode(HEAT_CTL_PIN, OUTPUT);
  // Calibration Light
  pinMode(CAL_INDICATOR_PIN, OUTPUT);
  digitalWrite(CAL_INDICATOR_PIN, HIGH);
  analogReference(EXTERNAL);
  cal = 0;
  for (unsigned int j=0; j < CAL_SAMPLES; j++) {
    cal += calcT(B, pin2voltage(analogRead(TEMP_SENSOR_PIN)));
  }
  cal /= CAL_SAMPLES;
  //Finished Calibration
  digitalWrite(CAL_INDICATOR_PIN, LOW);
  digitalWrite(TCS_OP_PIN, HIGH);
}

void loop() {
  digitalWrite(HEAT_CTL_PIN, LOW);
  delay(20);
  
  // Measure Temperature  
  // Op Light
  digitalWrite(TCS_OP_PIN, HIGH);
  temp = 0;
  for (unsigned int i=0; i < nsamples; i++) {
      Rsensor = pin2voltage(analogRead(TEMP_SENSOR_PIN));
      temp += calcT(B, Rsensor);
  }
  digitalWrite(TCS_OP_PIN, LOW);
  temp /= nsamples;
  temp = temp- cal;



  // here we have a simple PID loop where we duty cycle
  // the heater. it's fine the transients of TIP120 are
  // _very_ fast compared to this loop.

  // on period is a float in interval [0,1]
  float on_period = (SET_POINT - temp*TEMP_SCALE_FACTOR) / SET_POINT;
  // cannot have an on period longer than duty cycle
  // or shorter than minimum...
  on_period = min(max(CYC_PERIOD_MIN, on_period), 1.0);
  // cast back to int. now this value is millis
  // as a function of duty cycle that the
  // heater will be on.
  int cyc_period = static_cast<int>(on_period);

  // if we are close to set point, instigate P-control
  // with duty cycling.

  if ((SET_POINT - temp*TEMP_SCALE_FACTOR) < 4.0) {
    int dt=0;
    int t1=millis();
    while (dt < DUTY_CYC_INTERVAL) {
      dt = millis()-t1;

      
      if (dt>on_period*DUTY_CYC_INTERVAL) {
        heat_on = false;
        digitalWrite(HEAT_IND_PIN, LOW);
        digitalWrite(HEAT_CTL_PIN, LOW);
      }
      else {
        heat_on = true;
        digitalWrite(HEAT_CTL_PIN, HIGH);
        digitalWrite(HEAT_IND_PIN, HIGH);

      }
    }
  }

  // otherwise, full on
  else {
    int dt = 0;
    int t1 = millis();
    while (dt < DUTY_CYC_INTERVAL){
      digitalWrite(HEAT_IND_PIN,HIGH);
      digitalWrite(HEAT_CTL_PIN,HIGH);
      dt = millis() - t1;
    }
    
  }
  
  Serial.print(millis());
  Serial.print(", ");
  Serial.print(temp);
  Serial.print(", ");
  Serial.print(SET_POINT - temp);
  Serial.print(", ");
  Serial.print(cal);
  Serial.print(", ");
  Serial.print(on_period);
  Serial.print(", ");
  Serial.print(on_period * DUTY_CYC_INTERVAL);
  Serial.print(", ");
  Serial.print(DUTY_CYC_INTERVAL);
  Serial.print(", ");
  Serial.print(SET_POINT);
  Serial.print("\n");
}
