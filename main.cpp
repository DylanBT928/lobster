#include <random>

#include "limitorderbook.h"

int main()
{
    LimitOrderBook lobster;

    // ---------- begin testing ----------

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> distrib(0, 1);

    for (int i{ 1 }; i <= 10; ++i)
    {
        Side side;

        if (distrib(gen) == 0)
        {
            side = Side::BUY;
        }
        else
        {
            side = Side::SELL;
        }

        Order o{ i, 1005000, side };
        lobster.placeOrder(o);
    }

    lobster.cancelOrder(6);

    // ----------- end testing -----------

    lobster.print();
}
