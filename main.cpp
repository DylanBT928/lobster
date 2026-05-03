#include <chrono>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <random>
#include <thread>
#include <vector>

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

    std::vector<Order> orders;

    for (std::uint32_t oid{ 1 }; oid <= 20; ++oid)
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
        orders.push_back(o);
    }

    std::cout << "\033[2J\033[1;1H";

    for (std::size_t i{ 0 }; i < orders.size(); ++i)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        lobster.placeOrder(orders[i]);
        lobster.display();
        std::cout << "\033[2J\033[1;1H";
    }

    lobster.cancelOrder(13);

    // ----------- end testing -----------

    lobster.display();
}
