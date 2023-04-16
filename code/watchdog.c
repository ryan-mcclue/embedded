// SPDX-License-Identifier: zlib-acknowledgement

// should keep even in debug builds, as always important to identify system hangs

// reset info registers typically 'sticky', i.e. have to clear them otherwise won't update

// must check defaults regarding when watchdog is stopped, e.g. default is to freeze on breakpoints and CPU sleep?
// performing cleanup in watchdog reset ISR is often error-prone due to short amount of cycles available

// timeout value 5-30seconds, so 10seconds
// some have various 'reload request registers' that all have to be 'petted'; however safer to implement in software using one as more common

// could hang if waiting for a polling transaction to complete
// IMPORTANT(Ryan): only gracefully handle an error if it's likely to occur regularly; otherwise watchdog reset

// software watchdog would be repurposing a general purpose timer peripheral

