#include <database.hpp>

class Arbiter {
    public:
        Arbiter(account_name n): _self(n) { } 

        void subscribe(account_name subscriber, account_name provider, std::string endpoint, uint64_t dots);

        void unsubscribe(account_name subscriber, account_name provider, std::string endpoint, uint64_t dots, bool from_sub);

    private:
        account_name _self;
}
