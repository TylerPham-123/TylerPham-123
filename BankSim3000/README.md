# BankSim3000 Simulation

## Overview
BankSim3000 is a bank and teller simulation designed to assist bank managers in making informed decisions about the number of tellers to hire based on customer wait times. The simulation models customer arrivals, teller availability, and transaction processing.

## Purpose
The primary goal of this project is to simulate a bank environment where customers arrive at varying times and require service from tellers. By analyzing the simulation results, bank managers can determine optimal staffing levels to minimize customer wait times.

## Files
- **src/main.cpp**: Contains the main implementation of the BankSim3000 simulation. It includes necessary headers, defines the main function, and runs the simulation with predefined input.
- **CMakeLists.txt**: Configuration file for CMake, specifying the project name, required C++ standard, and source files to compile.
- **README.md**: Documentation for the project, explaining its purpose, how to build and run the simulation, and other relevant information.

## Building the Project
To build the project, follow these steps:
1. Ensure you have CMake installed on your system.
2. Open a terminal and navigate to the root directory of the project.
3. Create a build directory:
   ```
   mkdir build
   cd build
   ```
4. Run CMake to configure the project:
   ```
   cmake ..
   ```
5. Compile the project:
   ```
   make
   ```

## Running the Simulation
After building the project, you can run the simulation by executing the generated binary:
```
./BankSim3000
```

## Input
The simulation uses predefined input for customer arrivals and transaction times. You can modify the input in the `src/main.cpp` file as needed.

## License
This project is licensed under the MIT License. See the LICENSE file for more details.