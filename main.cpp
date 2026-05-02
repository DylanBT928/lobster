#include <cstdint>
#include <random>

#include "limitorderbook.h"

int main()
{
    LimitOrderBook lobster;

    // ---------- begin testing ----------

    std::random_device rd;
    std::mt19937 gen(rd());

    std::uint64_t spy{ 7206500 };
    std::uniform_int_distribution<std::uint64_t> price(spy * 0.9, spy * 1.1 + 1);
    std::uniform_int_distribution<> buyOrSell(0, 1);

    for (std::uint32_t oid{ 1 }; oid < 200000; ++oid)
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

        Order o{ oid, price(gen), side };
        lobster.placeOrder(o);
    }

    lobster.cancelOrder(13);

    // ----------- end testing -----------

    lobster.print();
}
