/**
 ******************************************************************************
 * File Name          : BatterySM.cpp
 * Description        : Primary battery state machine
 ******************************************************************************
*/
#include "BatterySM.hpp"
#include "SystemDefines.hpp"
#include "CommandMessage.hpp"
#include "WriteBufferFixedSize.h"
#include "GPIO.hpp"

/* Battery State Machine ------------------------------------------------------------------*/
/**
 * @brief Default constructor for Battery SM, initializes all states
 */
BatterySM::BatterySM(BatteryState startingState, bool enterStartingState)
{
    // Setup the internal array of states. Setup in order of enum.
    stateArray[BS_IDLE] = new Idle();
    stateArray[BS_CHARGING] = new Charging();
    stateArray[BS_DISCHARGING] = new Discharging();
    stateArray[BS_FAULT] = new Fault();

    // Verify all states are initialized AND state IDs are consistent
    for (uint8_t i = 0; i < BS_NONE; i++) {
        SOAR_ASSERT(stateArray[i] != nullptr);
        SOAR_ASSERT(stateArray[i]->GetStateID() == i);
    }

    bs_currentState = stateArray[startingState];

    // If we need to run OnEnter for the starting state, do so
    if (enterStartingState) {
        bs_currentState->OnEnter();
    }

    SOAR_PRINT("Battery State Machine Started in [ %s ] state\n", BaseBatteryState::StateToString(bs_currentState->GetStateID()));
}

/**
 * @brief Handles state transitions
 * @param nextState The next state to transition to
 * @return The state after the transition
 */
BatteryState BatterySM::TransitionState(BatteryState nextState)
{
    // Check if we're already in the next state (TransitionState does not allow entry into the existing state)
    if (nextState == bs_currentState->GetStateID())
        return bs_currentState->GetStateID();

    // Check the next state is valid
    if (nextState >= BS_NONE)
        return bs_currentState->GetStateID();

    BatteryState previousState = bs_currentState->GetStateID();

    // Exit the current state
    bs_currentState->OnExit();

    // Set the next state
    bs_currentState = stateArray[nextState];

    // Assert the next state is initalized
    SOAR_ASSERT(bs_currentState != nullptr, "bs_currentState is nullptr in TransitionState");

    // Enter the current state
    bs_currentState->OnEnter();

    SOAR_PRINT("BATTERY STATE TRANSITION [ %s ] --> [ %s ]\n", BaseBatteryState::StateToString(previousState), BaseBatteryState::StateToString(bs_currentState->GetStateID()));

    // Return the state after the transition
    return bs_currentState->GetStateID();
}

/**
 * @brief Handles current command
 * @param cm The command to handle
 */
void BatterySM::HandleCommand(Command& cm)
{
    SOAR_ASSERT(bs_currentState != nullptr, "Command received before state machine initialized");

    switch (cm.GetCommand()) {
    case DATA_COMMAND: {
        BatteryState nextState = bs_currentState->GetStateID(); // Default: no state change

        switch (cm.GetTaskCommand()) {
        case BMS_UPDATE: {
            BMSData bms;
            cm.CopyDataFromCommand((uint8_t*)&bms, sizeof(BMSData));
            nextState = bs_currentState->HandleBMSData(bms); // Returns new state
            break;
        }
        case CHARGER_UPDATE: {
            ChargerData charger;
            cm.CopyDataFromCommand((uint8_t*)&charger, sizeof(ChargerData));
            nextState = bs_currentState->HandleChargerData(charger); // Returns new state
            break;
        }
        case FUEL_GAUGE_UPDATE: {
            FuelGaugeData fuel_gauge;
            cm.CopyDataFromCommand((uint8_t*)&fuel_gauge, sizeof(FuelGaugeData));
            nextState = bs_currentState->HandleFuelGaugeData(fuel_gauge); // Returns new state
            break;
        }
        default:
            SOAR_PRINT("BatterySM - Unknown DATA_COMMAND TaskCommand: %d\n", cm.GetTaskCommand());
            break;
        }
    }
    }
}

/**
 * @brief Gets the current battery state as a proto enum
 * @return Current battery state
 */
Proto::BatteryState BatterySM::GetBatteryStateAsProto()
{
    switch (bs_currentState->GetStateID()) {
    case BS_IDLE:
        return Proto::BatteryState::BS_IDLE;
    case BS_CHARGING:
        return Proto::BatteryState::BS_CHARGING;
    case BS_DISCHARGING:
        return Proto::BatteryState::BS_DISCHHARGING;
    case BS_FAULT:
        return Proto::BatteryState::BS_FAULT;
    default:
        return Proto::BatteryState::BS_NONE;
    }
}

/* PreLaunch State ------------------------------------------------------------------*/
/**
 * @brief PreLaunch state constructor
 */
Idle::Idle()
{
    bsStateID = BS_IDLE;
}

/**
 * @brief Entry to Idle state
 * @return The state we're entering
 */
BatteryState Idle::OnEnter()
{
	// CHG/DSG FETs OFF

    return bsStateID;
}

/**
 * @brief Exit from Idle state
 * @return The state we're exiting
 */
BatteryState Idle::OnExit()
{
    // We don't do anything upon exiting idle

    return bsStateID;
}

/**
 * @brief Handles BMS data from Idle state
 * @return The rocket state to transition to or stay in. The current rocket state if no transition
 */
BatteryState Idle::HandleBMSData(const BMSData& bms)
{
    // Check for faults
	// Check voltage
	// Check current
	// etc...
    return currentState;
}

/**
 * @brief Handles charger data from Idle state
 * @return The rocket state to transition to or stay in. The current rocket state if no transition
 */
BatteryState Idle::HandleChargerData(const ChargerData& charger)
{
    // Check stuff
	// etc...
    return currentState;
}

/**
 * @brief Handles fuel gauge data from Idle state
 * @return The rocket state to transition to or stay in. The current rocket state if no transition
 */
BatteryState Idle::HandleFuelGaugeData(const FuelGaugeData& fuel_gauge)
{
    // Check stuff
	// etc...
    return currentState;
}

/* Charging State ------------------------------------------------------------------*/
/**
 * @brief Charging state constructor
 */
Charging::Charging()
{
    bsStateID = BS_CHARGING;
}

/**
 * @brief Entry to Charging state
 * @return The state we're entering
 */
BatteryState Charging::OnEnter()
{
	// CHG FET On
	// DSG FET Off

    return bsStateID;
}

/**
 * @brief Exit from Charging state
 * @return The state we're exiting
 */
BatteryState Charging::OnExit()
{
    // CHG FET Off

    return bsStateID;
}

/**
 * @brief Handles BMS data from Charging state
 * @return The rocket state to transition to or stay in. The current rocket state if no transition
 */
BatteryState Charging::HandleBMSData(const BMSData& bms)
{
    // Check for faults
	// Check voltage
	// Check current
	// etc...
    return currentState;
}

/**
 * @brief Handles charger data from Charging state
 * @return The rocket state to transition to or stay in. The current rocket state if no transition
 */
BatteryState Charging::HandleChargerData(const ChargerData& charger)
{
    // Check stuff
	// etc...
    return currentState;
}

/**
 * @brief Handles fuel gauge data from Charging state
 * @return The rocket state to transition to or stay in. The current rocket state if no transition
 */
BatteryState Charging::HandleFuelGaugeData(const FuelGaugeData& fuel_gauge)
{
    // Check stuff
	// etc...
    return currentState;
}

/* Discharging State ------------------------------------------------------------------*/
/**
 * @brief Discharging state constructor
 */
Discharging::Discharging()
{
    bsStateID = BS_DISCHARGING;
}

/**
 * @brief Entry to Discharging state
 * @return The state we're entering
 */
BatteryState Discharging::OnEnter()
{
	// CHG FET Off
	// DSG FET On

    return bsStateID;
}

/**
 * @brief Exit from Disharging state
 * @return The state we're exiting
 */
BatteryState Discharging::OnExit()
{
    // CHG FET Off
	// DSG FET Off

    return bsStateID;
}

/**
 * @brief Handles BMS data from Discharging state
 * @return The rocket state to transition to or stay in. The current rocket state if no transition
 */
BatteryState Discharging::HandleBMSData(const BMSData& bms)
{
    // Check for faults
	// Check voltage
	// Check current
	// etc...
    return currentState;
}

/**
 * @brief Handles charger data from Discharging state
 * @return The rocket state to transition to or stay in. The current rocket state if no transition
 */
BatteryState Discharging::HandleChargerData(const ChargerData& charger)
{
    // Check stuff
	// etc...
    return currentState;
}

/**
 * @brief Handles fuel gauge data from Discharging state
 * @return The rocket state to transition to or stay in. The current rocket state if no transition
 */
BatteryState Discharging::HandleFuelGaugeData(const FuelGaugeData& fuel_gauge)
{
    // Check stuff
	// etc...
    return currentState;
}

/* Fault State ------------------------------------------------------------------*/
/**
 * @brief Fault state constructor
 */
Fault::Fault()
{
    bsStateID = BS_FAULT;
}

/**
 * @brief Entry to Fault state
 * @return The state we're entering
 */
BatteryState Fault::OnEnter()
{
	// CHG FET Off
	// DSG FET Off
	// read who threw error
	// handle fault

    return bsStateID;
}

/**
 * @brief Exit from Fault state
 * @return The state we're exiting
 */
BatteryState Fault::OnExit()
{
	// clear error bits maybe?
	// or do nothing
    return bsStateID;
}

/**
 * @brief Handles BMS data from Fault state
 * @return The rocket state to transition to or stay in. The current rocket state if no transition
 */
BatteryState Fault::HandleBMSData(const BMSData& bms)
{
    // Check for faults
	// Check voltage
	// Check current
	// etc...
    return currentState;
}

/**
 * @brief Handles charger data from Discharging state
 * @return The rocket state to transition to or stay in. The current rocket state if no transition
 */
BatteryState Fault::HandleChargerData(const ChargerData& charger)
{
    // Check stuff
	// etc...
    return currentState;
}

/**
 * @brief Handles fuel gauge data from Discharging state
 * @return The rocket state to transition to or stay in. The current rocket state if no transition
 */
BatteryState Fault::HandleFuelGaugeData(const FuelGaugeData& fuel_gauge)
{
    // Check stuff
	// etc...
    return currentState;
}

/**
 * @brief Returns a string for the state
 */
const char* BaseBatteryState::StateToString(BatteryState stateId)
{
    switch(stateId) {
    case BS_IDLE:
        return "Idle";
    case BS_CHARGING:
        return "Charging";
    case BS_DISCHARGING:
        return "Discharging";
    case BS_FAULT:
        return "Fault";
    case BS_NONE:
        return "None";
    default:
        return "WARNING: Invalid";
    }
}
