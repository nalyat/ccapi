#include "ccapi_cpp/ccapi_session.h"
namespace ccapi {
class MyLogger final : public Logger {
 public:
  void logMessage(const std::string& severity, const std::string& threadId, const std::string& timeISO, const std::string& fileName,
                  const std::string& lineNumber, const std::string& message) override {
    std::lock_guard<std::mutex> lock(m);
    std::cout << threadId << ": [" << timeISO << "] {" << fileName << ":" << lineNumber << "} " << severity << std::string(8, ' ') << message << std::endl;
  }

 private:
  std::mutex m;
};
MyLogger myLogger;
Logger* Logger::logger = &myLogger;
class MyEventHandler : public EventHandler {
 public:
  bool processEvent(const Event& event, Session* session) override {
    if (event.getType() == Event::Type::SUBSCRIPTION_STATUS) {
      std::cout << "Received an event of type SUBSCRIPTION_STATUS:\n" + event.toStringPretty(2, 2) << std::endl;
      auto message = event.getMessageList().at(0);
      if (message.getType() == Message::Type::SUBSCRIPTION_STARTED) {
        Request request(Request::Operation::CANCEL_ORDER, "cryptocom", "BTC_USDT", "same correlation id for subscription and request");
        request.appendParam({
            {"ORDER_ID", "2111100095665909890"},
            // {"LIMIT_PRICE", "20000"},
            // {"QUANTITY", "0.000001"},
        });
        session->sendRequestByWebsocket(request);
      }
    } else if (event.getType() == Event::Type::SUBSCRIPTION_DATA) {
      std::cout << "Received an event of type SUBSCRIPTION_DATA:\n" + event.toStringPretty(2, 2) << std::endl;
    } else if (event.getType() == Event::Type::RESPONSE) {
      std::cout << "Received an event of type RESPONSE:\n" + event.toStringPretty(2, 2) << std::endl;
    }
    return true;
  }
};
} /* namespace ccapi */
using ::ccapi::MyEventHandler;
using ::ccapi::Request;
using ::ccapi::Session;
using ::ccapi::SessionConfigs;
using ::ccapi::SessionOptions;
using ::ccapi::Subscription;
using ::ccapi::UtilSystem;
int main(int argc, char** argv) {
  // if (UtilSystem::getEnvAsString("OKEX_API_KEY").empty()) {
  //   std::cerr << "Please set environment variable OKEX_API_KEY" << std::endl;
  //   return EXIT_FAILURE;
  // }
  // if (UtilSystem::getEnvAsString("OKEX_API_SECRET").empty()) {
  //   std::cerr << "Please set environment variable OKEX_API_SECRET" << std::endl;
  //   return EXIT_FAILURE;
  // }
  // if (UtilSystem::getEnvAsString("OKEX_API_PASSPHRASE").empty()) {
  //   std::cerr << "Please set environment variable OKEX_API_PASSPHRASE" << std::endl;
  //   return EXIT_FAILURE;
  // }
  SessionOptions sessionOptions;
  SessionConfigs sessionConfigs;
  MyEventHandler eventHandler;
  Session session(sessionOptions, sessionConfigs, &eventHandler);
  Subscription subscription("cryptocom", "BTC_USDT", "ORDER_UPDATE,PRIVATE_TRADE", "", "same correlation id for subscription and request");
  session.subscribe(subscription);
  std::this_thread::sleep_for(std::chrono::seconds(10));
  session.stop();
  std::cout << "Bye" << std::endl;
  return EXIT_SUCCESS;
}
