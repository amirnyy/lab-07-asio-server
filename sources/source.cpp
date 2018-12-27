// Copyright 2018 amirnyy <alex.mirnyy@gmail.com>

#include <server.hpp>

boost::asio::io_service service;
std::vector<std::shared_ptr<talk_to_client>> clients;
std::recursive_mutex mutex;

std::string talk_to_client::username() const {
    return username_;
}

void talk_to_client::set_clients_changed() {
    clients_changed_ = true;
}


boost::asio::ip::tcp::socket& talk_to_client::sock() {
    return sock_;
}

bool talk_to_client::timed_out() const {
    auto now = std::chrono::high_resolution_clock::now();
    auto ms = std::chrono::duration_cast
    <std::chrono::milliseconds>
              (now - last_ping);
    return ms.count() > 50000;
}

void talk_to_client::stop() {
    boost::system::error_code err;
    sock_.close(err);
}

void talk_to_client::read_request()
{
    if ( sock_.available())
        sock_.read_some(
            boost::asio::buffer(buff_, max_msg));
}

void talk_to_client::process_request() {
    unsigned enter = 0;
    for (unsigned i = 0; i < max_msg; i++) {
        if (buff_[i] == '\n') enter = i;
    }
    if (enter == 0) return;
    last_ping = std::chrono::high_resolution_clock::now();
    std::string msg ="";
    for (unsigned i = 0; i < enter; i++) {
        msg+=buff_[i];
    }
    for (unsigned i = 0; i < max_msg; i++) {
        buff_[i] = ' ';
    }
    if ( msg.find("login") == 0) on_login(msg);
    else if ( msg.find("ping") == 0) on_ping();
    else if ( msg.find("ask_clients") == 0) on_clients();
    else
    std::cerr << "invalid msg " << msg << std::endl;
}


void talk_to_client::on_login(const std::string & msg)
{
    username_ = "";
    for (unsigned i = 6; i < msg.size(); i++) {
        if ((msg[i] != ' ') && (msg[i] != ':') && (msg[i] != '='))
            username_+=msg[i];
    }
    write("login ok\n");
    set_clients_changed();
}


void talk_to_client::on_ping()
{
    write(clients_changed_ ? "ping client_list_changed\n" : "ping ok\n");
    clients_changed_ = false;
}


void talk_to_client::on_clients()
{
    std::string msg;
    std::lock_guard<std::recursive_mutex> lock(mutex);
    for (auto& client : clients)
        msg += client->username() + " ";
    write("clients " + msg + "\n");
    clients_changed_ = false;
}


void talk_to_client::write(const std::string & msg) {
    sock_.write_some(boost::asio::buffer(msg));
}


void talk_to_client::answer_to_client()
{
    try
    {
        read_request();
        process_request();
    }
    catch ( boost::system::system_error&)
    {
        stop();
    }
    if ( timed_out())
        stop();
}
