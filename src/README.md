# Custom Low-Latency Trading System Simulation

## Overview

This project simulates a custom low-latency trading system designed for order matching and execution. It leverages OpenCL for accelerated order matching, making it suitable for high-frequency trading applications.

## Features

- **Order Book**: Maintains buy and sell orders in a sorted manner.
- **Order Matching Engine**: Matches buy and sell orders with minimal latency using OpenCL.
- **Market Data Simulation**: Simulates market data to test order submissions and matches.
- **Concurrency**: Implements a concurrent model to handle multiple orders simultaneously.

## Technologies Used

- C++
- OpenCL
- CMake
- Concurrency

## Requirements

- macOS with Homebrew installed
- CMake
- OpenCL

### Building the Project

mkdir build
cd build
cmake ..
make

### Running the Project

./trading_system
