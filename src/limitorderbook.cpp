#include "limitorderbook.hpp"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <vector>

/**
 *  Order::Order - Constructor for a new order
 *  @oid: Order ID
 *  @p:   Price
 *  @iq:  Initial quantity
 *  @s:   Side (BUY / SELL)
 */
Order::Order(std::uint32_t oid, std::uint64_t p, std::uint8_t iq, Side s)
    : orderID(oid),
      price{ p },
      initialQuantity{ iq },
      remainingQuantity{ iq },
      side{ s }
{
}

/**
 *  LimitOrderBook::placeOrder - Inserts a new order and triggers matching
 *  @o: Order to place
 *
 *  Routes the order to the appropriate hashmap (bids or asks) based on
 *  its side, updates the orderID hashmap for O(1) lookups, and updates
 *  the maxBid / minAsk pointers.
 *
 *  Continuously loops and calls LimitOrderBook::executeTrade() as long as
 *  the spread is crossed (maxBid >= minAsk).
 */
void LimitOrderBook::placeOrder(const Order& o)
{
    if (o.side == Side::BUY)
    {
        bids[o.price].push_back(o);
        orderIDs.insert({ o.orderID, std::prev(bids[o.price].end()) });
        maxBid = &bids.begin()->second.front();
    }
    else
    {
        asks[o.price].push_back(o);
        orderIDs.insert({ o.orderID, std::prev(asks[o.price].end()) });
        minAsk = &asks.begin()->second.front();
    }

    while (maxBid && minAsk && !bids.empty() && !asks.empty())
    {
        if (maxBid->price >= minAsk->price)
        {
            executeTrade();

            if (!bids.empty() && !asks.empty())
            {
                maxBid = &bids.begin()->second.front();
                minAsk = &asks.begin()->second.front();
            }
            else
            {
                maxBid = nullptr;
                minAsk = nullptr;
            }
        }
        else
        {
            break;
        }
    }
}

/**
 *  LimitOrderBook::cancelOrder - Removes an order from the order book
 *  @oid: Order ID of the order to cancel
 *
 *  Uses the orderIDs hashmap to locate the iterator and removes it from
 *  the corresponding bid / ask list, cleaning up empty price levels if
 *  necessary.
 */
void LimitOrderBook::cancelOrder(std::uint32_t oid)
{
    auto mapIt = orderIDs.find(oid);

    if (mapIt == orderIDs.end())
    {
        return;
    }

    auto listIt = mapIt->second;
    std::uint64_t price = listIt->price;

    if (listIt->side == Side::BUY)
    {
        bids[price].erase(listIt);

        if (bids[price].empty())
        {
            bids.erase(price);
        }
    }
    else
    {
        asks[price].erase(listIt);

        if (asks[price].empty())
        {
            asks.erase(price);
        }
    }

    orderIDs.erase(mapIt);
}

/**
 *  LimitOrderBook::executeTrade - Matches currently crossed orders
 *
 *  Matches the best bid and best ask. Calculates the maximum fill volume,
 *  decrements the remaining quantities, and automatically removes fully
 *  depleted orders via LimitOrderBook::cancelOrder(). Assumes prices are
 *  already crossed.
 */
void LimitOrderBook::executeTrade()
{
    std::uint8_t fillQuantity = std::min(maxBid->remainingQuantity,
                                         minAsk->remainingQuantity);

    maxBid->remainingQuantity -= fillQuantity;
    minAsk->remainingQuantity -= fillQuantity;

    if (maxBid->remainingQuantity == 0)
    {
        cancelOrder(maxBid->orderID);
    }

    if (minAsk->remainingQuantity == 0)
    {
        cancelOrder(minAsk->orderID);
    }
}

/**
 *  LimitOrderBook::display - Prints the current state of the order book
 *
 *  Outputs the aggregated levels of asks and bids to stdout, alongside
 *  the best bid, best ask, and current spread. Includes ANSI color codes
 *  for visual distinction.
 */
void LimitOrderBook::display() const
{
    std::vector<const Order*> flatBids;

    for (auto& [price, orders] : bids)
    {
        for (const auto& o : orders)
        {
            flatBids.push_back(&o);
        }
    }

    std::vector<const Order*> flatAsks;

    for (auto& [price, orders] : asks)
    {
        for (const auto& o : orders)
        {
            flatAsks.push_back(&o);
        }
    }

    std::size_t len = std::max(flatBids.size(), flatAsks.size());
    std::size_t dashes{ 73 };
    std::size_t height{ 20 };

    std::cout << "\033[2J\033[1;1H";

    for (std::size_t i{ 0 }; i < dashes; ++i)
    {
        std::cout << '-';
    }

    std::cout << '\n';

    for (std::size_t i{ 0 }; i < len; ++i)
    {
        if (i < flatBids.size())
        {
            const auto* b = flatBids[i];

            std::cout << std::left << std::setw(8) << b->orderID;
            std::cout << "\033[32m" << std::right << std::fixed << std::setprecision(4)
                      << std::setw(14) << b->price / 10000.0 << "\033[0m";
            std::cout << std::setw(5) << +b->initialQuantity
                      << std::setw(5) << +b->remainingQuantity;
        }
        else
        {
            std::cout << std::setw(32) << ' ';
        }

        std::cout << "    |    ";

        if (i < flatAsks.size())
        {
            const auto* a = flatAsks[i];

            std::cout << std::left << std::setw(8) << a->orderID;
            std::cout << "\033[31m" << std::right << std::fixed << std::setprecision(4)
                      << std::setw(14) << a->price / 10000.0 << "\033[0m";
            std::cout << std::setw(5) << +a->initialQuantity
                      << std::setw(5) << +a->remainingQuantity;
        }

        std::cout << '\n';

        if (--height <= 0)
        {
            break;
        }
    }

    for (std::size_t i{ 0 }; i < height; ++i)
    {
        std::cout << std::setw(32) << ' ' << "    |    \n";
    }

    for (std::size_t i{ 0 }; i < dashes; ++i)
    {
        std::cout << '-';
    }

    std::cout << '\n';

    std::cout << "best bid: ";

    if (maxBid)
    {
        std::cout << std::fixed << std::setprecision(4) << maxBid->price / 10000.0;
    }
    else
    {
        std::cout << "n/a";
    }

    std::cout << " | best ask: ";

    if (minAsk)
    {
        std::cout << std::fixed << std::setprecision(4) << minAsk->price / 10000.0;
    }
    else
    {
        std::cout << "n/a";
    }

    std::cout << " | spread: ";

    if (maxBid && minAsk)
    {
        std::cout << (minAsk->price - maxBid->price) / 10000.0 << "\n";
    }
    else
    {
        std::cout << "n/a\n";
    }
}
