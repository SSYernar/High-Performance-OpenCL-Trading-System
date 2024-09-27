#include "market_data.hpp"

MarketData::MarketData(MatchingEngine &engine)
    : engine(engine), price_distribution(100.0, 200.0), quantity_distribution(1, 100),
      order_type_distribution(0, 1) {}

void MarketData::generate_random_orders(int num_orders)
{
    for (int i = 0; i < num_orders; ++i)
    {
        Order order;
        order.id = i + 1;
        order.price = price_distribution(generator);
        order.quantity = quantity_distribution(generator);
        order.is_buy = order_type_distribution(generator) == 1;

        engine.submit_order(order);
    }
}
