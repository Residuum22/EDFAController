#include "pid_controller.h"
#include "esp_log.h"

void pid_controller_init(pid_controller_t *pid)
{
    pid->integrator = 0.0f;
    pid->previousError = 0.0f;

    pid->differentiator = 0.0f;
    pid->previousMeasurement = 0.0f;

    pid->output = 0.0f;
}

static void pid_controller_control(pid_controller_t *pid, float error, float measurement)
{
    float proportional = pid->Kp * error;

    pid->integrator = pid->integrator + 0.5f * pid->Ki * pid->sampleTime * (error + pid->previousError);

    if (pid->integrator > pid->limitIntMax)
    {
        pid->integrator = pid->limitIntMax;
    }
    else if (pid->integrator < pid->limitIntMin)
    {
        pid->integrator = pid->limitIntMin;
    }

    pid->differentiator = -(2.0f * pid->Kd * (measurement - pid->previousMeasurement) + (2.0f * pid->tau - pid->sampleTime) * pid->differentiator) / (2.0f * pid->tau + pid->sampleTime);

    ESP_LOGI("PID_CONTROLLER", "Proportional: %0.2f, Integrator: %0.2f", proportional, pid->integrator);

    pid->output = proportional + pid->integrator + pid->differentiator;

    if (pid->output > pid->limitMax)
    {
        pid->output = pid->limitMax;
    }
    else if (pid->output < pid->limitMin)
    {
        pid->output = pid->limitMin;
    }
}

float pid_controller_update_peltier(pid_controller_t *pid, float setpoint, float measurement)
{
    float error = setpoint - measurement;

    ESP_LOGI("PID_CONTROLLER", "%0.2f", error);

    if (error <= 0)
    {
        pid_controller_control(pid, error, measurement);
    }
    else
    {
        pid->integrator = 0;
    }

    pid->previousError = error;
    pid->previousMeasurement = measurement;
    
    // output will be negative but i cannot out
    pid->output = pid->output * -1;
    
    return pid->output;
}

float pid_controller_update_laser(pid_controller_t *pid, float setpoint, float measurement)
{
    float error = setpoint - measurement;

    ESP_LOGI("PID_CONTROLLER", "%0.2f", error);

    pid_controller_control(pid, error, measurement);

    pid->previousError = error;
    pid->previousMeasurement = measurement;

    return pid->output;
}