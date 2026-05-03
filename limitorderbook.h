#ifndef LIMITORDERBOOK_H
#define LIMITORDERBOOK_H

#include <cstdint>
#include <vector>

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
    std::vector<Order> bidSide;
    std::vector<Order> askSide;
    Order* maxBid;
    Order* minAsk;
};

#endif
