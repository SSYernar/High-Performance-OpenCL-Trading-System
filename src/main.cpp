#include "matching_engine.hpp"
#include "market_data.hpp"
#include <iostream>

int main()
{
    MatchingEngine engine;
    engine.start();

    MarketData market_data(engine);
    market_data.generate_random_orders(10);

    std::this_thread::sleep_for(std::chrono::seconds(5)); // Increase wait time to allow order processing
    engine.stop();

    return 0;
}
