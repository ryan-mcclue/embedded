// SPDX-License-Identifier: zlib-acknowledgement

// register level driver
// 
// CAN and UART bus length a lot longer
// SPI requires one dedicated signal for each device

// I2C has built-in error detection (SPI, some can go unnoticed)
// I2C does message framing (unlike UART)
// Use of open-drain reduces signal numbers by allowing sharing, however slows things down
// Could get address conflicts as certain devices only have certain configurable addresses
// (a solution could be to add another I2C bus)

// transaction is read or write
// 1. generate start condition 
// 2. devices have unique 7bit addresses
//    (first byte of transaction from master is slave target address followed by read/write bit)
// 3. slaves send ACK if address match (no ACKs is considered a NACK)
// 4. read/write each byte followed by a NACK
// 5. generate stop condition (transaction over and bus is idle)

// open-drain technique to share electrical signal between devices
