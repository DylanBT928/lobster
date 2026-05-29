#ifndef LIMITORDERBOOK_H
#define LIMITORDERBOOK_H

#include <cstdint>
#include <list>
#include <map>
#include <unordered_map>

/*
 *  enum Side - Represents the side of the market
 *  @BUY: Indicates a bid order
 *  @ASK: Indicates an ask order
 */
enum class Side
{
    BUY,
    SELL
};

/*
 *  struct Order - Represents a single limit order
 *  @orderID:           Unique identifier of the order
 *  @price:             Limit price for execution
 *  @initialQuantity:   Starting volume of the order
 *  @remainingQuantity: Remaining unfilled volume of the order
 *  @side:              Market side (BUY or SELL)
 */
struct Order
{
    Order(std::uint32_t oid, std::uint64_t p, std::uint8_t iq, Side s);
    std::uint32_t orderID;
    std::uint64_t price;
    std::uint8_t initialQuantity;
    std::uint8_t remainingQuantity;
    Side side;
};

/*
 *  class LimitOrderBook - Core matching engine structure
 *  @orderIDs: Hashmap for O(1) order lookups and cancellations
 *  @bids:     Buy orders sorted by descending price (highest bid first)
 *  @asks:     Sell orders sorted by ascending price (lowest ask first)
 *  @maxBid:   Pointer to the current highest bid order
 *  @maxAsk:   Pointer to the current lowest ask order
 *
 *  Maintains the state of all open bids and asks, including an interface to
 *  place, cancel, and execute trades based on price-time priority.
 */
class LimitOrderBook
{
   public:
    void placeOrder(const Order& o);
    void cancelOrder(std::uint32_t oid);
    void executeTrade();
    void display() const;

   private:
    std::unordered_map<std::uint32_t, std::list<Order>::iterator> orderIDs;
    std::map<std::uint64_t, std::list<Order>, std::greater<std::uint64_t>> bids;
    std::map<std::uint64_t, std::list<Order>> asks;
    Order* maxBid;
    Order* minAsk;
};

#endif /* LIMITORDERBOOK_H */
