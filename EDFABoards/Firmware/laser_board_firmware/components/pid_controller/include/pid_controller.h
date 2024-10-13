#pragma once

typedef struct pid_controller_t
{
    float Kp, Kd, Ki;

    float tau;

    float sampleTime;

    float limitMin, limitMax;

    float limitIntMin, limitIntMax;

    float integrator;
    float previousError;

    float differentiator;
    float previousMeasurement;

    float output;
} pid_controller_t;

void pid_controller_init(pid_controller_t *pid);

float pid_controller_update_laser(pid_controller_t *pid, float setpoint, float measurement);

float pid_controller_update_peltier(pid_controller_t *pid, float setpoint, float measurement);
