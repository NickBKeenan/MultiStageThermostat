//
#ifndef MultistageTherostat_h
#define MultistageTherostat_h

#include <DHT.h>

#define MODE_HEAT 1
#define MODE_MANUAL 2
#define MODE_COOL -1
#define MODE_OFF 0
#define MODE_FLOOR 3

struct Settings {
	int mode;
	float HeatingSetpoint;
	float CoolingSetpoint;
	int HumSetpoint;
};


class MultiStageThermostat
{

	DHT sensor;

	float lastHum;   //Stores humidity value

	float lastTemp;  //Stores temperature value
	int Stage; // what was returned the last time we checked the temperature

	// settings variables
	float setTemp;   // temperature setting from the unit
	int mode; // heating or cooling
	// configurable variables
	float thermostatInterval; // how much above or below the setpoint to trigger a change of stage
	int stageCount; // the number of stages. Includes 0, so maximum is stageCount-1
	int* stageDelays; // array of delays between stage changes
	int defaultStage; // on start mode, where to go by default. 
	
	void OnHitUpperLimit(); // called when the upper thermostat limit is hit
	void OnHitLowerLimit(); // called when the lower thermostat limit is hit
	void onStartingMode(); // called when we're in starting mode
	

protected: // these are used for defluttering
	bool startingmode;  //this is true until the thermostat has been satisfied for the first time
	unsigned long nextAdjustmentTime = 0; //don't adjust the fan speed more than once every three minutes
	float upperthreshold = 0;  // if we hit this temperature, slow the fan down
	float lowerthreshold = 0; // if we hit this temperature, speed the fan up
public:
	MultiStageThermostat(int p_sensorPin);
	void setup();
	void setSettings(Settings & newSettings);
	void ReadTemp();
	
	void setStageDelays(int p_stageCount, int * p_stageDelays);
	float getTemp();
	float getHumidity();

	char* getShortStatus();
	char* getLongStatus();
	int getStage();
	int getLastStage();

	float CtoF(float Cin);
	float FtoC(float Fin);
	void setThermostatInterval(float interval);
	void setDefaultStage(int stage);



};
#endif