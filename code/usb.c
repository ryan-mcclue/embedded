// SPDX-License-Identifier: zlib-acknowledgement

/*
 * USB classes:
 *   * CDC (FS, HS): higher throughput. CDC emulates COM port?
 *   * MSC: like CDC uses bulk transfer for IN and OUT; expects files
 *   * HID (FS, HS): sends data in one direction. uses interrupts
 *
 * VCP (virtual com port), i.e. USB device?
 * OTG, i.e. USB host?
 *
 * USB type C:
 *   * Downstream Facing Port (DFP) hub
 *   * Upstream Facing Port (UFP) device
 *
 *  ESD protection protects against short duration, high voltage pulses from damaging your device, 
 *  by redirecting the energy to the circuit ground. 
 *  The diodes begin to conduct (basically by shorting to the ground) and absorb the current when the voltage across then goes above a certain threshold. 
 *  ESD events are common when you plug in connectors.
 *
    ESD is ElectroStatic Discharge protection. That is the same thing as the spark to the doorknob in the winter. It typically involves 1000 or more volts which can easily be developed by rubbing your shoes on a nylon carpet, but extremely low energy (even though your finger won't think of it as low).
    If you bring your USB drive in your pocket, then the drive will be at the same voltage potential as your hand, but may be thousands of volts different than the computer or laptop. So, yes, ESD happens when you touch the housing of the USB plug to the connector.*
 *
 *   IMPORTANT(Ryan): With oscilloscope, will get odd looking pulses if undersampling
 */
