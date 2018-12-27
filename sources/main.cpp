// Copyright 2018 amirnyy <alex.mirnyy@gmail.com>
#include<server.hpp>

extern boost::asio::io_service service;
extern std::vector<std::shared_ptr<talk_to_client>> clients;
extern std::recursive_mutex mutex;

void init() {
    boost::shared_ptr< logging::core > core = logging::core::get();

    boost::shared_ptr< sinks::text_file_backend > backend =
        boost::make_shared< sinks::text_file_backend >(
            keywords::file_name = "file_%5N.log",
            keywords::rotation_size = 5 * 1024 * 1024,
            keywords::format = "[%TimeStamp%]: %Message%",
            keywords::time_based_rotation =
                sinks::file::rotation_at_time_point(12, 0, 0));

    typedef sinks::synchronous_sink< sinks::text_file_backend > sink_t;
    boost::shared_ptr< sink_t > sink(new sink_t(backend));
    //sink ->set_filter(logging::trivial::severity >= logging::trivial::info);
    core->add_sink(sink);
}

void accept_thread()
{
    boost::asio::ip::tcp::acceptor acceptor(service,
    boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), 8001));
    while (true)
    {
        auto client = std::make_shared<talk_to_client>();
        acceptor.accept(client->sock());
        std::lock_guard<std::recursive_mutex> lock(mutex);
        clients.push_back(client);
        BOOST_LOG_TRIVIAL(trace) << "Add client" << std::endl;
    }
}

void handle_clients_thread()
{
    while (true)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        std::lock_guard<std::recursive_mutex> lock(mutex);
        for (auto b = clients.begin(), e = clients.end(); b != e; ++b)
            (*b)->answer_to_client();
        clients.erase(std::remove_if(clients.begin(),
                                     clients.end(),
                                     boost::bind(
                                     &talk_to_client::timed_out, _1)),
                      clients.end());
    }
}


int main() {
    init();
    std::thread thread1(accept_thread);
    std::thread thread2(handle_clients_thread);
    thread1.join();
    thread2.join();
    return 0;
}
