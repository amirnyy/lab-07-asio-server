// Copyright 2018 amirnyy <alex.mirnyy@gmail.com>

#ifndef INCLUDE_SERVER_HPP_
#define INCLUDE_SERVER_HPP_

#include <string>
#include <iostream>
#include <chrono>
#include <algorithm>
#include <cstdlib>
#include <boost/aligned_storage.hpp>
#include <boost/array.hpp>
#include <boost/bind.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/asio.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/log/core.hpp>
#include <boost/core/null_deleter.hpp>
#include <boost/smart_ptr/shared_ptr.hpp>
#include <boost/smart_ptr/make_shared_object.hpp>
#include <boost/log/sinks/sync_frontend.hpp>
#include <boost/log/sinks/text_ostream_backend.hpp>
#include <boost/log/sources/logger.hpp>
#include <boost/log/sinks/text_file_backend.hpp>
#include <boost/log/sources/record_ostream.hpp>
#include <mutex>
#include <vector>
namespace logging = boost::log;
namespace src = boost::log::sources;
namespace sinks = boost::log::sinks;
class talk_to_client;
extern boost::asio::io_service service;
extern std::vector<std::shared_ptr<talk_to_client>> clients;
extern std::recursive_mutex mutex;
namespace keywords = boost::log::keywords;
namespace expr = boost::log::expressions;

class talk_to_client {
private:
    boost::asio::ip::tcp::socket sock_;
    enum { max_msg = 1024 };
    char buff_[1024];
    bool clients_changed_;
    std::string username_;
    std::chrono::time_point<std::chrono::high_resolution_clock> last_ping;
public:
    talk_to_client(): sock_(service){}
    std::string username() const;
    boost::asio::ip::tcp::socket& sock();
    bool timed_out() const;
    void stop();
    void read_request();
    void process_request();
    void on_login(const std::string & msg);
    void on_ping();
    void on_clients();
    void write(const std::string & msg);
    void answer_to_client();
    void set_clients_changed();
};

#endif // INCLUDE_SERVER_HPP_
