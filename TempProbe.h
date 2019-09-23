/*
 * TempProbe.h
 */

 #define ADC_BIT_DEPTH 10
 #define ADC_OHM_VALUE 2.2e4

 class TempProbe {
 public:
   typedef enum {
     STATUS_NONE,
     STATUS_OK,
   } temp_status_t;

   TempProbe(const unsigned char pin);

   void Process();
   float getTemp();
   float getTempAvg();
   temp_status_t getStatus();

 private:
   void calcTemp();
   void calcTempAvg();
   void setTemp(const float temp);
   void setStatus(const temp_status_t status);

 private:
   struct SteinhartCoefficients {
     float A, B, C;
   };

   int _pin;
   float _temp;
   float _tempAvg;
   bool _hasAvg;
   float _offset;
   temp_status_t _status;
   SteinhartCoefficients _s = {7.3431401e-4, 2.1574370e-4, 9.5156860e-8};
 };

 TempProbe::TempProbe(const unsigned char pin) :
  _pin(pin),
  _temp(0.0f),
  _tempAvg(0),
  _hasAvg(false),
  _offset(0.0f),
  _status(STATUS_NONE) {
 }

 void TempProbe::Process() {
   calcTemp();
   calcTempAvg();
 }

 constexpr float celsiusToFahrenheit(const float tempC) {
   return 32.0f + ((9.0f / 5.0f) * tempC);
 }

 void TempProbe::setTemp(const float temp) {
   _temp = celsiusToFahrenheit(temp);
   _temp += _offset;
   setStatus(STATUS_OK);
 }

 void TempProbe::setStatus(const temp_status_t status){
   _status = status;
 }

 float TempProbe::getTemp() {
   return _temp;
 }

 float TempProbe::getTempAvg() {
   return _tempAvg;
 }

TempProbe::temp_status_t TempProbe::getStatus() {
   return _status;
 }

 void TempProbe::calcTemp() {
   static_assert(ADC_BIT_DEPTH <= (8 * sizeof(int)), "ADC bit depth too large");
   static constexpr int amax = (1 << ADC_BIT_DEPTH) - 1;

   float aval = analogRead(_pin);
   if (aval == 0 || aval == amax) {
     setStatus(STATUS_NONE);
     return;
   }

   const float lnR = log(ADC_OHM_VALUE / (((float) amax / aval) - 1.0f));
   const float tempK = 1.0f / ((_s.A) + (_s.B * lnR) + (_s.C * lnR * lnR * lnR));

   setTemp(tempK - 273.15f);
 }

 void TempProbe::calcTempAvg() {
   if (_status == STATUS_OK) {
     if (!_hasAvg) {
       _tempAvg = _temp;
       _hasAvg = true;
     } else {
       _tempAvg += (2.0f / (1.0f + 60.0f)) * (_temp - _tempAvg);
     }
   }
 }
