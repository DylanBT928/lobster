#include "../include/Orderbook.hpp"

#include <chrono>
#include <ctime>
#include <numeric>

Orderbook::Orderbook()
    : ordersPruneThread_{ [this]
                          { PruneGoodForDayOrders(); } }
{
}

Orderbook::~Orderbook()
{
    shutdownConditionVariable_.notify_one();
    ordersPruneThread_.join();
}

Trades Orderbook::AddOrder(OrderPointer order)
{
    std::scoped_lock ordersLock{ ordersMutex_ };

    if (orders_.contains(order->GetOrderID()))
    {
        return {};
    }

    if (order->GetOrderType() == OrderType::FillAndKill && CanMatch(order->GetSide(), order->GetPrice()) == false)
    {
        return {};
    }

    if (order->GetOrderType() == OrderType::FillOrKill && CanFullyFill(order->GetSide(), order->GetPrice(), order->GetRemainingQuantity()) == false)
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
    OnOrderAdded(order);

    return MatchOrders();
}

void Orderbook::CancelOrder(OrderID orderID)
{
    std::scoped_lock ordersLock{ ordersMutex_ };

    CancelOrderInternal(orderID);
}

Trades Orderbook::ModifyOrder(OrderModify order)
{
    OrderType orderType;

    {
        std::scoped_lock ordersLock{ ordersMutex_ };

        if (orders_.contains(order.GetOrderID()) == false)
        {
            return {};
        }

        const auto& [existingOrder, _] = orders_.at(order.GetOrderID());
        orderType = existingOrder->GetOrderType();
    }

    CancelOrder(order.GetOrderID());

    return AddOrder(order.ToOrderPointer(orderType));
}

std::size_t Orderbook::Size() const
{
    std::scoped_lock ordersLock{ ordersMutex_ };

    return orders_.size();
}

OrderbookLevelInfos Orderbook::GetOrderInfos() const
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

void Orderbook::PruneGoodForDayOrders()
{
    using namespace std::chrono;
    const auto end = hours(16);

    while (true)
    {
        const auto now = system_clock::now();
        const auto now_c = system_clock::to_time_t(now);
        std::tm now_parts;

#if defined(_WIN32) || defined(_MSC_VER)
        localtime_s(&now_parts, &now_c);
#else
        localtime_r(&now_c, &now_parts);
#endif

        if (now_parts.tm_hour >= end.count())
        {
            ++now_parts.tm_mday;
        }

        now_parts.tm_hour = end.count();
        now_parts.tm_min = 0;
        now_parts.tm_sec = 0;

        auto next = system_clock::from_time_t(mktime(&now_parts));
        auto till = next - now + milliseconds(100);

        {
            std::unique_lock ordersLock{ ordersMutex_ };

            if (shutdownConditionVariable_.wait_for(ordersLock, till) == std::cv_status::no_timeout)
            {
                return;
            }
        }

        OrderIDs orderIDs;

        {
            std::unique_lock ordersLock{ ordersMutex_ };

            for (const auto& [_, entry] : orders_)
            {
                const auto& [order, it] = entry;

                if (order->GetOrderType() != OrderType::GoodForDay)
                {
                    continue;
                }

                orderIDs.push_back(order->GetOrderID());
            }
        }

        CancelOrders(orderIDs);
    }
}

void Orderbook::CancelOrders(OrderIDs orderIDs)
{
    std::scoped_lock ordersLock{ ordersMutex_ };

    for (const auto& orderID : orderIDs)
    {
        CancelOrderInternal(orderID);
    }
}

void Orderbook::CancelOrderInternal(OrderID orderID)
{
    auto itEntry = orders_.find(orderID);

    if (itEntry == orders_.end())
    {
        return;
    }

    auto order = itEntry->second.order_;
    auto price = order->GetPrice();

    if (order->GetSide() == Side::Sell)
    {
        auto itLevel = asks_.find(price);

        if (itLevel != asks_.end())
        {
            auto& q = itLevel->second;
            auto it = std::find(q.begin(), q.end(), order);

            if (it != q.end())
            {
                q.erase(it);
            }

            if (q.empty())
            {
                asks_.erase(itLevel);
            }
        }
    }
    else
    {
        auto itLevel = bids_.find(price);

        if (itLevel != bids_.end())
        {
            auto& q = itLevel->second;
            auto it = std::find(q.begin(), q.end(), order);

            if (it != q.end())
            {
                q.erase(it);
            }

            if (q.empty())
            {
                bids_.erase(itLevel);
            }
        }
    }

    orders_.erase(itEntry);
    OnOrderCancelled(order);
}

void Orderbook::OnOrderCancelled(OrderPointer order)
{
    UpdateLevelData(order->GetPrice(), order->GetRemainingQuantity(), LevelData::Action::Remove);
}

void Orderbook::OnOrderAdded(OrderPointer order)
{
    UpdateLevelData(order->GetPrice(), order->GetInitialQuantity(), LevelData::Action::Add);
}

void Orderbook::OnOrderMatched(Price price, Quantity quantity, bool isFullyFilled)
{
    UpdateLevelData(price, quantity, isFullyFilled ? LevelData::Action::Remove : LevelData::Action::Match);
}

void Orderbook::UpdateLevelData(Price price, Quantity quantity, LevelData::Action action)
{
    auto& data = data_[price];

    data.count_ += action == LevelData::Action::Remove ? -1 : action == LevelData::Action::Add ? 1
                                                                                               : 0;
    if (action == LevelData::Action::Remove || action == LevelData::Action::Match)
    {
        data.quantity_ -= quantity;
    }
    else
    {
        data.quantity_ += quantity;
    }

    if (data.count_ == 0)
    {
        data_.erase(price);
    }
}

bool Orderbook::CanFullyFill(Side side, Price price, Quantity quantity) const
{
    if (CanMatch(side, price) == false)
    {
        return false;
    }

    std::optional<Price> threshold;

    if (side == Side::Buy)
    {
        const auto [askPrice, _] = *asks_.begin();
        threshold = askPrice;
    }
    else
    {
        const auto [bidPrice, _] = *bids_.begin();
        threshold = bidPrice;
    }

    for (const auto& [levelPrice, levelData] : data_)
    {
        if (threshold.has_value() &&
                (side == Side::Buy && threshold.value() > levelPrice) ||
            (side == Side::Sell && threshold.value() < levelPrice))
        {
            continue;
        }

        if ((side == Side::Buy && levelPrice > price) ||
            (side == Side::Sell && levelPrice < price))
        {
            continue;
        }

        if (quantity <= levelData.quantity_)
        {
            return true;
        }

        quantity -= levelData.quantity_;
    }

    return false;
}

bool Orderbook::CanMatch(Side side, Price price) const
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
        return price <= bestBid;
    }
}

Trades Orderbook::MatchOrders()
{
    Trades trades;
    trades.reserve(orders_.size());

    for (;;)
    {
        if (bids_.empty() || asks_.empty())
        {
            break;
        }

        auto& bestBidNode = *bids_.begin();
        auto& bestAskNode = *asks_.begin();

        Price bidPrice = bestBidNode.first;
        Price askPrice = bestAskNode.first;

        auto& bids = bestBidNode.second;
        auto& asks = bestAskNode.second;

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

            bool erasedLevel = false;

            if (bids.empty())
            {
                bids_.erase(bidPrice);
                data_.erase(bidPrice);
                erasedLevel = true;
            }

            if (asks.empty())
            {
                asks_.erase(askPrice);
                data_.erase(askPrice);
                erasedLevel = true;
            }

            trades.push_back(Trade{
                TradeInfo{ bid->GetOrderID(), bid->GetPrice(), quantity },
                TradeInfo{ ask->GetOrderID(), ask->GetPrice(), quantity } });

            OnOrderMatched(bid->GetPrice(), quantity, bid->IsFilled());
            OnOrderMatched(ask->GetPrice(), quantity, ask->IsFilled());

            if (erasedLevel)
            {
                break;
            }
        }
    }

    if (bids_.empty() == false)
    {
        auto& order = bids_.begin()->second.front();

        if (order->GetOrderType() == OrderType::FillAndKill)
        {
            CancelOrder(order->GetOrderID());
        }
    }

    if (asks_.empty() == false)
    {
        auto& order = asks_.begin()->second.front();

        if (order->GetOrderType() == OrderType::FillAndKill)
        {
            CancelOrder(order->GetOrderID());
        }
    }

    return trades;
}
