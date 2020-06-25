/*
 * drv_button.c
 *
 *  Created on: 25 ����. 2020 �.
 *      Author: vladyslav.daliavskyi
 */

#include "drv_button.h"

static const drvButton_data buttonData[drvButton_Count] = {
    { halGPIO_PortA, halGPIO_Pin0 },
};

void drvButtonInit(drvButton_struct* const buttonStruct) {
    halGPIO_struct* hal_GPIO_struct = (halGPIO_struct*) buttonStruct;

    hal_GPIO_struct->port = buttonData[buttonStruct->button].port;

    const halGPIO_initStruct initStruct = {
            buttonData[buttonStruct->button].pin,
            halGPIO_ModeIn,
            halGPIO_Speed2Mhz,
            halGPIO_OTPP,
            halGPIO_PuPdNo
    };

    halGPIOInit(hal_GPIO_struct, &initStruct);

    buttonStruct->click_counter = 0;
    buttonStruct->click_duration = 0;

    for (uint8_t i=0; i<drvButton_ActionsCount; i++){
        buttonStruct->actionCallbacks[i] = NULL;
    }
}

void drvButtonSetCallBack(drvButton_struct* const buttonStruct,
        const drvButton_action action, const drvButtonActionCallBack callback) {

    buttonStruct->actionCallbacks[action] = callback;
}

static uint8_t drvButtonIsClicked(drvButton_struct* const buttonStruct) {
    // check on high pin level
    if (halGPIOReadPin((halGPIO_struct*) buttonStruct) == halGPIO_Set
            // check out of limit
            && buttonStruct->click_counter < 110) {

        buttonStruct->click_counter++;

    } else if (buttonStruct->click_counter != 0) {

        buttonStruct->click_counter--;

    }

    if (buttonStruct->click_counter > 100) return drvButton_Clicked;
    if (buttonStruct->click_counter < 10)  return drvButton_Falled;

    return drvButton_Float;
}

void drvButtonRun(drvButton_struct* const buttonStruct) {

    switch (drvButtonIsClicked(buttonStruct)) {

        case drvButton_Clicked:
            buttonStruct->click_duration++;
            break;

        case drvButton_Falled:
            if (buttonStruct->click_duration > drvButtonLongPressDuration &&
                    buttonStruct->actionCallbacks[drvButton_LongPress] != NULL) {

                buttonStruct->actionCallbacks[drvButton_LongPress]();

            } else if (buttonStruct->click_duration > drvButtonSimplePressDuration &&
                    buttonStruct->actionCallbacks[drvButton_Press] != NULL) {

                    buttonStruct->actionCallbacks[drvButton_Press]();

            }

            buttonStruct->click_duration = 0;
            break;

        default:
            break;
    }
}



