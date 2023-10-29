#include "pid_controller.h"

void pid_controller_init(pid_controller_t *pid)
{
    pid->integrator = 0.0f;
    pid->previousError = 0.0f;

    pid->differentiator = 0.0f;
    pid->previousMeasurement = 0.0f;

    pid->output = 0.0f;
}

float pid_controller_update(pid_controller_t *pid, float setpoint, float measurement)
{
    float error = setpoint - measurement;

    float proportional = pid->Kp * error;

    pid->integrator = pid->integrator + 0.5f * pid->Ki * pid->tau * (error + pid->previousError);

    if (pid->integrator > pid->limitIntMax)
    {
        pid->integrator = pid->limitIntMax;
    }
    else if (pid->integrator < pid->limitIntMin)
    {
        pid->integrator = pid->limitIntMin;
    }

    pid->differentiator = -(2.0f * pid->Kd * (measurement - pid->previousMeasurement) + (2.0f * pid->tau - pid->tau) * pid->differentiator) / (2.0f * pid->tau + pid->tau);

    pid->output = proportional + pid->integrator + pid->differentiator;

    if (pid->output > pid->limitMax)
    {
        pid->output = pid->limitMax;
    }
    else if (pid->output < pid->limitMin)
    {
        pid->output = pid->limitMin;
    }

    pid->previousError = error;
    pid->previousMeasurement = measurement;

    return pid->output;
}