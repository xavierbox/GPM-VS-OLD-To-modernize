#include <boost/multi_index_container.hpp>
#include <boost/multi_index/sequenced_index.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <iostream>
#include <vector>

namespace  bmi = boost::multi_index;
using index_list_type = boost::multi_index_container<
    std::string,
    bmi::indexed_by<
        bmi::sequenced<bmi::tag<struct ordered_by_insertion> >,
        bmi::ordered_unique<
            bmi::tag<struct ordered_by_id>,
            bmi::identity<std::string>
            >
        >
    >;

int multi_index_test(int argc, char* argv[]) {
    index_list_type t1 = {"BBB", "AAA", "CCC"};
       
    auto& by_ins = t1.get<ordered_by_insertion>();
    auto& by_id  = t1.get<ordered_by_id>();
    for(auto it=by_id.begin(); it != by_id.end();++it) {
        auto val = *it;
        auto position = distance(by_ins.begin(), bmi::project<ordered_by_insertion>(t1, it));
        std::cout << "Found " << val << " with insertion-order position " << position << "\n";
 
    }
    auto test = by_id.find("AAA");
    auto position = distance(by_ins.begin(), bmi::project<ordered_by_insertion>(t1, test));
    std::cout << "Found by search " << *test << " with insertion-order position " << position << "\n";
    return 0;
}