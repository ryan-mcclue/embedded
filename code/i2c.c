// SPDX-License-Identifier: zlib-acknowledgement

// drivers: uart, dio, timer (wrap HAL_GetTick()), i2c 
// these have the stm32 file name prefix
// also, they contain the HAL specific 'handles/ports/bases'
// independent code references these: gps, temperature, console, stat, blinky, log

// register level driver (can still use HAL register API)
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
// so, when initialising the clock speed, this is the bit-rate?

// clock stretching, i.e. keep clock low gives devices more time
// e.g. MCU can't generate data when slave wants it 
// offloads error checking in software, as hardware deals with it

// combined format is write+read in a single transaction (provides atomicity; not as important for common single master setup)
// e.g. write register to then read from

// if multiple devices can't handle sample rate, perhaps put devices on separate buses to communicate in parallel

// mock i2c and timers for unit testing? 

// a functional requirement would be I2C specific, while non functional would be code implementation 

// the console commands (along with error counts, i.e. bus statistics) are essential for testability
// we want iterative testing for driver development, i.e. testing as early as possible
// so say, want to be able to trigger a start condition and inspect on logic analyser. then move on, etc.
// interrupts early on too, as they are core to driver functioning

// should include comments as to why not including DMA or doing polling etc?
// DMA is an enhancement to interrupt or separate?

// poll-model or callback?

// when in a state, will have events that occur, e.g. interrupt, timer expiring
// may need a guard timer to ensure not getting stuck in a state

// states for i2c are 'waiting points' in transaction

// could look at linux kernel i2c driver source

// with interrupts, note the software level and interrupt level code
// with interrupts, way to prevent simultaneous data corruption may be through disabling interrupts if not in idle state

// acquire/release() improvements:
//   * queue to ensure fairness
//   * max use timer to release() if not done manually

typedef struct I2C I2C;
struct I2C
{
  // TODO(Ryan): Could update guard timer period at each state, so as to not wait as long
  u32 transaction_guard_time_ms;
  u32 guard_timer_id;

  I2C_TypeDef* i2c_base;
    uint8_t* msg_bfr;
    uint32_t msg_len;
    uint32_t msg_bytes_xferred;

    uint16_t dest_addr;

    bool reserved;
    enum states state;
    enum i2c_errors last_op_error;
    enum states last_op_error_state;
};


enum i2c_errors {
    I2C_ERR_NONE,  // Must have value 0.
    I2C_ERR_INVALID_INSTANCE,
    I2C_ERR_BUS_BUSY,
    I2C_ERR_GUARD_TMR,
    I2C_ERR_PEC,
    I2C_ERR_TIMEOUT,
    I2C_ERR_ACK_FAIL,
    I2C_ERR_BUS_ERR,
    I2C_ERR_INTR_UNEXPECT,
};

enum i2c_u16_pms {
    CNT_RESERVE_FAIL,
    CNT_BUS_BUSY,
    CNT_GUARD_TMR,
    CNT_PEC_ERR,
    CNT_TIMEOUT,
    CNT_ACK_FAIL,
    CNT_BUS_ERR,
    CNT_INTR_UNEXPECT,

    NUM_U16_PMS
};

INTERNAL void
i2c_init(u32 transaction_guard_time_ms, )
{
  global_i2c.transaction_guard_time_ms = transaction_guard_time_ms;
  global_i2c.transaction_guard_timer_id = timer_create(transaction_guard_time_ms, i2c_callback, NULL);

  global_i2c.i2c_base = i2c_base; 

  DISABLE_ALL_I2C_INTERRUPTS();
            evt_irq_type = I2C1_EV_IRQn;
            err_irq_type = I2C1_ER_IRQn;

    NVIC_SetPriority(evt_irq_type,
                     NVIC_EncodePriority(NVIC_GetPriorityGrouping(),0, 0));
    NVIC_EnableIRQ(evt_irq_type);
    NVIC_SetPriority(err_irq_type,
                     NVIC_EncodePriority(NVIC_GetPriorityGrouping(),0, 0));
    NVIC_EnableIRQ(err_irq_type);

#define INTERRUPT_ENABLE_MASK (LL_I2C_CR2_ITEVTEN | LL_I2C_CR2_ITBUFEN | \
                               LL_I2C_CR2_ITERREN)
#define DISABLE_ALL_INTERRUPTS(st) st->i2c_reg_base->CR2 &= \
        ~INTERRUPT_ENABLE_MASK
#define ENABLE_ALL_INTERRUPTS(st) st->i2c_reg_base->CR2 |= \
        INTERRUPT_ENABLE_MASK
}

INTERNAL u32
i2c_write(u32 dest_addr, u8 *msg_buf, u32 msg_len)
{
    return start_op(instance_id, dest_addr, msg_bfr, msg_len,
                    STATE_MSTR_WR_GEN_START);
}

int32_t i2c_read(enum i2c_instance_id instance_id, uint32_t dest_addr,
                 uint8_t* msg_bfr, uint32_t msg_len)
{
    return start_op(instance_id, dest_addr, msg_bfr, msg_len,
                    STATE_MSTR_RD_GEN_START);
}

INTERNAL u32
i2c_op(u32 dst_addr, u8 *msg_buf, u32 msg_len, I2C_OP_TYPE op_type)
{
  LOG_TRACE("i2c_op type=%d msg_len=%d dst_addr=0x%04x\n", op_type, msg_len, dst_addr);

  if (global_i2c.state != I2C_STATE_IDLE)
  {

  }

  if (LL_I2C_IsActiveFlag_BUSY(st->i2c_reg_base)) {
    INC_SAT_U16(cnts_u16[CNT_BUS_BUSY]);
    st->last_op_error = I2C_ERR_BUS_BUSY;
    st->last_op_error_state = STATE_IDLE;
    return MOD_ERR_PERIPH;
  }

  tmr_inst_start(st->guard_tmr_id, 100);

    st->dest_addr = dest_addr;
    st->msg_bfr = msg_bfr;
    st->msg_len = msg_len;
    st->msg_bytes_xferred = 0;

    st->last_op_error = I2C_ERR_NONE;
    st->last_op_error_state = STATE_IDLE;

    st->state = op_type;

    LL_I2C_Enable(st->i2c_reg_base);
    LL_I2C_DisableBitPOS(st->i2c_reg_base);
    LL_I2C_GenerateStartCondition(st->i2c_reg_base);

    ENABLE_ALL_I2C_INTERRUPTS(st);
}


void I2C1_EV_IRQHandler(void)
{
    i2c_interrupt(I2C_INSTANCE_1, INTER_TYPE_EVT, I2C1_EV_IRQn);
}

void I2C1_ER_IRQHandler(void)
{
    i2c_interrupt(I2C_INSTANCE_1, INTER_TYPE_ERR, I2C1_ER_IRQn);
}

// if wanting to add to a state machine:
//   1. add new states
//   2. add flags for existing states

INTERNAL void
i2c_interrupt()
{
    log_trace("i2c_interrupt state=%d xferred=%lu sr1=0x%04x\n", st->state,
              st->msg_bytes_xferred, sr1);

    // for state machines, add code to advance to next state
    // informal state machine uses switch statements?
    // a formal state machine uses tables and forces you to think about all states?
}

// most module requirements are to do background sampling, i.e. interrupt

// typical to get latest sample value, and not actually get sensor value on demand?
// this is because more energy efficient as the sensor can read value and store, then go back to sleep?

// TODO(Ryan): common sensor terms: accuracy, precision, repeatability etc.

// delay after sending command to avoid getting NAK as device does no clock stretching

// look for algorithm that instructs how to interpret raw sensor bytes, i.e. convert signal
// may require reading configuration values from register

// if working with integers, be aware of order of operations in calculations to avoid loss of precision

// if hardware performs CRC, must provide inputs for CRC used? 

// sensor may perform oversampling, i.e. much higher than Nyquist, in order to reduce noise
// increased oversampling will increase output resolution bits

// look at measurement time, so know how long to wait before actually reading result?


// the breakout boards like EEPROM used in dev as has pull-up resistor on it.
// furthermore, will probably have configurable address pins tied to ground
// however, when in production probably use EEPROM in DAP package

// with polling, want to know how long takes to reply to put in timeout 

// with DMA, want to know if EEPROM has finished writing before issuing again
void
i2c_device_test(void)
{
  u32 trial_count = 20;
  u32 trial_timeout_ms = 10;
  HAL_I2C_IsDeviceReady();

  u32 comm_timeout_ms = 10;
  HAL_I2C_Mem_Read();

  u64 cycles_start = CYCLE_COUNT_READ();
  HAL_I2C_Mem_Write();
  u64 cycles_end = CYCLE_COUNT_READ();

  // TODO(Ryan): Use __NOP() as a breakpoint
  
  // TODO(Ryan): Whenever working with I2C device, test the trial and timeout count
  // Often the trial count related to I2C speed, e.g. 10x faster than 10x trial count

  // EEPROM will have page boundary overriding

  // flash has much large page size (so, use if need larger transfers) 
}

void
eeprom_write(void)
{
  if (IsDeviceReady()) Write()/Read();
}
