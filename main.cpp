#include <iostream>
#include <vector>

class LimitOrderBook
{
   public:
    void print()
    {
        std::cout << "lobster - limit order book" << std::endl;
        std::cout << "--------------------------" << std::endl;
    }
};

int main()
{
    LimitOrderBook lobster;
    lobster.print();
}
