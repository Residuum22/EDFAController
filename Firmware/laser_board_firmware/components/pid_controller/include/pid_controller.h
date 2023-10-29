#pragma once

typedef struct pid_controller_t
{
    float Kp, Kd, Ki;

    float tau;

    float limitMin, limitMax;

    float limitIntMin, limitIntMax;

    float sampleTime;

    float integrator;
    float previousError;

    float differentiator;
    float previousMeasurement;

    float output;
} pid_controller_t;

void pid_controller_init(pid_controller_t *pid);

float pid_controller_update(pid_controller_t *pid, float setpoint, float measurement);
