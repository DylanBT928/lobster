#pragma once

#include "Order.hpp"
#include "OrderbookLevelInfos.hpp"
#include "OrderModify.hpp"
#include "Trade.hpp"
#include "types.hpp"

#include <map>
#include <unordered_map>

class Orderbook
{
   public:
    Trades AddOrder(OrderPointer order);
    void CancelOrder(OrderID orderID);
    Trades ModifyOrder(OrderModify order);

    std::size_t Size() const;
    OrderbookLevelInfos GetOrderInfos() const;

   private:
    struct OrderEntry
    {
        OrderPointer order_{ nullptr };
        OrderPointers::iterator location_;
    };

    struct LevelData
    {
        Quantity quantity_;
        Quantity count_;

        enum class Action
        {
            Add,
            Remove,
            Match
        };
    };

    std::map<Price, OrderPointers, std::greater<Price>> bids_;
    std::map<Price, OrderPointers, std::less<Price>> asks_;
    std::unordered_map<OrderID, OrderEntry> orders_;

    bool CanMatch(Side side, Price price) const;
    Trades MatchOrder();
};
