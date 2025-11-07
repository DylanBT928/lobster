#include "../include/Orderbook.hpp"

#include <iostream>

int main()
{
    Orderbook orderbook;

    const OrderID orderID = 1;
    orderbook.AddOrder(std::make_shared<Order>(OrderType::GoodTillCancel, orderID, Side::Buy, 100, 10));
    std::cout << orderbook.Size() << std::endl;  // 1
    orderbook.CancelOrder(orderID);
    std::cout << orderbook.Size() << std::endl;  // 0

    return 0;
}
