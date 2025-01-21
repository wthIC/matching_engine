# Matching Engine

## Overview
This project implements a simple matching engine designed to match buy and sell orders based on price and quantity. It includes:
- A server that handles the matching logic.
- A client to generate test data and send orders to the server.

The engine supports various scenarios, including edge cases, and allows for easy testing using predefined test case files or random data generation.

## Features
- Supports buy and sell order matching.
- Handles partial fills and edge cases like no matching prices.
- Includes a data generator script for:
  - Random order generation.
  - Predefined test case execution.
- Easily customizable order parameters and test cases.

## Test Cases
The project includes the following test cases:
1. `test.txt`: One seller, 100 buyers.
2. `test2.txt`: No matching prices, no match.
3. `test3.txt`: Order `i` and order `i+1` always match.
4. `test4.txt`: Randomly generated but finite orders.

### Adding Test Cases
- Test case format: `x,y,z`
  - `x`: Order type (0 for sell, 1 for buy).
  - `y`: Price (positive integer).
  - `z`: Quantity (positive integer).
- Add new test cases following this format, and specify the test file in the data generator.

## Order Structure
Each order has the following attributes:
- `OrderID`: Positive integer.
- `Type`: 0 (Sell) or 1 (Buy).
- `Price`: Positive integer (1–100 by default).
- `Quantity`: Positive integer (1–50 by default).

Limits can be modified in the data generator script.

## Running the Project

### Running the Server
1. Compile the server: g++ matching_engine.cpp -o matching_engine.out
2. Run the server: ./matching_engine.out [print flag]
Replace `[print flag]` with `true` to enable message logging or `false` to disable it.

### Running the Client
1. Compile the data generator: g++ data_generator.cpp -o data_generator.out -std=c++11 -pthread
2. Run the client: ./data_generator.out [random flag]
Replace `[random flag]` with:
- `true`: Generate random orders indefinitely.
- `false`: Use a predefined test case file (default: `test.txt`).  
  You can modify the file name in the data generator (line 81).

## Output
When a match or partial fill is found, the server sends a message to the client in the following format:
Match found - Buyer: x, Seller: y, Price: p_xy, Quantity: q_xy

## Customization
- Adjust order parameter limits (price, quantity) in the data generator script.
- Add or modify test cases to explore new scenarios.

# Matching Engine
