# Man from U.A.R.T.

## Overview
The "Man from U.A.R.T." project is a simple embedded application developed for STM32 microcontrollers. It demonstrates UART communication, providing a framework to interact between two UART interfaces, process received messages, and transmit responses. This project can serve as a foundation for applications involving UART communication.

## Features
- **UART Communication:** Transmit and receive messages using multiple UART peripherals.
- **Interactive Command Processing:** Accepts commands via UART and performs appropriate actions.
- **Dynamic Message Handling:** Processes and formats received messages for better usability.
- **Multiple UART Support:** Configurable for UART1 or UART2.
- **Graceful Exit:** Recognizes specific commands to terminate the application.

## How It Works
1. **Setup and Initialization:**
   - The program initializes UART peripherals (`UART1`, `UART2`, `UART3`) and configures the selected UART (`uart_no`) for communication.
2. **Receiving Data:**
   - The program uses interrupt-based UART reception (`HAL_UART_Receive_IT`) for asynchronous data handling.
3. **Processing Commands:**
   - Commands received via UART are analyzed and executed. For instance:
     - `q` or `Q`: Quit the program.
     - Specific strings trigger corresponding responses.
4. **Transmitting Data:**
   - The program uses `HAL_UART_Transmit` to send responses or formatted messages to the selected UART.

## File Structure
- **main.c:** Contains the core logic for UART communication and message processing.
- **LICENSE:** (if provided) Contains the licensing details for the project.

## Configuration
- Modify the `uart_no` variable to select the UART interface (e.g., `1` for UART1 or `2` for UART2).
- Ensure appropriate initialization functions are called for the selected UART (e.g., `MX_USART1_UART_Init`, `MX_USART2_UART_Init`).

## Key Functions
- `HAL_UART_RxCpltCallback`: Callback function invoked when UART receives data.
- `formatStr`: Formats received strings, handles new connections, and processes commands.
- `title_line`: Displays a prompt for the user to input their name or commands.
- `main`: Main program entry point, containing the initialization and main loop.

## Example Usage
1. Configure the UART number by setting the `uart_no` variable.
2. Build and flash the program onto the STM32 microcontroller.
3. Open a serial terminal connected to the selected UART.
4. Interact with the program by sending commands or messages.
5. Exit the program by sending the `q` command.

## Build and Flash Instructions
1. Open the project in your preferred STM32 development environment (e.g., STM32CubeIDE).
2. Ensure all dependencies are configured (e.g., HAL libraries for UART).
3. Compile the project.
4. Flash the compiled binary to the STM32 microcontroller.

## Requirements
- **Hardware:** STM32 microcontroller with at least three UART peripherals.
- **Software:** STM32CubeIDE or another development environment supporting STM32.
- **Dependencies:** STM32 HAL libraries for UART and GPIO.

## Notes
- The program assumes proper hardware connections between UART peripherals and external devices.
- The UART3 interface is used for debugging and interacting with the program.
- Messages sent and received via UART are limited to predefined buffer sizes.

## License
This project is licensed under terms that can be found in the LICENSE file in the root directory. If no LICENSE file is provided, the software is provided "AS-IS."

## Acknowledgments
- Developed using STM32 HAL libraries.
- Inspired by practical needs for UART-based communication in embedded systems.

