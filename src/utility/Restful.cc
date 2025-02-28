#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <boost/beast/http/message.hpp>
#include <boost/beast/http/string_body.hpp>
#include <boost/beast/http/verb.hpp>
#include <functional>
#include <string>
#include <thread>
#include <unordered_map>

std::string data_store;  // 简易数据存储
using handleFunc = std::function<void(
    const boost::beast::http::request<boost::beast::http::string_body> &,
    const boost::beast::http::response<boost::beast::http::string_body> &)>;
std::unordered_map<std::string, handleFunc> routeMap;

void routerRegister(const std::string &path, boost::beast::http::verb method, handleFunc handler) {}

// 处理 HTTP 请求
void handle_request(
    const boost::beast::http::request<boost::beast::http::string_body> &req,
    boost::beast::http::response<boost::beast::http::string_body> &res) {
  if (req.method() == boost::beast::http::verb::get && req.target() == "/api/data") {
    res.result(boost::beast::http::status::ok);
    res.set(boost::beast::http::field::content_type, "application/json");
    res.body() = R"({"data": ")" + data_store + R"("})";
  } else if (req.method() == boost::beast::http::verb::post && req.target() == "/api/data") {
    data_store = req.body();
    res.result(boost::beast::http::status::created);
    res.body() = R"({"message": "data created"})";
  } else {
    res.result(boost::beast::http::status::not_found);
    res.body() = "404 Not Found";
  }
  res.prepare_payload();  // 自动计算 Content-Length
}

// 启动服务
int main() {
  boost::asio::io_context ioContext;
  boost::asio::ip::tcp::acceptor acceptor(ioContext, {boost::asio::ip::tcp::v4(), 9933});

  while (true) {
    boost::asio::ip::tcp::socket socket(ioContext);
    acceptor.accept(socket);  // 阻塞等待连接

    // 在新线程中处理请求
    std::thread([socket = std::move(socket)]() mutable {
      boost::beast::flat_buffer buffer;
      boost::beast::http::request<boost::beast::http::string_body> req;
      boost::beast::http::response<boost::beast::http::string_body> res;

      // 读取请求并处理
      boost::beast::http::read(socket, buffer, req);
      handle_request(req, res);
      boost::beast::http::write(socket, res);
      socket.shutdown(boost::asio::ip::tcp::socket::shutdown_send);
    }).detach();
  }
  return 0;
}