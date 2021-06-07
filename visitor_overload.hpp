//
// Created by kouta on 5/6/21.
//

#ifndef VITANET_VISITOR_OVERLOAD_HPP
#define VITANET_VISITOR_OVERLOAD_HPP

template<class... Ts>
struct overloaded : Ts ... {
    using Ts::operator()...;
};

template<class... Ts> overloaded(Ts...) -> overloaded<Ts...>;

#endif //VITANET_VISITOR_OVERLOAD_HPP
