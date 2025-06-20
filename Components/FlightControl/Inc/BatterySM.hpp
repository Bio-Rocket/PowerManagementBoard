/**
 ******************************************************************************
 * File Name          : BatterySM.hpp
 * Description        : Battery state machine, handles all battery state transitions.
 ******************************************************************************
*/
#ifndef BR_AVIONICS_BATTERY_SM
#define BR_AVIONICS_BATTERY_SM

#include "Command.hpp"
#include "CoreProto.h"

enum BatteryState
{
    BS_IDLE = 0,
    BS_CHARGING,
    BS_DISCHARGING,
	BS_FAULT,
    BS_NONE         // Invalid state, must be last
};

/**
 * @brief Base class for Battery State Machine
 */
class BaseBatteryState
{
public:
    virtual BatteryState HandleCommand(Command& cm) = 0; //Handle a command based on the current state
    virtual BatteryState OnEnter() = 0;  //Returns the state we're entering
    virtual BatteryState OnExit() = 0;   //Returns the state we're exiting
    virtual BatteryState HandleBMSData(const BMSData& bms) = 0;
    virtual BatteryState HandleChargerData(const ChargerData& charger) = 0;
    virtual BatteryState HandleFuelGaugeData(const FuelGaugeData& fuel_gauge) = 0;

    virtual BatteryState GetStateID() { return bsStateID; }

    static const char* StateToString(BatteryState stateId);

protected:
    BatteryState bsStateID = BS_NONE;    //The name of the state we're in

};

/**
 * @brief Battery State Machine
 */
class BatterySM
{
public:
    BatterySM(BatteryState startingState, bool enterStartingState);

    void HandleCommand(Command& cm);

    Proto::BatteryState GetBatteryStateAsProto();

protected:
    BatteryState TransitionState(BatteryState nextState);

    // Variables
    BaseBatteryState* stateArray[BS_NONE];
    BaseBatteryState* bs_currentState;
};

/**
 * @brief Idle state, waiting for commands to proceeding sequences
 */
class Idle : public BaseBatteryState
{
public:
    Idle();

    // Base class
    BatteryState HandleCommand(Command& cm) override;
    BatteryState OnEnter() override;
    BatteryState OnExit() override;
    BatteryState HandleBMSData(const BMSData& bms) override;
    BatteryState HandleChargerData(const ChargerData& charger) override;
    BatteryState HandleFuelGaugeData(const FuelGaugeData& fuel_gauge) override;
};

/**
 * @brief Charging state
 */
class Charging : public BaseBatteryState
{
public:
    Charging();

    BatteryState HandleCommand(Command& cm) override;
    BatteryState OnEnter() override;
    BatteryState OnExit() override;
    BatteryState HandleBMSData(const BMSData& bms) override;
    BatteryState HandleChargerData(const ChargerData& charger) override;
    BatteryState HandleFuelGaugeData(const FuelGaugeData& fuel_gauge) override;
};

/**
 * @brief Charging state
 */
class Discharging : public BaseBatteryState
{
public:
    Discharging();

    BatteryState HandleCommand(Command& cm) override;
    BatteryState OnEnter() override;
    BatteryState OnExit() override;
    BatteryState HandleBMSData(const BMSData& bms) override;
    BatteryState HandleChargerData(const ChargerData& charger) override;
    BatteryState HandleFuelGaugeData(const FuelGaugeData& fuel_gauge) override;
};

/**
 * @brief Fault state
 */
class Fault : public BaseBatteryState
{
public:
    Fault();

    BatteryState HandleCommand(Command& cm) override;
    BatteryState OnEnter() override;
    BatteryState OnExit() override;
    BatteryState HandleBMSData(const BMSData& bms) override;
    BatteryState HandleChargerData(const ChargerData& charger) override;
    BatteryState HandleFuelGaugeData(const FuelGaugeData& fuel_gauge) override;

};


#endif // BR_AVIONICS_BATTERY_SM
