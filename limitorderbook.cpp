#include "limitorderbook.h"

#include <cstddef>
#include <iomanip>
#include <iostream>

Order::Order(int oid, int p, Side s)
{
    orderID = oid;
    price = p;
    side = s;
}

void LimitOrderBook::placeOrder(Order o)
{
    orders.push_back(o);
}

void LimitOrderBook::cancelOrder(int oid)
{
    for (std::size_t i{ 0 }; i < orders.size(); ++i)
    {
        if (orders[i].orderID == oid)
        {
            orders.erase(orders.begin() + i);
            break;
        }
    }
}

void LimitOrderBook::print()
{
    std::cout << " lobster - limit order book " << std::endl;
    std::cout << "----------------------------" << std::endl;

    for (const Order& o : orders)
    {
        std::cout << o.orderID << '\t';
        std::cout << std::fixed << std::setprecision(4)
                  << o.price / 10000.0 << '\t';

        if (o.side == Side::BUY)
        {
            std::cout << "\033[32mBUY\033[0m\n";
        }
        else
        {
            std::cout << "\033[31mSELL\033[0m\n";
        }
    }
}
