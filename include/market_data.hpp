#ifndef MARKET_DATA_HPP
#define MARKET_DATA_HPP

#include "matching_engine.hpp"
#include <random>

class MarketData
{
public:
    MarketData(MatchingEngine &engine);
    void generate_random_orders(int num_orders);

private:
    MatchingEngine &engine;
    std::default_random_engine generator;
    std::uniform_real_distribution<double> price_distribution;
    std::uniform_int_distribution<int> quantity_distribution;
    std::uniform_int_distribution<int> order_type_distribution;
};

#endif
