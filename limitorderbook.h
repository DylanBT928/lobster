#ifndef LIMITORDERBOOK_H
#define LIMITORDERBOOK_H

#include <cstdint>
#include <list>
#include <map>
#include <unordered_map>

enum class Side
{
    BUY,
    SELL
};

struct Order
{
    Order(std::uint32_t oid, std::uint64_t p, std::uint8_t iq, Side s);
    std::uint32_t orderID;
    std::uint64_t price;
    std::uint8_t initialQuantity;
    std::uint8_t remainingQuantity;
    Side side;
    Order* prev;
    Order* next;
};

class LimitOrderBook
{
   public:
    LimitOrderBook();
    ~LimitOrderBook();

    void placeOrder(Order& o);
    void cancelOrder(std::uint32_t oid);
    void executeTrade();
    void display();

   private:
    std::unordered_map<std::uint32_t, std::list<Order>::iterator> orderIDs;
    std::map<std::uint64_t, std::list<Order>> bids;
    std::map<std::uint64_t, std::list<Order>, std::greater<std::uint64_t>> asks;
    Order* maxBid;
    Order* minAsk;
};

#endif
