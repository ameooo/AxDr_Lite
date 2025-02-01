/*
 * @Description:
 * @Date: 2024-12-27 18:06:36
 * @Author: 弈秋
 * @FirmwareVersion: v1.0.0.0
 * @LastEditTime: 2025-01-21 22:33:38
 * @LastEditors: 弈秋仙贝
 */

#include "AppStateMachine.h"
#include "mc_task.h"
#include "mgesc_state.h"

void AppStateMachine_enterBoot(AppStateMachine *stateMachine);
void AppStateMachine_enterConfiguration(AppStateMachine *stateMachine);
void AppStateMachine_enterHwInitialization(AppStateMachine *stateMachine);
void AppStateMachine_enterApplicationInitialization(AppStateMachine *stateMachine);
void AppStateMachine_enterRun(AppStateMachine *stateMachine);
void AppStateMachine_enterLimitedConfiguration(AppStateMachine *stateMachine);
void AppStateMachine_enterError(AppStateMachine *stateMachine);
void AppStateMachine_enterShutdown(AppStateMachine *stateMachine);

void AppStateMachine_processBoot(AppStateMachine *stateMachine);
void AppStateMachine_processConfiguration(AppStateMachine *stateMachine);
void AppStateMachine_processHwInitialization(AppStateMachine *stateMachine);
void AppStateMachine_processApplicationInitialization(AppStateMachine *stateMachine);
void AppStateMachine_processRun(AppStateMachine *stateMachine);
void AppStateMachine_processLimitedConfiguration(AppStateMachine *stateMachine);
void AppStateMachine_processError(AppStateMachine *stateMachine);
void AppStateMachine_processShutdown(AppStateMachine *stateMachine);

AppStateMachine AppState;

void AppStateMachine_init(void)
{
    AppState.ReadyCnt = 0;
    AppState.hwInitReady = false;
    AppState.hwMode = AppMode_off;
    AppState.hwModeRequest = AppMode_boot;
}

void AppStateMachine_request(AppMode request)
{
}

void AppStateMachine_process(void)
{

    if (AppState.hwModeRequest != AppState.hwMode)
    {
        AppMode hwMode = AppState.hwMode;

        /* State transition table */
        switch (AppState.hwModeRequest)
        {
        case AppMode_off:
            break;

        case AppMode_boot:
            AppStateMachine_enterBoot(&AppState);
            break;

        case AppMode_configuration:
            AppStateMachine_enterConfiguration(&AppState);
            break;

        case AppMode_hwInitialization:
            AppStateMachine_enterHwInitialization(&AppState);
            break;

        case AppMode_appInitialization:
            AppStateMachine_enterApplicationInitialization(&AppState);
            break;

        case AppMode_run:
            AppStateMachine_enterRun(&AppState);
            break;

        case AppMode_limitedConfiguration:
            AppStateMachine_enterLimitedConfiguration(&AppState);
            break;

        case AppMode_error:
            AppStateMachine_enterError(&AppState);
            break;

        case AppMode_shutdown:
            AppStateMachine_enterShutdown(&AppState);
            break;

        default:
            break;
        }

        if (AppState.hwMode == hwMode)
        {
            AppState.hwModeRequest = hwMode;
        }
    }

    /* State action table */
    switch (AppState.hwMode)
    {
    case AppMode_off:
        break;

    case AppMode_boot:
        AppStateMachine_processBoot(&AppState);
        break;

    case AppMode_configuration:
        AppStateMachine_processConfiguration(&AppState);
        break;

    case AppMode_hwInitialization:
        AppStateMachine_processHwInitialization(&AppState);
        break;

    case AppMode_appInitialization:
        AppStateMachine_processApplicationInitialization(&AppState);
        break;

    case AppMode_run:
        AppStateMachine_processRun(&AppState);
        break;

    case AppMode_limitedConfiguration:
        AppStateMachine_processLimitedConfiguration(&AppState);
        break;

    case AppMode_error:
        AppStateMachine_processError(&AppState);
        break;

    case AppMode_shutdown:
        AppStateMachine_processShutdown(&AppState);
        break;

    default:
        break;
    }
}

void AppStateMachine_enterBoot(AppStateMachine *stateMachine)
{
    if (AppState.hwMode == AppMode_off)
    {
        AppState.hwMode = AppState.hwModeRequest;
    }
}

void AppStateMachine_enterConfiguration(AppStateMachine *stateMachine)
{
    if (AppState.hwMode == AppMode_boot)
    {
        AppState.hwMode = AppState.hwModeRequest;
    }
}

void AppStateMachine_enterHwInitialization(AppStateMachine *stateMachine)
{
    if (AppState.hwMode == AppMode_configuration)
    {
		AppState.hwMode = AppState.hwModeRequest;
    }
}

void AppStateMachine_enterApplicationInitialization(AppStateMachine *stateMachine)
{
    if ((AppState.hwMode == AppMode_hwInitialization) || (AppState.hwMode == AppMode_limitedConfiguration))
    {
        AppState.hwMode = AppState.hwModeRequest;
    }
}

void AppStateMachine_enterRun(AppStateMachine *stateMachine)
{
    if (AppState.hwMode == AppMode_appInitialization)
    {
        AppState.hwMode = AppState.hwModeRequest;
    }
}

void AppStateMachine_enterLimitedConfiguration(AppStateMachine *stateMachine)
{
    if ((AppState.hwMode == AppMode_run) || (AppState.hwMode == AppMode_error))
    {
        AppState.hwMode = AppState.hwModeRequest;
    }
}

void AppStateMachine_enterError(AppStateMachine *stateMachine)
{
    if ((AppState.hwMode == AppMode_hwInitialization) || (AppState.hwMode == AppMode_appInitialization) || (AppState.hwMode == AppMode_run))
    {
        AppState.hwMode = AppState.hwModeRequest;
    }
}

void AppStateMachine_enterShutdown(AppStateMachine *stateMachine)
{
    if ((AppState.hwMode == AppMode_configuration) || (AppState.hwMode == AppMode_error) || (AppState.hwMode == AppMode_run) || (AppState.hwMode == AppMode_limitedConfiguration))
    {
        AppState.hwMode = AppState.hwModeRequest;
    }
}

void AppStateMachine_processBoot(AppStateMachine *stateMachine)
{
    AppState.hwModeRequest = AppMode_configuration;
}

void AppStateMachine_processConfiguration(AppStateMachine *stateMachine)
{
    MC_Motor_Init();
    AppState.hwModeRequest = AppMode_hwInitialization;
}

void AppStateMachine_processHwInitialization(AppStateMachine *stateMachine)
{
    mc_state_e state = MC_State_Process_Get();

    if (state >= MC_STATE_FAULT_NOW)
    {
        AppState.hwModeRequest = AppMode_error;
    }
    else if (state > MC_STATE_INIT)
    {
        AppState.hwModeRequest = AppMode_appInitialization;
    }
}

void AppStateMachine_processApplicationInitialization(AppStateMachine *stateMachine)
{
    AppState.hwModeRequest = AppMode_run;
}

void AppStateMachine_processRun(AppStateMachine *stateMachine)
{
}

void AppStateMachine_processLimitedConfiguration(AppStateMachine *stateMachine)
{
}

void AppStateMachine_processError(AppStateMachine *stateMachine)
{
}

void AppStateMachine_processShutdown(AppStateMachine *stateMachine)
{
}

bool AppStateMachine_initHardware(void)
{
    return true;
}
