__kernel void match_orders(__global const float* buy_prices, __global const int* buy_quantities, 
                           __global const float* sell_prices, __global const int* sell_quantities, 
                           __global int* matched_orders, int num_buys, int num_sells) {
    int gid = get_global_id(0);
    if (gid < num_buys) {
        for (int j = 0; j < num_sells; j++) {
            if (buy_prices[gid] >= sell_prices[j] && buy_quantities[gid] > 0 && sell_quantities[j] > 0) {
                int matched_quantity = min(buy_quantities[gid], sell_quantities[j]);
                matched_orders[gid * 2] = gid; // Store buy order index
                matched_orders[gid * 2 + 1] = j; // Store sell order index

                buy_quantities[gid] -= matched_quantity;
                sell_quantities[j] -= matched_quantity;
            }
        }
    }
}
