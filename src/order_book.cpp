#include "order_book.hpp"
#include <CL/cl.h>
#include <vector>

void OrderBook::add_order(const Order &order)
{
    std::lock_guard<std::mutex> lock(order_book_mutex);
    if (order.is_buy)
    {
        buy_orders[order.price].push_back(order);
    }
    else
    {
        sell_orders[order.price].push_back(order);
    }
}

std::vector<Order> OrderBook::match_orders()
{
    std::lock_guard<std::mutex> lock(order_book_mutex);

    // Prepare data for OpenCL
    std::vector<float> buy_prices;
    std::vector<int> buy_quantities;
    std::vector<float> sell_prices;
    std::vector<int> sell_quantities;

    for (const auto &[price, orders] : buy_orders)
    {
        for (const auto &order : orders)
        {
            buy_prices.push_back(price);
            buy_quantities.push_back(order.quantity);
        }
    }

    for (const auto &[price, orders] : sell_orders)
    {
        for (const auto &order : orders)
        {
            sell_prices.push_back(price);
            sell_quantities.push_back(order.quantity);
        }
    }

    // Initialize OpenCL
    cl_platform_id platform_id = nullptr;
    clGetPlatformIDs(1, &platform_id, nullptr);

    cl_device_id device_id = nullptr;
    clGetDeviceIDs(platform_id, CL_DEVICE_TYPE_GPU, 1, &device_id, nullptr);

    cl_context context = clCreateContext(nullptr, 1, &device_id, nullptr, nullptr, nullptr);
    cl_command_queue queue = clCreateCommandQueue(context, device_id, 0, nullptr);

    cl_mem d_buy_prices = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                                         buy_prices.size() * sizeof(float), buy_prices.data(), nullptr);
    cl_mem d_buy_quantities = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                                             buy_quantities.size() * sizeof(int), buy_quantities.data(), nullptr);
    cl_mem d_sell_prices = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                                          sell_prices.size() * sizeof(float), sell_prices.data(), nullptr);
    cl_mem d_sell_quantities = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                                              sell_quantities.size() * sizeof(int), sell_quantities.data(), nullptr);

    // Prepare buffer for matched orders
    std::vector<int> matched_orders(buy_prices.size() * 2, -1);
    cl_mem d_matched_orders = clCreateBuffer(context, CL_MEM_WRITE_ONLY | CL_MEM_COPY_HOST_PTR,
                                             matched_orders.size() * sizeof(int), matched_orders.data(), nullptr);

    // Load OpenCL kernel
    const char *kernel_source = read_kernel_file("src/opencl_kernels.cl");
    cl_program program = clCreateProgramWithSource(context, 1, &kernel_source, nullptr, nullptr);
    clBuildProgram(program, 1, &device_id, nullptr, nullptr, nullptr);

    cl_kernel kernel = clCreateKernel(program, "match_orders", nullptr);
    clSetKernelArg(kernel, 0, sizeof(cl_mem), &d_buy_prices);
    clSetKernelArg(kernel, 1, sizeof(cl_mem), &d_buy_quantities);
    clSetKernelArg(kernel, 2, sizeof(cl_mem), &d_sell_prices);
    clSetKernelArg(kernel, 3, sizeof(cl_mem), &d_sell_quantities);
    clSetKernelArg(kernel, 4, sizeof(cl_mem), &d_matched_orders);
    clSetKernelArg(kernel, 5, sizeof(int), &buy_prices.size());
    clSetKernelArg(kernel, 6, sizeof(int), &sell_prices.size());

    size_t global_work_size = buy_prices.size();
    clEnqueueNDRangeKernel(queue, kernel, 1, nullptr, &global_work_size, nullptr, 0, nullptr, nullptr);
    clFinish(queue);

    // Read matched orders back to host
    clEnqueueReadBuffer(queue, d_matched_orders, CL_TRUE, 0, matched_orders.size() * sizeof(int), matched_orders.data(), 0, nullptr, nullptr);

    // Process matched orders
    std::vector<Order> orders_matched;
    for (size_t i = 0; i < matched_orders.size() / 2; i++)
    {
        if (matched_orders[i * 2] != -1 && matched_orders[i * 2 + 1] != -1)
        {
            // Match found, add to orders_matched
            orders_matched.push_back({i, buy_prices[matched_orders[i * 2]], buy_quantities[matched_orders[i * 2]], true});
            orders_matched.push_back({i, sell_prices[matched_orders[i * 2 + 1]], sell_quantities[matched_orders[i * 2 + 1]], false});
        }
    }

    // Cleanup OpenCL resources
    clReleaseMemObject(d_buy_prices);
    clReleaseMemObject(d_buy_quantities);
    clReleaseMemObject(d_sell_prices);
    clReleaseMemObject(d_sell_quantities);
    clReleaseMemObject(d_matched_orders);
    clReleaseKernel(kernel);
    clReleaseProgram(program);
    clReleaseCommandQueue(queue);
    clReleaseContext(context);

    return orders_matched;
}

// Helper function to read OpenCL kernel from file
const char *read_kernel_file(const char *file_name)
{
    std::ifstream file(file_name);
    if (!file.is_open())
    {
        std::cerr << "Failed to open kernel file." << std::endl;
        return nullptr;
    }
    std::string source_code((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    char *source_cstr = new char[source_code.size() + 1];
    std::strcpy(source_cstr, source_code.c_str());
    return source_cstr;
}
