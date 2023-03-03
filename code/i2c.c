// SPDX-License-Identifier: zlib-acknowledgement

// register level driver
// 
// CAN and UART bus length a lot longer
// SPI requires one dedicated signal for each device

// I2C has built-in error detection (SPI, some can go unnoticed)
// I2C does message framing (unlike UART)
// Use of open-drain reduces signal numbers by allowing sharing, however slows things down
// Could get address conflicts 
// (a solution could be to add another I2C bus)
// (also some devices addresses can be configured with an ADDR line connected to a GPIO)
// (only require unique at the time talking to device)

// as open-drain, low signal wins over high
// therefore, if identical addresses and two slaves respond
// 11111010 and 00001111 = 00001010
// master will be unawares of this

// transaction is read or write (bus is only active during a transaction)
// 1. generate start condition 
// (for multi-master bus arbitration, will first check if bus is busy)
// 2. devices have unique 7bit addresses 
//    (first byte of transaction from master is slave target address followed by read/write bit)
//    (slave address 0 is broadcast; in this case, data portion can indicate a standard procedure like soft reset)
//    (if reseting a device, the device must still in some workable condition, e.g. clock must work)
// 3. slaves send ACK if address match (no ACKs is considered a NACK)
// 4. read/write each byte followed by a NACK
// 5. generate stop condition (transaction over and bus is idle)

// open-drain technique to share electrical signal between devices

// reading trace signal:
// when SCL low-to-high, look at SDA, i.e. data sampled on rising edge
// so, data typically sampled via edge detection except:
//   * start condition: falling edge when clock high
//   * stop condition: rising edge when clock high
// low-value is ACK

// bus speed more-or-less the clock speed?

// clock stretching, i.e. keep clock low gives devices more time
// e.g. MCU can't generate data when slave wants it 

// combined format is write+read in a single transaction (provides atomicity)
// e.g. write register to then read from

// if multiple devices can't handle sample rate, perhaps put devices on separate buses to communicate in parallel

// mock i2c and timers for unit testing? 
