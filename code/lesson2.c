// SPDX-License-Identifier: zlib-acknowledgement

/* Useful serial oscilloscope tool, windows: https://x-io.co.uk/serial-oscilloscope/  
 * Software visualisation is for post-processing. Want it to run in the sleep duty cycle (which is often 90% in battery powered devices) so as to not affect system timing
 * Design more pre-planning for embedded. (notably hardware block diagrams. so, important to know what protocols used for what common peripherals)
 * This is because hardware is involved 
 * However, diagrams are just for your own understanding (so software is not useful)
 * https://lucid.app/lucidchart/4877b417-3928-4946-93e2-d6ea91f1451f/edit?beaconFlowId=D87AD983CE11B92D&invitationId=inv_ef9f17ee-abfa-42e3-9069-208d3af34b56&page=0_0#
 * plantuml.com for code generating diagrams
 * State machines seemed to be used frequently as a way to architect this
 *
 * platform(PowerInfo power_info, UserCommands user_commands)
 * {
 *    // handle power
 *    // handle commands
 *    // perform autonomous commands, i.e. system_control(), e.g. maintain stability
 *    // DEBUG CODE WILL BE REQUIRED TO OPERATE IN PARTICULAR CPU MODE
 * }
 *
 * During dev kit phase, creating a debugging toolchain and 
 * testing framework (more tests for more 'riskier' parts) is essential
 * Have standard unit/integration tests and POST (power-on-self-tests) which run every time on board power-up
 * (probably command console also)
 *
 * Is more waterfall (in reality a spiral waterfall, i.e ability to go back) as we have to deal with hardware, and can't magically get new boards every 2 weeks like in agile.
 * (so, maybe agile for software, but waterfall for the project as a whole?)
 * component selection is capabilities and then what environment for these components
 * dev prototype is basically stm32 discovery board and breadboard sensors
 * actual product will progress from a layout to a gerber file to pcb. once is this state, that is when alpha/beta etc. releases are done  
 * pulling apart drone controller revealed potassium carbonate from the potassium hydroxide (battery alkaline)
 *
 * Mbed is a type of board and compiler for that board. STM32 is a line of MCUs from ST.
 * ST make mbed compatible boards, e.g. nucleo 
 *
 * ifixit.com teardowns
 */
