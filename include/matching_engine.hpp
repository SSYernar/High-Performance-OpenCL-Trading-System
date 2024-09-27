#ifndef MATCHING_ENGINE_HPP
#define MATCHING_ENGINE_HPP

#include "order_book.hpp"
#include <thread>
#include <atomic>
#include <condition_variable>

class MatchingEngine
{
public:
    MatchingEngine();
    ~MatchingEngine();

    void submit_order(const Order &order);
    void start();
    void stop();

private:
    void process_orders();

    OrderBook order_book;
    std::atomic<bool> running;
    std::mutex orders_mutex;
    std::condition_variable order_cv;
    std::vector<Order> pending_orders;
    std::thread matching_thread;
};

#endif
