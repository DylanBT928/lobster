#ifndef LIMITORDERBOOK_H
#define LIMITORDERBOOK_H

#include <vector>

enum class Side
{
    BUY,
    SELL
};

class Order
{
   public:
    Order(int oid, int p, Side s);

    int orderID;
    int price;
    Side side;
};

class LimitOrderBook
{
   public:
    void placeOrder(Order o);
    void cancelOrder(int oid);
    void print();

   private:
    std::vector<Order> orders;
};

#endif
