/***
 * @Description:
 * @Date: 2024-12-27 18:06:36
 * @Author: 弈秋
 * @FirmwareVersion: v1.0.0.0
 * @LastEditTime: 2024-12-28 13:54:18
 * @LastEditors: 弈秋
 */

#ifndef __APPSTATEMACHINE_H
#define __APPSTATEMACHINE_H

#include "main.h"

/** Application modes
 * \note Item order matters
 */
typedef enum
{
    AppMode_off = 0,              /** \brief off state  */
    AppMode_boot,                 /** \brief boot state  */
    AppMode_configuration,        /** \brief configuration state */
    AppMode_hwInitialization,     /** \brief Hardware initialization */
    AppMode_appInitialization,    /** \brief Application initialization */
    AppMode_run,                  /** \brief run state */
    AppMode_limitedConfiguration, /** \brief configuration state (limited) */
    AppMode_error,                /** \brief error state  */
    AppMode_shutdown,             /** \brief shutdown state  */
} AppMode;

typedef struct
{
    // bool hwEmergency;
    volatile AppMode hwMode; /** \brief current mode */
    uint32_t ReadyCnt;       /** \brief counter for mode change */
    bool hwInitReady;
    volatile AppMode hwModeRequest; /** \brief requested mode */
} AppStateMachine;

void AppStateMachine_init(void);
void AppStateMachine_request(AppMode request); /** \brief Request a new mode */
void AppStateMachine_process(void);            /** \brief Process the mode change */

#endif /* __APPSTATEMACHINE_H */
/********************************END OF FILE**********************************/
