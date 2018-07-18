#include <bondage.hpp>

void Bondage::bond(account_name token, account_name subscriber, account_name provider, std::string endpoint, uint64_t dots) {
    require_auth(subscriber);
    
    uint64_t price = Bondage::calc_dots_price(provider, specifier, dots);

    SEND_INLINE_ACTION(token, transfer, {provider, N(active)}, {provider, Bondage::_self, price, std::string()} );

    db::holderIndex holders(_self, subscriber);

    auto iterator = holders.find(subscriber);
    // TODO: Modify or create new record insde holders table
    if (iterator != holders.end()) {

    } else {

    }
}
