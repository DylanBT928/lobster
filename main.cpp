#include <cstdint>
#include <random>

#include "limitorderbook.h"

int main()
{
    LimitOrderBook lobster;

    // ---------- begin testing ----------

    std::random_device rd;
    std::mt19937 gen(rd());

    std::uniform_int_distribution<uint64_t> price(100, 9999999999);
    std::uniform_int_distribution<> buyOrSell(0, 1);

    for (std::uint16_t oid{ 1 }; oid <= 20; ++oid)
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
