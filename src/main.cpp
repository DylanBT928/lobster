#include <algorithm>
#include <cmath>
#include <ctime>
#include <deque>
#include <format>
#include <iostream>
#include <limits>
#include <list>
#include <map>
#include <memory>
#include <numeric>
#include <optional>
#include <queue>
#include <set>
#include <stack>
#include <string>
#include <tuple>
#include <unordered_map>
#include <variant>
#include <vector>

enum class OrderType
{
    GoodTillCancel,
    FillAndKill
};

enum class Side
{
    Buy,
    Sell
};

using Price = std::int32_t;
using Quantity = std::uint32_t;
using OrderID = std::uint64_t;

struct LevelInfo
{
    Price price_;
    Quantity quantity_;
};

using LevelInfos = std::vector<LevelInfo>;

class OrderbookLevelInfos
{
   public:
    OrderbookLevelInfos(const LevelInfos& bids, const LevelInfos& asks)
        : bids_{ bids },
          asks_{ asks }
    {
    }

    const LevelInfos& GetBids() const
    {
        return bids_;
    }

    const LevelInfos& GetAsks() const
    {
        return asks_;
    }

   private:
    LevelInfos bids_;
    LevelInfos asks_;
};

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

struct TradeInfo
{
    OrderID orderID_;
    Price price_;
    Quantity quantity_;
};

class Trade
{
   public:
    Trade(const TradeInfo& bidTrade, const TradeInfo& askTrade)
        : bidTrade_{ bidTrade },
          askTrade_{ askTrade }
    {
    }

    const TradeInfo& GetBidTrade() const
    {
        return bidTrade_;
    }

    const TradeInfo& GetAskTrade() const
    {
        return askTrade_;
    }

   private:
    TradeInfo bidTrade_;
    TradeInfo askTrade_;
};

using Trades = std::vector<Trade>;

class Orderbook
{
   public:
    Trades AddOrder(OrderPointer order)
    {
        if (orders_.contains(order->GetOrderID()))
        {
            return {};
        }

        if (order->GetOrderType() == OrderType::FillAndKill && CanMatch(order->GetSide(), order->GetPrice()) == false)
        {
            return {};
        }

        OrderPointers::iterator iterator;

        if (order->GetSide() == Side::Buy)
        {
            auto& orders = bids_[order->GetPrice()];
            orders.push_back(order);
            iterator = std::next(orders.begin(), orders.size() - 1);
        }
        else
        {
            auto& orders = asks_[order->GetPrice()];
            orders.push_back(order);
            iterator = std::next(orders.begin(), orders.size() - 1);
        }

        orders_.insert({ order->GetOrderID(), OrderEntry{ order, iterator } });
        return MatchOrder();
    }

    void CancelOrder(OrderID orderID)
    {
        if (!orders_.contains(orderID))
        {
            return;
        }

        const auto& [order, iterator] = orders_.at(orderID);

        if (order->GetSide() == Side::Sell)
        {
            auto price = order->GetPrice();
            auto& orders = asks_.at(price);
            orders.erase(iterator);

            if (orders.empty())
            {
                asks_.erase(price);
            }
        }
        else
        {
            auto price = order->GetPrice();
            auto& orders = bids_.at(price);
            orders.erase(iterator);

            if (orders.empty())
            {
                bids_.erase(price);
            }
        }

        orders_.erase(orderID);
    }

    Trades MatchOrder(OrderModify order)
    {
        if (orders_.contains(order.GetOrderID()) == false)
        {
            return {};
        }

        const auto& [existingOrder, _] = orders_.at(order.GetOrderID());
        CancelOrder(order.GetOrderID());
        return AddOrder(order.ToOrderPointer(existingOrder->GetOrderType()));
    }

    std::size_t Size() const
    {
        return orders_.size();
    }

    OrderbookLevelInfos GetOrderInfos() const
    {
        LevelInfos bidInfos;
        LevelInfos askInfos;

        bidInfos.reserve(orders_.size());
        askInfos.reserve(orders_.size());

        auto CreateLevelInfos = [](Price price, const OrderPointers& orders)
        {
            return LevelInfo{
                price, std::accumulate(orders.begin(), orders.end(),
                                       (Quantity)0, [](Quantity runningSum, const OrderPointer& order)
                                       { return runningSum + order->GetRemainingQuantity(); })
            };
        };

        for (const auto& [price, orders] : bids_)
        {
            bidInfos.push_back(CreateLevelInfos(price, orders));
        }

        for (const auto& [price, orders] : asks_)
        {
            askInfos.push_back(CreateLevelInfos(price, orders));
        }

        return OrderbookLevelInfos{ bidInfos, askInfos };
    }

   private:
    struct OrderEntry
    {
        OrderPointer order_{ nullptr };
        OrderPointers::iterator location_;
    };

    std::map<Price, OrderPointers, std::greater<Price>> bids_;
    std::map<Price, OrderPointers, std::less<Price>> asks_;
    std::unordered_map<OrderID, OrderEntry> orders_;

    bool CanMatch(Side side, Price price) const
    {
        if (side == Side::Buy)
        {
            if (asks_.empty())
            {
                return false;
            }

            const auto& [bestAsk, _] = *asks_.begin();
            return price >= bestAsk;
        }
        else
        {
            if (bids_.empty())
            {
                return false;
            }

            const auto& [bestBid, _] = *bids_.begin();
            return price >= bestBid;
        }
    }

    Trades MatchOrder()
    {
        Trades trades;
        trades.reserve(orders_.size());

        while (true)
        {
            if (bids_.empty() || asks_.empty())
            {
                break;
            }

            auto& [bidPrice, bids] = *bids_.begin();
            auto& [askPrice, asks] = *asks_.begin();

            if (bidPrice < askPrice)
            {
                break;
            }

            while (bids.size() && asks.size())
            {
                auto bid = bids.front();
                auto ask = asks.front();

                Quantity quantity = std::min(bid->GetRemainingQuantity(), ask->GetRemainingQuantity());

                bid->Fill(quantity);
                ask->Fill(quantity);

                if (bid->IsFilled())
                {
                    bids.pop_front();
                    orders_.erase(bid->GetOrderID());
                }

                if (ask->IsFilled())
                {
                    asks.pop_front();
                    orders_.erase(ask->GetOrderID());
                }

                if (bids.empty())
                {
                    bids_.erase(bidPrice);
                }

                if (asks.empty())
                {
                    asks_.erase(askPrice);
                }

                trades.push_back(Trade{
                    TradeInfo{ bid->GetOrderID(), bid->GetPrice(), quantity },
                    TradeInfo{ ask->GetOrderID(), ask->GetPrice(), quantity } });
            }
        }

        if (bids_.empty() == false)
        {
            auto& [_, bids] = *bids_.begin();
            auto& order = bids.front();

            if (order->GetOrderType() == OrderType::FillAndKill)
            {
                CancelOrder(order->GetOrderID());
            }
        }

        if (asks_.empty() == false)
        {
            auto& [_, asks] = *asks_.begin();
            auto& order = asks.front();

            if (order->GetOrderType() == OrderType::FillAndKill)
            {
                CancelOrder(order->GetOrderID());
            }
        }

        return trades;
    }
};

int main()
{
    Orderbook orderbook;
    const OrderID orderID = 1;
    orderbook.AddOrder(std::make_shared<Order>(OrderType::GoodTillCancel, orderID, Side::Buy, 100, 10));

    std::cout << orderbook.Size() << std::endl;  // 1

    orderbook.CancelOrder(orderID);

    std::cout << orderbook.Size() << std::endl;  // 0
}
