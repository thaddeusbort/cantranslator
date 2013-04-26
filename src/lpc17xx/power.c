#include "power.h"
#include "log.h"
#include "gpio.h"
#include <stdbool.h>
#include "lpc17xx_pinsel.h"
#include "lpc17xx_clkpwr.h"

#define POWER_CONTROL_PORT 2
#define POWER_CONTROL_PIN 13

#define PROGRAM_BUTTON_PORT 2
#define PROGRAM_BUTTON_PIN 12

#define USB_CONNECT_PORT 2
#define USB_CONNECT_PIN 9

uint32_t DISABLED_PERIPHERALS[] = {
    CLKPWR_PCONP_PCTIM0,
    CLKPWR_PCONP_PCTIM1,
    CLKPWR_PCONP_PCSPI,
    CLKPWR_PCONP_PCI2C0,
    CLKPWR_PCONP_PCRTC,
    CLKPWR_PCONP_PCSSP1,
    CLKPWR_PCONP_PCI2C1,
    CLKPWR_PCONP_PCSSP0,
    CLKPWR_PCONP_PCI2C2,
};

void setPowerPassthroughStatus(bool enabled) {
    int pinStatus;
    debugNoNewline("Switching 12v power passthrough ");
    if(enabled) {
        debug("on");
        pinStatus = 0;
    } else {
        debug("off");
        pinStatus = 1;
    }
    setGpioValue(POWER_CONTROL_PORT, POWER_CONTROL_PIN, pinStatus);
}

void initializePower() {
    debug("Initializing power controls...");
    // Configure 12v passthrough control as a digital output
    PINSEL_CFG_Type powerPassthroughPinConfig;
    powerPassthroughPinConfig.OpenDrain = 0;
    powerPassthroughPinConfig.Pinmode = 1;
    powerPassthroughPinConfig.Funcnum = 0;
    powerPassthroughPinConfig.Portnum = POWER_CONTROL_PORT;
    powerPassthroughPinConfig.Pinnum = POWER_CONTROL_PIN;
    PINSEL_ConfigPin(&powerPassthroughPinConfig);

    setGpioDirection(POWER_CONTROL_PORT, POWER_CONTROL_PIN, GPIO_DIRECTION_OUTPUT);
    setPowerPassthroughStatus(true);

    debug("Done.");

    debugNoNewline("Turning off unused peripherals...");
    for(int i = 0; i < sizeof(DISABLED_PERIPHERALS) /
            sizeof(DISABLED_PERIPHERALS[0]); i++) {
        CLKPWR_ConfigPPWR(DISABLED_PERIPHERALS[i], DISABLE);
    }

    debug("Done.");

    PINSEL_CFG_Type programButtonPinConfig;
    programButtonPinConfig.OpenDrain = 0;
    programButtonPinConfig.Pinmode = 1;
    programButtonPinConfig.Funcnum = 1;
    programButtonPinConfig.Portnum = PROGRAM_BUTTON_PORT;
    programButtonPinConfig.Pinnum = PROGRAM_BUTTON_PIN;
    PINSEL_ConfigPin(&programButtonPinConfig);
}

void updatePower() {
}

void handleWake() {
    // TODO This isn't especially graceful, we just reset the device after a
    // wakeup. Then again, it makes the code a hell of a lot simpler because we
    // only have to worry about initialization of core peripherals in one spot,
    // setup() in cantranslator.cpp and main.cpp. I'll leave this for now and we
    // can revisit it if there is some reason we need to keep state between CAN
    // wakeup events (e.g. fine_odometer_since_restart could be more persistant,
    // but then it actually might be more confusing since it'd be
    // fine_odometer_since_we_lost_power)
    NVIC_SystemReset();
}

void CANActivity_IRQHandler(void) {
    handleWake();
}

void EINT2_IRQHandler(void) {
    handleWake();
}

void enterLowPowerMode() {
    debug("Going to low power mode");
    NVIC_EnableIRQ(CANActivity_IRQn);
    NVIC_EnableIRQ(EINT2_IRQn);

    // Turn off USB connection status LED
    setGpioValue(USB_CONNECT_PORT, USB_CONNECT_PIN, GPIO_VALUE_HIGH);

    setPowerPassthroughStatus(false);

    // Disable brown-out detection when we go into lower power
    LPC_SC->PCON |= (1 << 2);

    CLKPWR_DeepSleep();
}
