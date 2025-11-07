#pragma once

#include "OrderType.hpp"
#include "Side.hpp"
#include "types.hpp"

#include <exception>
#include <format>
#include <list>

class Order
{
   public:
    Order(OrderType orderType, OrderID orderID, Side side, Price price, Quantity quantity)
        : orderType_{ orderType },
          orderID_{ orderID },
          side_{ side },
          price_{ price },
          initialQuantity_{ quantity },
          remainingQuantity_{ quantity }
    {
    }

    OrderType GetOrderType() const
    {
        return orderType_;
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

    Quantity GetInitialQuantity() const
    {
        return initialQuantity_;
    }

    Quantity GetRemainingQuantity() const
    {
        return remainingQuantity_;
    }

    Quantity GetFilledQuantity() const
    {
        return GetInitialQuantity() - GetRemainingQuantity();
    }

    bool IsFilled() const
    {
        return GetRemainingQuantity() == 0;
    }

    void Fill(Quantity quantity)
    {
        if (quantity > GetRemainingQuantity())
        {
            throw std::logic_error(std::format("Order ({}) cannot be filled for more than its remaining quantity.", GetOrderID()));
        }

        remainingQuantity_ -= quantity;
    }

   private:
    OrderType orderType_;
    OrderID orderID_;
    Side side_;
    Price price_;
    Quantity initialQuantity_;
    Quantity remainingQuantity_;
};

using OrderPointer = std::shared_ptr<Order>;
using OrderPointers = std::list<OrderPointer>;
