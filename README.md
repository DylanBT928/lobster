# lobster 🦞

Lobster is a Limit Order Book (LOB) and matching engine written in C++17. It simulates a financial exchange by maintaining price-time priority for buy and sell orders, executing trades when prices cross, and providing a real-time visualization of the order book in the terminal.

![Lobster Demo](demo.gif)


## Architecture


### Data Structures

* **Price Levels (Binary Search Tree):** Price levels are maintained using a BST (`std::map`), which keeps prices naturally sorted. By keeping dedicated pointers tracking the highest bid and lowest ask, finding the best price for matching is an O(1) operation.
* **Time Priority (Doubly Linked List):** Orders at the same price level are stored in a doubly linked list (`std::list`). This ensures strict First-In-First-Out (FIFO) execution while allowing O(1) additions, executions, and removals.
* **Order Lookup (Hash Map):** An `orderID` is mapped directly to its list iterator using a hash map (`std::unordered_map`). This guarantees O(1) order cancellations without needing to search through the book.


### Order Structure

* `orderID` (`uint32_t`): Unique identifier for O(1) lookup.
* `price` (`uint64_t`): The limit price of the order.
* `quantity` (`uint32_t`): Tracks both the initial quantity and the remaining quantity to safely handle partial fills.
* `side` (`Side` enum): Indicates whether the order is a BUY (Bid) or SELL (Ask).


### Core Functions

* `placeOrder(Order& o)`: Submits a new order to the book and attempts to find a match.
* `cancelOrder(uint32_t oid)`: Locates and removes an order in O(1) time.
* `executeTrade()`: Processes crossing orders, updating quantities, and clearing fully filled orders.
* `display()`: Renders the state of the order book to the terminal.


## Build and Run

```sh
g++ -std=c++17 -O3 -Wall -Wextra -Wpedantic -Iinclude src/main.cpp src/limitorderbook.cpp -o lobster
./lobster
```


## License

This project is licensed under the Apache License 2.0. See the [LICENSE](LICENSE) file for details.
