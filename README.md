# MultiStageThermostat
 Thermostat  module for controlling multi-stage device with DHT22

 This is code for controlling a multi-stage piece of HVAC equipment -- ie, one that has multiple output levels. It was written for the Chiltrix CXI series of fan coil units, which have four fan speeds, but could be used with any type of heating or cooling equipment that has multiple output levels.
 It requires a temperature sensor that detects the temperature, an on-off thermostat won't work. It is written for the DHT-22 sensor but could work with anything. The granularity will be the precision of the sensor.

 **Principle of operation:**
 The user specifies a set temperature and an allowable interval. When the user calls GetStage(), the thermostat reads the temperature. If the current temperature is withing the allowable interval of the set temperature, GetStage() returns the current stage. If the temperature is below the allowable interval, the stage is decreased by one and returned. If it is above the allowable interval the stage is increased and returned.
 To prevent rapid changing of stages, once the stage changes future changes are prevented for a user-defined interval.
On startup, if the temperature is below the allowable interval, the stage is set to maximum until the setpoint is reached.

**Summary of important functions**
MultiStageThermostat(int p_sensorPin); 
 Creates a MultiStageThermostat object. Parameter is the Arduino pin which is attached to the DHT22 temperature/humidity sensor

 
 * Initialization functions
void Setup(); 
 Initializes the DHT22 sensor
void setSettings(Settings & newSettings);
 Sets the setpoint and mode (heating or cooling). Has to be called before the object is used and can be called to change settings. The structure Settings has the members:
 int mode;
	float HeatingSetpoint;
	float CoolingSetpoint;
	int HumSetpoint;
	
	void setStageDelays(int p_stageCount, int * p_stageDelays);
  This sets the number of stages (p_stageCount) and also sets the delay between switching stages. I have found that some stages take longer to respond than others.
  
	void setThermostatInterval(float interval);
  This sets what the allowable interval is above and below the temeperature setting.
	void setDefaultStage(int stage);
  This sets the stage that will run if the object starts and the temperature is within the allowable interval of the setting.

*Running functions
 int getStage();
  This is the heart of the program. This is called repeatedly. It calls ReadTemp() to read the temperature and returns the appropriate stage.
	int getLastStage();
  This is called if you don't want to change the stage but want to know what getStage() returned the last time it was called.
 
 void ReadTemp();
  Reads the temperature from the DHT22
 float getTemp();
  Returns what the last read of ReadTemp() got
	float getHumidity();
  Returns the humidity reported by the last call to ReadTemp()



  **Sample Code: **
  MultiStageThermostat mst(DHTPin);
  #define MAXFANSPEED 4
  #define ADJUSTMENT_INTERVAL 180
  #define SetTemp 72
  
  void Setup()
  {


  int Delays[]=
{
  ADJUSTMENT_INTERVAL+60, 
  ADJUSTMENT_INTERVAL,
  ADJUSTMENT_INTERVAL,
  ADJUSTMENT_INTERVAL, 
  ADJUSTMENT_INTERVAL
};

    Settings settings;
    settings.CoolingSetpoint=SetTemp;
    settings.HeatingSetpoint=SetTemp;
    settings.mode=MODE_HEAT;

    mst.setThermostatInterval(0.2);
    mst.setDefaultStage(2);
     
    mst.setSettings(settings);
    mst.setStageDelays(MAXFANSPEED+1, Delays);
    mst.setup();    
	}

 void loop()
 {
 int stage=mst.getStage();
 }

 
