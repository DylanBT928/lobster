#pragma once

#include "Order.hpp"

class OrderModify
{
   public:
    OrderModify(OrderID orderID, Side side, Price price, Quantity quantity)
        : orderID_{ orderID },
          side_{ side },
          price_{ price },
          quantity_{ quantity }
    {
    }

    OrderID GetOrderID() const
    {
        return orderID_;
    }

    Side GetSide() const
    {
        return side_;
    }

    Price GetPrice() const
    {
        return price_;
    }

    Quantity GetQuantity() const
    {
        return quantity_;
    }

    OrderPointer ToOrderPointer(OrderType orderType) const
    {
        return std::make_shared<Order>(orderType, GetOrderID(), GetSide(), GetPrice(), GetQuantity());
    }

   private:
    OrderID orderID_;
    Side side_;
    Price price_;
    Quantity quantity_;
};
