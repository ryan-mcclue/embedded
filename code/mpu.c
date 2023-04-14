// SPDX-License-Identifier: zlib-acknowledgement

// mpu optional feature outlined in armv7-m
// so, present in most cortex-m

// in 'Thread mode', i.e. outside of ISR, priveleged or unpriveleged (MPU can change access based on these two modes)

// __attribute__((used)) to overide -ffunction-sections

// if running third party code, use MPU to disable reading from external Flash
// security, prevent executing data as code
// stack overflow
