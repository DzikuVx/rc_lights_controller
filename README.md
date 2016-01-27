Lights controller for RC planes, drones, quadcopters driven by RC PWM signal

RC Lights controller can work in following modes:

* ***No RC input*** - in this mode, light pattern is set by pressing a button. There are 6 preprogrammed patterns that can be used without PWM input. Selected mode is stored in EEPROM memory and will be automatically selected on next power up,
* ***Single RC PWM input*** - in this mode, lights pattern is determined by RC signal on INPUT_1. 2 and 3 position switches are supported. By default:
    * with 2 position switch *RC Lights Controller* works like on-off switch for OUTPUT_1 and OUTPUT_2. OUTPUT_3 is not used
    * with 3 position switch, OUTPUT_1 and OUTPUT_2 works as: OFF, BLINK, ON. OUTPUT_3 is not used  
* ***Two RC inputs*** - in this mode, total of 9 light modes can be selected using two 3 position switches. Two 2 position switches allows to choose one from 4 light modes. By default:
    * ***INPUT_1*** works like master switch providing the same "modes" like in ***Single RC PWM Input***
    * ***INPUT_2*** works like subprogram selector 
    * ***OUTPUT_3*** is enabled only when ***INPUT_1*** is in ***HIGH*** state
    
# Modes

# OUTPUT_1 and OUTPUT_2
    
|               | INPUT_2 LOW or not connected  | INPUT_2 MID   | INPUT_2 HIGH  | 
|---            |---                            |---            |---            |
| INPUT_1 LOW   | OFF                           |   OFF         | OFF           |
| INPUT_1 MID   | BLINK 1                       | BLINK 2       | BLINK 3       |
| INPUT_1 HIGH  | ON                            | ON            | OFF           |

# OUTPUT_3

