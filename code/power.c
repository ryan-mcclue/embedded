// SPDX-License-Identifier: zlib-acknowledgement

// diode for voltage drop with minimal power loss
// diode voltage drop is also independent of current value 
// less flexible voltage drop, e.g. 0.6V-1.7V (schottky diode 0.15V-0.45V) (zener diode?)
// why not use a voltage regulator? LDO?

// power calculation: 
//  * mcu runs at 1.5V-3.6V (draws less current if running at lower voltage?)
//  * peripheral runs at 1.8V-3.6V drawing max 130mA


