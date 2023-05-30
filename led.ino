
  /*        TinyML -  Arduino MKR Zero
   * (PA20) Pin7   -       GPIO6       - BUZZER
   * (PA21) Pin6   -       GPIO7       - PWM_RED/POW_EN
   * (PA22) Pin9   -       GPIO0       - PWM_BLUE
   * (PA23) Pin8   -       GPIO1       - PWM_GREEN
   */
#define BUZER_PIN 6 
#define POW_EN_PIN 7
#define PWM_BLUE_PIN 0
#define PWM_GREEN_PIN 1
#define SENSOR_PIN A0

// Enable buzzer. true - enabled, false - disabled
#define BUZZER_EN false
// Buzzer PWM duty. Range 0-255, sets PWM duty 0 = 0%, 255 = 100% duty
#define BUZZER_DUTY 127
// Buzz time in [ms]
#define BUZZ_TIME 500

// Blinking period in [ms]
#define BLINK_PERIOD 130

// Brightness values. Range 0-255, sets PWM duty 0 = 0%, 255 = 100% duty
#define BRIGHTNESS_0 20
#define BRIGHTNESS_1 60
#define BRIGHTNESS_2 90
#define BRIGHTNESS_3 250

// Light sensor levels. Range 0-1023. 0 = 0V. 1023 = VCC on the input pin
#define SENSOR_BRIGHTNESS_MODE_0 50  // if sensor value is less than this -> BRIGHTNESS_0 is used
#define SENSOR_BRIGHTNESS_MODE_1 500 // if sensor value is less than this and more than MODE_0 -> BRIGHTNESS_1 is used
#define SENSOR_BRIGHTNESS_MODE_2 900 // if sensor value is less than this and more than MODE_1 -> BRIGHTNESS_2 is used
                                     // if sensor value is greater than MODE_2 -> BRIGHTNESS_3 is used

typedef enum logoState_t{
  initialize,
  blinking,
};

bool sirenDetected = false;
bool logoEnabled = false;
unsigned int sensorLvl = 0;
unsigned int brightness = 0;
logoState_t state = initialize;
unsigned long long int buzzMillis = 0;
unsigned long long int blinkMillis = 0;

void ledLogo_init ()
{
  // Init outputs
  pinMode(BUZER_PIN, OUTPUT);
  pinMode(POW_EN_PIN, OUTPUT);
  pinMode(PWM_BLUE_PIN, OUTPUT);
  pinMode(PWM_GREEN_PIN, OUTPUT);

  // Set everything to 0
  analogWrite(BUZER_PIN, 0);
  digitalWrite(POW_EN_PIN, LOW);
  analogWrite(PWM_BLUE_PIN, 0);
  analogWrite(PWM_GREEN_PIN, 0);
}


void logoRoutine()
{
  if(sirenDetected)
  {
    if(state == initialize)
    {
      // Enable power
      digitalWrite(POW_EN_PIN, HIGH);
      delay(20);

      // Read lightsensor value
      sensorLvl = analogRead(SENSOR_PIN);
      Serial.print("Sensor value: ");
      Serial.println(sensorLvl);
      
      // Decide brightness based on sensor value
      if(sensorLvl < SENSOR_BRIGHTNESS_MODE_0)
      {
        brightness = BRIGHTNESS_0;
        Serial.println("Brightness mode 0");
      }
      else if(sensorLvl < SENSOR_BRIGHTNESS_MODE_1 && sensorLvl > SENSOR_BRIGHTNESS_MODE_0)
      {
        brightness = BRIGHTNESS_1;
        Serial.println("Brightness mode 1");
      }
      else if(sensorLvl < SENSOR_BRIGHTNESS_MODE_2 && sensorLvl > SENSOR_BRIGHTNESS_MODE_1)
      {
        brightness = BRIGHTNESS_2;
        Serial.println("Brightness mode 2");
      }
      else
      {
        brightness = BRIGHTNESS_3;
        Serial.println("Brightness mode 3");
      }

      #if BUZZER_EN
        buzzMillis = millis();
        analogWrite(BUZER_PIN, BUZZER_DUTY);
      #endif
      
      blinkMillis = millis();
      analogWrite(PWM_BLUE_PIN, brightness);
      analogWrite(PWM_GREEN_PIN, brightness);
      logoEnabled = true;
      
      state = blinking;
    }
    
    
    if(state == blinking)
    {
      #if BUZZER_EN
        if(millis() - buzzMillis > BUZZ_TIME)
        {
          analogWrite(BUZER_PIN, 0);
        }
      #endif
      
      if(millis() - blinkMillis > BLINK_PERIOD)
      {
        blinkMillis = millis();
        if(logoEnabled)
        {
          analogWrite(PWM_BLUE_PIN, 0);
          analogWrite(PWM_GREEN_PIN, 0);
          logoEnabled = false;
        }
        else
        {
          analogWrite(PWM_BLUE_PIN, brightness);
          analogWrite(PWM_GREEN_PIN, brightness);
          logoEnabled = true;
        }
      }
    }
  }
  else // siren not detected
  {
    digitalWrite(POW_EN_PIN, LOW);
    analogWrite(PWM_BLUE_PIN, 0);
    analogWrite(PWM_GREEN_PIN, 0);
    analogWrite(BUZER_PIN, 0);
    state = initialize;
  }
}
