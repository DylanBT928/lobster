#include "limitorderbook.h"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <iomanip>
#include <iostream>

Order::Order(uint16_t oid, uint64_t p, Side s)
{
    orderID = oid;
    price = p;
    side = s;
}

void LimitOrderBook::placeOrder(Order o)
{
    if (o.side == Side::BUY)
    {
        bidSide.push_back(o);
    }
    else
    {
        askSide.push_back(o);
    }
}

void LimitOrderBook::cancelOrder(uint16_t oid)
{
    for (std::size_t i{ 0 }; i < bidSide.size(); ++i)
    {
        if (bidSide[i].orderID == oid)
        {
            bidSide.erase(bidSide.begin() + i);
            break;
        }
    }

    for (std::size_t i{ 0 }; i < askSide.size(); ++i)
    {
        if (askSide[i].orderID == oid)
        {
            askSide.erase(askSide.begin() + i);
            break;
        }
    }
}

void LimitOrderBook::print()
{
    std::size_t len = std::max(bidSide.size(), askSide.size());

    std::cout << "---------------------------------------------------------------" << std::endl;

    for (std::size_t i{ 0 }; i < len; ++i)
    {
        if (i < bidSide.size())
        {
            std::cout << std::left << std::setw(8)
                      << bidSide[i].orderID;
            std::cout << std::right << std::fixed << std::setprecision(4)
                      << std::setw(12) << bidSide[i].price / 100000.0;
            std::cout << "    \033[32mBID\033[0m";
        }
        else
        {
            std::cout << std::setw(27) << ' ';
        }

        std::cout << "    |    ";

        if (i < askSide.size())
        {
            std::cout << std::left << std::setw(8)
                      << askSide[i].orderID;
            std::cout << std::right << std::fixed << std::setprecision(4)
                      << std::setw(12) << askSide[i].price / 100000.0;
            std::cout << "    \033[31mASK\033[0m";
        }

        std::cout << '\n';
    }

    std::cout << "---------------------------------------------------------------" << std::endl;
}
