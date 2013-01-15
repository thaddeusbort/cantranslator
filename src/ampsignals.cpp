#ifndef CAN_EMULATOR
#include "canread.h"
#include "canwrite.h"
#include "signals.h"
#include "log.h"
#include "shared_handlers.h"
#include "handlers.h"

extern Listener listener;

#ifdef __LPC17XX__
#define can1 LPC_CAN1
#define can2 LPC_CAN2
#endif // __LPC17XX__

#ifdef __PIC32__
extern void* can1;
extern void* can2;
extern void handleCan1Interrupt();
extern void handleCan2Interrupt();
#endif // __PIC32__

const int CAN_BUS_COUNT = 2;
CanBus CAN_BUSES[CAN_BUS_COUNT] = {
    { 500000, 0x101, can1, 
#ifdef __PIC32__
        handleCan1Interrupt,
#endif // __PIC32__
    },
    { 250000, 0x102, can2, 
#ifdef __PIC32__
        handleCan2Interrupt,
#endif // __PIC32__
    },
};

const int MESSAGE_COUNT = 12;
CanMessage CAN_MESSAGES[MESSAGE_COUNT] = {
    {&CAN_BUSES[0], 1579}, // BMS Current
    {&CAN_BUSES[0], 1575}, // BMS Temps
    {&CAN_BUSES[0], 1578}, // BMS Voltage
    {&CAN_BUSES[0], 1744}, // Odometer
    {&CAN_BUSES[0], 1745}, // Vehicle Data
    {&CAN_BUSES[1], 419365953}, // Power Electronics Status
    {&CAN_BUSES[1], 419366209}, // Electric Vehicle Status
    {&CAN_BUSES[1], 419360256}, // Engine Temperature 1
    {&CAN_BUSES[1], 217056256}, // Electronic Engine Controller 1
    {&CAN_BUSES[1], 419365185}, // Charger Temps
    {&CAN_BUSES[1], 419361089}, // Cruise Control/Vehicle Speed
    {&CAN_BUSES[1], 419363905}, // Dash Display
};

const int SIGNAL_COUNT = 22;
CanSignalState SIGNAL_STATES[SIGNAL_COUNT][12] = {
    { {2, "DRIVE"}, {1, "NEUTRAL"}, {0, "REVERSE"}, {3, "PARK"}, },
    { {0, "STANDBY"}, {4, "CHARGE"}, {1, "DRIVE"}, {12, "CHARGECOMPLETE"}, {3, "PRECHARGE"}, },
    { {1, "ON"}, {0, "OFF"}, {2, "ERROR"}, },
    { {0, "DISABLED"}, {1, "ENABLED"}, {2, "ERROR"}, },
};

CanSignal SIGNALS[SIGNAL_COUNT] = {
    {&CAN_MESSAGES[0], "current", 0, 16, 0.100000, 0.000000, 0.000000, 0.000000, 1, true, false, NULL, 0, false, NULL}, // CellMaxTemp
    {&CAN_MESSAGES[1], "battery_temperature", 32, 8, 1.000000, 0.000000, 0.000000, 0.000000, 1, true, false, NULL, 0, false, NULL}, // CellMaxTemp
    {&CAN_MESSAGES[2], "pack_voltage", 0, 16, 0.100000, 0.000000, 0.000000, 0.000000, 1, true, false, NULL, 0, false, NULL}, // PackVoltage
    {&CAN_MESSAGES[3], "odometer", 0, 32, 0.125000, 0.000000, 0.000000, 0.000000, 1, true, false, NULL, 0, false, NULL}, // OdometerKm
    {&CAN_MESSAGES[4], "ambient_temp", 48, 8, 1.000000, -40.000000, 0.000000, 0.000000, 1, true, false, NULL, 0, false, NULL}, // AmbientTemp
    {&CAN_MESSAGES[4], "key_cranked", 31, 1, 1.000000, 0.000000, 0.000000, 0.000000, 1, false, false, NULL, 0, false, NULL}, // KeyCranked
    {&CAN_MESSAGES[4], "brake_pedal_status", 16, 8, 1.000000, 0.000000, 0.000000, 0.000000, 1, false, false, NULL, 0, false, NULL}, // Brake
    {&CAN_MESSAGES[4], "key_position", 28, 3, 1.000000, 0.000000, 0.000000, 0.000000, 1, false, false, NULL, 0, false, NULL}, // KeyPosition
    {&CAN_MESSAGES[4], "transmission_temp", 40, 8, 1.000000, -40.000000, 0.000000, 0.000000, 1, true, false, NULL, 0, false, NULL}, // TransmissionTemp
    {&CAN_MESSAGES[4], "prnd_position", 34, 2, 1.000000, 0.000000, 0.000000, 0.000000, 1, false, false, SIGNAL_STATES[0], 4, false, NULL}, // PRNDPosition
    {&CAN_MESSAGES[4], "accelerator_pedal", 8, 8, 1.000000, 0.000000, 0.000000, 0.000000, 1, true, false, NULL, 0, false, NULL}, // AcceleratorPedal
    {&CAN_MESSAGES[4], "vehicle_speed", 0, 8, 1.000000, 0.000000, 0.000000, 0.000000, 1, true, false, NULL, 0, false, NULL}, // VehicleSpeed
    {&CAN_MESSAGES[4], "coolant_temp", 56, 8, 1.000000, -40.000000, 0.000000, 0.000000, 1, true, false, NULL, 0, false, NULL}, // CoolantTemp
    {&CAN_MESSAGES[4], "transmission_gear_position", 36, 4, 1.000000, -1.000000, 0.000000, 0.000000, 1, false, false, NULL, 0, false, NULL}, // GearNumber
    {&CAN_MESSAGES[0], "inverter_temperature", 48, 16, 0.031250, -273.000000, 0.000000, 0.000000, 1, true, false, NULL, 0, false, NULL}, // PowerElectronicsMaxTemp
    {&CAN_MESSAGES[1], "vehicle_mode", 4, 4, 1.000000, 0.000000, 0.000000, 0.000000, 1, true, false, SIGNAL_STATES[1], 5, false, NULL}, // VehicleMode
    {&CAN_MESSAGES[2], "motor_temperature", 16, 16, 0.031250, -273.000000, 0.000000, 0.000000, 1, true, false, NULL, 0, false, NULL}, // MotorTemperature
    {&CAN_MESSAGES[3], "engine_speed", 24, 16, 0.125000, 0.000000, 0.000000, 0.000000, 15, true, false, NULL, 0, false, NULL}, // EngSpd
    {&CAN_MESSAGES[4], "charger_temperature", 48, 16, 0.031250, -273.000000, 0.000000, 0.000000, 1, true, false, NULL, 0, false, NULL}, // ChargerMaxTemp
    {&CAN_MESSAGES[5], "cruise_control_active", 30, 2, 1.000000, 0.000000, 0.000000, 0.000000, 1, true, false, SIGNAL_STATES[2], 3, false, NULL}, // CruiseControlActive
    {&CAN_MESSAGES[5], "cruise_control_enabled", 28, 2, 1.000000, 0.000000, 0.000000, 0.000000, 1, true, false, SIGNAL_STATES[3], 3, false, NULL}, // CruiseControlEnabled
    {&CAN_MESSAGES[6], "fuel_level", 8, 8, 0.400000, 0.000000, 0.000000, 0.000000, 1, true, false, NULL, 0, false, NULL}, // FuelLevel
};

const int IO_SIGNAL_COUNT = 4;
IoSignal IO_SIGNALS[IO_SIGNAL_COUNT] = {
    { "parking_brake_status", true, 1, true, false, 10 }, // ParkingBrakeStatus
    { "turn_signal_left", true, 1, true, false, 8 }, // TurnSignalLeft
    { "seatbelt_warning", true, 1, true, false, 11 }, // SeatbeltWarning
    { "turn_signal_right", true, 1, true, false, 9 }, // TurnSignalRight
};

const int COMMAND_COUNT = 0;
CanCommand COMMANDS[COMMAND_COUNT] = {
};

CanCommand* getCommands() {
    return COMMANDS;
}

int getCommandCount() {
    return COMMAND_COUNT;
}

CanSignal* getSignals() {
    return SIGNALS;
}

int getSignalCount() {
    return SIGNAL_COUNT;
}

CanBus* getCanBuses() {
    return CAN_BUSES;
}

int getCanBusCount() {
    return CAN_BUS_COUNT;
}

const char* getMessageSet() {
    return "generic";
}

int getIoSignalCount() {
    return IO_SIGNAL_COUNT;
}

IoSignal* getIoSignals() {
    return IO_SIGNALS;
}

void decodeCanMessage(int id, uint64_t data) {
    switch (id) {
    case 0x62b: // BMS Current
        translateCanSignal(&listener, &SIGNALS[0], data, SIGNALS, SIGNAL_COUNT); // CellMaxTemp
        break;
    case 0x627: // BMS Temps
        translateCanSignal(&listener, &SIGNALS[1], data, SIGNALS, SIGNAL_COUNT); // CellMaxTemp
        break;
    case 0x62a: // BMS Voltage
        translateCanSignal(&listener, &SIGNALS[2], data, SIGNALS, SIGNAL_COUNT); // PackVoltage
        break;
    case 0x6d0: // Odometer
        translateCanSignal(&listener, &SIGNALS[3], data, SIGNALS, SIGNAL_COUNT); // OdometerKm
        break;
    case 0x6d1: // Vehicle Data
        translateCanSignal(&listener, &SIGNALS[4], data, SIGNALS, SIGNAL_COUNT); // AmbientTemp
        translateCanSignal(&listener, &SIGNALS[5], data, &booleanHandler, SIGNALS, SIGNAL_COUNT); // KeyCranked
        translateCanSignal(&listener, &SIGNALS[6], data, &booleanHandler, SIGNALS, SIGNAL_COUNT); // Brake
        translateCanSignal(&listener, &SIGNALS[7], data, SIGNALS, SIGNAL_COUNT); // KeyPosition
        translateCanSignal(&listener, &SIGNALS[8], data, SIGNALS, SIGNAL_COUNT); // TransmissionTemp
        translateCanSignal(&listener, &SIGNALS[9], data, &stateHandler, SIGNALS, SIGNAL_COUNT); // PRNDPosition
        translateCanSignal(&listener, &SIGNALS[10], data, SIGNALS, SIGNAL_COUNT); // AcceleratorPedal
        translateCanSignal(&listener, &SIGNALS[11], data, SIGNALS, SIGNAL_COUNT); // VehicleSpeed
        translateCanSignal(&listener, &SIGNALS[12], data, SIGNALS, SIGNAL_COUNT); // CoolantTemp
        translateCanSignal(&listener, &SIGNALS[13], data, SIGNALS, SIGNAL_COUNT); // GearNumber
        break;
    case 0x18ff0441: // Power Electronics Status
        translateCanSignal(&listener, &SIGNALS[14], data, SIGNALS, SIGNAL_COUNT); // PowerElectronicsMaxTemp
        break;
    case 0x18ff0541: // Electric Vehicle Status
        translateCanSignal(&listener, &SIGNALS[15], data, &stateHandler, SIGNALS, SIGNAL_COUNT); // VehicleMode
        break;
    case 0x18feee00: // Engine Temperature 1
        translateCanSignal(&listener, &SIGNALS[16], data, SIGNALS, SIGNAL_COUNT); // MotorTemperature
        break;
    case 0xcf00400: // Electronic Engine Controller 1
        translateCanSignal(&listener, &SIGNALS[17], data, SIGNALS, SIGNAL_COUNT); // EngSpd
        break;
    case 0x18ff0141: // Charger Temps
        translateCanSignal(&listener, &SIGNALS[18], data, SIGNALS, SIGNAL_COUNT); // ChargerMaxTemp
        break;
    case 0x18fef141: // Cruise Control/Vehicle Speed
        translateCanSignal(&listener, &SIGNALS[19], data, &stateHandler, SIGNALS, SIGNAL_COUNT); // CruiseControlActive
        translateCanSignal(&listener, &SIGNALS[20], data, &stateHandler, SIGNALS, SIGNAL_COUNT); // CruiseControlEnabled
        break;
    case 0x18fefc41: // Dash Display
        translateCanSignal(&listener, &SIGNALS[21], data, SIGNALS, SIGNAL_COUNT); // FuelLevel
        break;
    }
}

void readIoSignal(IoSignal* signal) {
    translateIoSignal(&listener, signal);
}

CanFilter FILTERS[12];

CanFilter* initializeFilters(uint64_t address, int* count) {
    switch(address) {
    case 0x101:
        *count = 5;
        FILTERS[0] = {0, 0x62b, 1};
        FILTERS[1] = {1, 0x627, 1};
        FILTERS[2] = {2, 0x62a, 1};
        FILTERS[3] = {3, 0x6d0, 1};
        FILTERS[4] = {4, 0x6d1, 1};
        break;
    case 0x102:
        *count = 7;
        FILTERS[0] = {0, 0x18ff0441, 1};
        FILTERS[1] = {1, 0x18ff0541, 1};
        FILTERS[2] = {2, 0x18feee00, 1};
        FILTERS[3] = {3, 0xcf00400, 1};
        FILTERS[4] = {4, 0x18ff0141, 1};
        FILTERS[5] = {5, 0x18fef141, 1};
        FILTERS[6] = {6, 0x18fefc41, 1};
        break;
    }
    return FILTERS;
}

#endif // CAN_EMULATOR
