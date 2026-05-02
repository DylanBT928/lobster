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
    Order(uint32_t oid, uint64_t p, Side s);
    uint32_t orderID;
    uint64_t price;
    Side side;
};

class LimitOrderBook
{
   public:
    void placeOrder(Order o);
    void cancelOrder(uint32_t oid);
    void executeTrade();
    void display();

   private:
    std::vector<Order> bidSide;
    std::vector<Order> askSide;
};

#endif
