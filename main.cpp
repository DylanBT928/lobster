#include <chrono>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <random>
#include <thread>

#include "limitorderbook.h"

int main()
{
    LimitOrderBook lobster;

    // ---------- begin testing ----------

    std::random_device rd;
    std::mt19937 gen(rd());

    std::uint64_t spy{ 7206500 };
    std::uniform_int_distribution<std::uint64_t> price(spy * 0.9, spy * 1.1 + 1);
    std::uniform_int_distribution<std::uint8_t> quantity(1, 9);
    std::uniform_int_distribution<> buyOrSell(0, 1);

    std::list<Order> orders;

    for (std::uint32_t oid{ 1 }; oid <= 50; ++oid)
    {
        Side side;

        if (buyOrSell(gen) == 0)
        {
            side = Side::BUY;
        }
        else
        {
            side = Side::SELL;
        }

        Order o{ oid, price(gen), quantity(gen), side };
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        lobster.placeOrder(o);
        lobster.display();
    }

    lobster.cancelOrder(1);

    // ----------- end testing -----------

    lobster.display();
}
