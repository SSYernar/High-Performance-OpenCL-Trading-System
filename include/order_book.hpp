#ifndef ORDER_BOOK_HPP
#define ORDER_BOOK_HPP

#include <map>
#include <mutex>
#include <vector>
#include <iostream>

struct Order
{
    int id;
    double price;
    int quantity;
    bool is_buy; // true for buy, false for sell
};

class OrderBook
{
public:
    OrderBook() = default;

    void add_order(const Order &order);
    void remove_order(int order_id);
    std::vector<Order> match_orders();

private:
    std::map<double, std::vector<Order>> buy_orders;                        // buy: highest price first
    std::map<double, std::vector<Order>, std::greater<double>> sell_orders; // sell: lowest price first
    std::mutex order_book_mutex;                                            // protects the order book
};

#endif
