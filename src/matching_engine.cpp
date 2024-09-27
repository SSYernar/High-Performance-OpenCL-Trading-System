#include "matching_engine.hpp"
#include <thread>
#include <future>

MatchingEngine::MatchingEngine() : running(false) {}

MatchingEngine::~MatchingEngine()
{
    stop();
}

void MatchingEngine::submit_order(const Order &order)
{
    {
        std::lock_guard<std::mutex> lock(orders_mutex);
        pending_orders.push_back(order);
    }
    std::cout << "Submitted Order ID: " << order.id
              << ", Price: " << order.price
              << ", Quantity: " << order.quantity
              << (order.is_buy ? " (Buy)" : " (Sell)") << std::endl;
    order_cv.notify_one();
}

void MatchingEngine::start()
{
    running = true;
    matching_thread = std::thread(&MatchingEngine::process_orders, this);
}

void MatchingEngine::stop()
{
    if (running)
    {
        running = false;
        order_cv.notify_one();
        if (matching_thread.joinable())
        {
            matching_thread.join();
        }
    }
}

void MatchingEngine::process_orders()
{
    while (running)
    {
        std::unique_lock<std::mutex> lock(orders_mutex);
        order_cv.wait(lock, [this]
                      { return !pending_orders.empty() || !running; });

        // Capture orders to process
        std::vector<Order> orders_to_process = std::move(pending_orders);
        pending_orders.clear();
        lock.unlock(); // Unlock while processing

        // Process each order in parallel
        std::vector<std::future<void>> futures;
        for (const auto &order : orders_to_process)
        {
            futures.emplace_back(std::async(std::launch::async, [this, order]
                                            {
                order_book.add_order(order);
                // Simulate order matching after adding to order book
                std::vector<Order> matched_orders = order_book.match_orders();
                for (const auto& matched_order : matched_orders) {
                    std::cout << "Matched Order ID: " << matched_order.id
                              << ", Price: " << matched_order.price
                              << ", Quantity: " << matched_order.quantity
                              << (matched_order.is_buy ? " (Buy)" : " (Sell)") << std::endl;
                } }));
        }

        // Wait for all futures to complete
        for (auto &future : futures)
        {
            future.get();
        }
    }
}
