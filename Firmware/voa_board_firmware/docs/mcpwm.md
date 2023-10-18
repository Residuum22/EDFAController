# MCPWM Design Plan

## Available resources

```c
#define SOC_MCPWM_GROUPS                     (2)    ///< 2 MCPWM groups on the chip (i.e., the number of independent MCPWM peripherals)
#define SOC_MCPWM_TIMERS_PER_GROUP           (3)    ///< The number of timers that each group has
#define SOC_MCPWM_OPERATORS_PER_GROUP        (3)    ///< The number of operators that each group has
#define SOC_MCPWM_COMPARATORS_PER_OPERATOR   (2)    ///< The number of comparators that each operator has
#define SOC_MCPWM_GENERATORS_PER_OPERATOR    (2)    ///< The number of generators that each operator has
#define SOC_MCPWM_TRIGGERS_PER_OPERATOR      (2)    ///< The number of triggers that each operator has
#define SOC_MCPWM_GPIO_FAULTS_PER_GROUP      (3)    ///< The number of fault signal detectors that each group has
#define SOC_MCPWM_CAPTURE_TIMERS_PER_GROUP   (1)    ///< The number of capture timers that each group has
#define SOC_MCPWM_CAPTURE_CHANNELS_PER_TIMER (3)    ///< The number of capture channels that each capture timer has
#define SOC_MCPWM_GPIO_SYNCHROS_PER_GROUP    (3)    ///< The number of GPIO synchros that each group has
#define SOC_MCPWM_SWSYNC_CAN_PROPAGATE       (1)    ///< Software sync event can be routed to its output
```

### Timers

According to the resources each MCPWM group has three independent timer, but these timer aren't in syncron.

### Operators & Comparators & Generators

Each MCPWM group has three different operator, each stores two comparators and two generators.

## ZXB52* motor driver IC

- There is two input pin
    - Reverse (REV)
    - Forward (FWD)

## GOAL

Create a stepper motor driver with these components.

## Design

This stepper motor is a bipolar stepper motor, with two independent coil. These are required two different motor driver.
Forward and reverse mode can't be in one MCPWM group because:
- FWD and REV inputs need two generators, which are only available in one operator group.
- Two operator needed because of the two independent coils of the stepper motor. 
- Three operator are available in one MCPWM group and two are allocated for forward mode and the left one is not enough for reverse.

## Issue during development

With two MCPWM group (one for reverse and one for forwards) when rooted in the same output pins only one will work.

-> Solution: On direction change the group need to be deinitialized and reinitialize.

\+ Information: Only one timer is enough because in this case no timer syncronisation is needed.

### Forward matrix

A+|A-|B+|B-|
1|0|1|0| 
0|1|1|0| 
0|1|0|1| 
1|0|0|1| 

### Reverse matrix

A+|A-|B+|B-|
1|0|0|1| 
0|1|0|1| 
0|1|1|0| 
1|0|1|0| 