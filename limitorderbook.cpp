#include "limitorderbook.h"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <iomanip>
#include <iostream>

Order::Order(std::uint32_t oid, std::uint64_t p, std::uint8_t iq, Side s)
    : orderID(oid),
      price{ p },
      initialQuantity{ iq },
      remainingQuantity{ iq },
      side{ s }
{
}

LimitOrderBook::LimitOrderBook()
    : maxBid{ nullptr }, minAsk{ nullptr }
{
}

LimitOrderBook::~LimitOrderBook()
{
    maxBid = nullptr;
    minAsk = nullptr;
}

void LimitOrderBook::placeOrder(Order& o)
{
    if (o.side == Side::BUY)
    {
        bidSide.push_back(o);

        if (!maxBid)
        {
            maxBid = &o;
        }

        if (o.price > maxBid->price)
        {
            maxBid = &o;
        }
    }
    else
    {
        askSide.push_back(o);

        if (!minAsk)
        {
            minAsk = &o;
        }

        if (o.price < minAsk->price)
        {
            minAsk = &o;
        }
    }

    if (maxBid && minAsk)
    {
        if (maxBid->price >= minAsk->price)
        {
            executeTrade();
        }
    }
}

void LimitOrderBook::cancelOrder(std::uint32_t oid)
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

void LimitOrderBook::executeTrade()
{
    for (std::size_t i{ 0 }; i < bidSide.size(); ++i)
    {
        if (bidSide[i].orderID == maxBid->orderID)
        {
            bidSide.erase(bidSide.begin() + i);
            break;
        }
    }

    for (std::size_t i{ 0 }; i < askSide.size(); ++i)
    {
        if (askSide[i].orderID == minAsk->orderID)
        {
            askSide.erase(askSide.begin() + i);
            break;
        }
    }

    sort(bidSide.begin(), bidSide.end(),
         [](const Order& a, const Order& b)
         {
             return a.price > b.price;
         });
    sort(askSide.begin(), askSide.end(),
         [](const Order& a, const Order& b)
         {
             return a.price < b.price;
         });

    maxBid = &bidSide[0];
    minAsk = &askSide[0];
}

void LimitOrderBook::display()
{
    std::size_t len = std::max(bidSide.size(), askSide.size());
    std::size_t dashes{ 75 };

    std::cout << "\033[2J\033[1;1H";

    for (std::size_t i{ 0 }; i < dashes; ++i)
    {
        std::cout << '-';
    }

    std::cout << '\n';

    for (std::size_t i{ 0 }; i < len; ++i)
    {
        if (i < bidSide.size())
        {
            std::cout << std::left << std::setw(8)
                      << bidSide[i].orderID;
            std::cout << "\033[32m" << std::right << std::fixed
                      << std::setprecision(4) << std::setw(15)
                      << bidSide[i].price / 10000.0 << "\033[0m";
            std::cout << "    " << +bidSide[i].initialQuantity
                      << "    " << +bidSide[i].remainingQuantity;
        }
        else
        {
            std::cout << std::setw(33) << ' ';
        }

        std::cout << "    |    ";

        if (i < askSide.size())
        {
            std::cout << std::left << std::setw(8)
                      << askSide[i].orderID;
            std::cout << "\033[31m" << std::right << std::fixed
                      << std::setprecision(4) << std::setw(15)
                      << askSide[i].price / 10000.0 << "\033[0m";
            std::cout << "    " << +askSide[i].initialQuantity
                      << "    " << +askSide[i].remainingQuantity;
        }

        std::cout << '\n';
    }

    for (std::size_t i{ 0 }; i < dashes; ++i)
    {
        std::cout << '-';
    }

    std::cout << '\n';

    if (maxBid && minAsk)
    {
        std::cout << '\n';
        std::cout << "max bid: " << maxBid->price << ' ' << '\n';
        std::cout << "min ask: " << minAsk->price << ' ' << '\n';
        std::cout << '\n';
    }
}
