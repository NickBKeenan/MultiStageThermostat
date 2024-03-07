
#include <MultiStageThermostat.h>
MultiStageThermostat::MultiStageThermostat(int p_sensorpin) :sensor(p_sensorpin, DHT22)
{
    
    setThermostatInterval(0.1);
    setDefaultStage(0);
    setStageDelays(0, NULL);
    Stage = 0;
    stageCount = 0;

}

void MultiStageThermostat::setSettings( Settings & newsettings)
{
    startingmode = true;
    nextAdjustmentTime = 0;
   
    upperthreshold = 0;
    lowerthreshold = 0;
    mode = newsettings.mode;
    if (mode == MODE_HEAT)
    {
        // round to nearest 0.1 degrees
        int temp;
        temp= newsettings.HeatingSetpoint*10;
        setTemp = temp;
        setTemp /= 10;
    }
    else
    {
        // round to nearest 0.1 degrees
        int temp;
        temp = newsettings.CoolingSetpoint * 10;
        setTemp = temp;
        setTemp /= 10;
    }
    
}
void MultiStageThermostat::ReadTemp()
{
    float hum, temp;
    hum = sensor.readHumidity();
    temp = sensor.readTemperature();
    if (!isnan(temp))
    {
        lastTemp = temp;

    }
    if(!isnan(hum))
    {
        lastHum = hum;
    }

}
int MultiStageThermostat::getStage()
{
 
    //////////////////////////////////////////////////////////////////////////////

    // This is the heart of the program.

    // Essentially, you check the current temperature, if it's above the setpoint you slow the fan down, if it's below you speed the fan up
    // But there are lots of twists

    // If the fan is on its lowest setting, and the temperture is above the setpoint, stop the fan. Do this by switching the unit out of heating mode
    // and into auto mode. The setpoint for auto needs to be set below that for heating.

    // You don't want the fan speed changing around a lot, that's annoying. So the variable nextAdjustmentTime is used to limit adjustments to one
    // every three minutes -- four if you're coming out of being off because it takes about a minute for the unit to come on

    // The temperature sensor tends to bounce around between two degrees. This function gets called whenever the temperature changes, you only want to 
    // change the fan speed once for each setpoint. The variables upperthreshold and lowerthreshold hold the current points at which the fan speed changes. 
    // Once a threshold has been hit, it gets moved 0.1 away from the setpoint, so the temperature has to move a whole unit to move it again. If the
    // temperture moves two units toward the setpoint then the threshold gets reset to its original value.

    // On startup, if the temperature is below the setpoint the fan is put on highest speed until the setpoint is reached for the first time.
    // This is tracked in the variable startingmode.

    // modes
    //0=auto；1=cooling;2=dehumidification；3=ventilate；4=heating；
    // dehum and ventilate don't care about temperature, just let run
    // cooling and heating are the same, the only difference is which direction the temperature goes in
    // auto I need to think about
    ReadTemp();

    if (stageCount == 0)
    {
        Stage = 0;
        return Stage;
    }
    
    // everything is different in starting mode
    if (startingmode)
    {
        onStartingMode();
        return Stage;   
    }


        //Whenever we cross a threshold, we increase that threshold so we don't keep crossing back and forth
        // When we move 2 units away from the threshold toward the setpoint, then reset the threshold 
        //does the threshold need to be adjusted? Have we moved two units closer to the setpoint?
        if (upperthreshold > lastTemp + 2*thermostatInterval)
        {
            upperthreshold = lastTemp + 2 * thermostatInterval;
            if (upperthreshold < setTemp + thermostatInterval)
            {
                upperthreshold = setTemp + thermostatInterval;
            }
        }
        if (lowerthreshold < lastTemp - 2*thermostatInterval)
        {
            lowerthreshold = lastTemp - 2 * thermostatInterval;
            if (lowerthreshold > setTemp - thermostatInterval)
            {
                lowerthreshold = setTemp - thermostatInterval;
            }

        }





        if (millis() > nextAdjustmentTime) // don't adust fan more than once every three minutes
        {
            // this is meant to compare temp >=upperthreshold . However since they are floating points the exact equals doesn't always hit
            if (upperthreshold - lastTemp < 0.09)
            {

                OnHitUpperLimit();
            }
            if (lastTemp - lowerthreshold < 0.09)
            {
                
                OnHitLowerLimit();
            }
        }


        if (Stage < 0) {
            Stage = 0;
        }
        if (Stage > stageCount-1) {
            Stage = stageCount-1;
        }

 /*
        
        Serial.println();
        Serial.print("Stage=");
        Serial.print(Stage);
        Serial.print(" Setpoint=");
        Serial.print(setTemp);
        Serial.print(" Temp=");
        Serial.print(lastTemp);
        Serial.print(" Upper lim=");
        Serial.print(upperthreshold);
        Serial.print(" Lower lim=");
        Serial.print(lowerthreshold);
        Serial.print(" Time to next check=");
        if (nextAdjustmentTime > millis())
        {
            Serial.print((nextAdjustmentTime-millis())/1000);
        }
        else
            Serial.print("0");

   */     
        return Stage; // success!
    


}
void MultiStageThermostat::onStartingMode()
{
    // When we first start up, we have no history, so we have to figure out where to start. 
    // on startup, there  are three possibilities: 
    // we're outside the band to the good (ie above if heating, below if cooling). In that case, exit starting mode and set stage to zero
    // we're outside the band to the bad (ie below if heating, above if cooling). In that case, stay in starting mode and set stage to maximum
    // we're within the band. Exit starting mode. Stage will either be zero or maxium. If it's zero, stay zero. Otherwise throttle back to defaultStage

    
    
            
        upperthreshold = setTemp + thermostatInterval;
        lowerthreshold = setTemp - thermostatInterval;
        // if we're outside the thermostat band, set to off or highest. Otherwise do nothing, just keep doing what you're doing
        if (mode == MODE_HEAT)
        {
            if (lastTemp >= upperthreshold) {

                Stage = 0;;
                startingmode = false;

            }
            else
            {
                if (lastTemp > lowerthreshold)
                {
                    if (Stage > defaultStage)
                        Stage = defaultStage;
                    nextAdjustmentTime = millis() + stageDelays[Stage] * 1000;
                    startingmode = false;
                }
                else
                {
                    Stage = stageCount - 1; // set for highest speed
                }
            }

        }
        if (mode == MODE_COOL)
        {
            if (lastTemp <= lowerthreshold) 
            {
                Stage = 0;
                startingmode = false;

            }
            else
            {
                if (lastTemp < upperthreshold)
                {
                    if (Stage > defaultStage)
                        Stage = defaultStage;
                    startingmode = false;
                    nextAdjustmentTime = millis() + stageDelays[Stage] * 1000;

                }
                else
                {


                   Stage = stageCount - 1; // maximum speed


                }

            }
        }
        /*
        Serial.println();
        Serial.print("Starting Mode: Stage=");
        Serial.print(Stage);
        Serial.print(" Setpoint=");
        Serial.print(setTemp);
        Serial.print(" Temp=");
        Serial.print(lastTemp);
        */


}
void MultiStageThermostat::setup()
{
    
    sensor.begin();
    lastHum=0;   //Stores humidity value

    lastTemp=0;  //Stores temperature value
    Stage = 0;
    

}


void MultiStageThermostat::OnHitUpperLimit()
// called when the upper thermostat limit is hit
{
    int interval;
    if (mode == MODE_HEAT)
    {
        if (Stage == 0) // don't do anything if we're already off
            return;
        Stage--;
        interval = stageDelays[Stage];
    }
    if (mode == MODE_COOL)
    {
        if (Stage == stageCount - 1)
            return;
        interval = stageDelays[Stage];
        Stage++;
    }
    
    // move the threshold up by a unit so we don't trigger again
    upperthreshold += thermostatInterval;
    
    // if we're turning fan off, allow another 60 seconds to turn it back on again
    // when you turn the fan speed off, it shuts off the zone valve. When you turn it back on again it takes about 60 seconds for the coil to heat up

    // this needs to be modified to use array of intervals;
    nextAdjustmentTime = millis() + interval*1000; 

}
void MultiStageThermostat::OnHitLowerLimit()
// called when the lower thermostat limit is hit
{
    int interval;
    if (mode == MODE_HEAT)
    {
        if (Stage == stageCount - 1)
            return;
        interval = stageDelays[Stage];
        Stage++;
    }
    if (mode == MODE_COOL)
    { 
        if (Stage == 0)
            return;
        Stage--;
        
        if (Stage <= 0)
        {
            Stage = 0;
        }
        interval = stageDelays[Stage];
    }
    // move the threshold down a unit
    lowerthreshold -= thermostatInterval;
    nextAdjustmentTime = millis() + interval * 1000;

}

float MultiStageThermostat::getTemp()
{
    return lastTemp;
}
float MultiStageThermostat::getHumidity()
{
    return lastHum;
}
int MultiStageThermostat::getLastStage()
{
    return Stage;
}
float MultiStageThermostat::CtoF(float inC)
{
    float retval;
    retval = inC * 9 / 5 + 32;
    return retval;
}

float MultiStageThermostat::FtoC(float inF)
{
    float retval;
    retval = (inF-32) *  5/9 ;
    return retval;
}

void MultiStageThermostat::setStageDelays(int p_stageCount, int* p_stageDelays)
{
     stageCount= p_stageCount; // the number of stages
     stageDelays= p_stageDelays; // array of delays between stage changes


}

void MultiStageThermostat::setThermostatInterval(float interval)
{
    thermostatInterval = interval;
}
void MultiStageThermostat::setDefaultStage(int newStage)
{
    defaultStage = newStage;
}