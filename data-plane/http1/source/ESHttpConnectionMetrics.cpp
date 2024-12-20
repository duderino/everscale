#ifndef ES_HTTP_CONNECTION_METRICS_H
#include <ESHttpConnectionMetrics.h>
#endif

ES::HttpConnectionMetrics::HttpConnectionMetrics() {}
ES::HttpConnectionMetrics::~HttpConnectionMetrics() {}

// #include <iostream>
// #include <map>
// #include <list>
//
// using namespace std;
//
// int main() {
//   //
//   // Int Map (RB Tree)
//   //
//
//   map<int, int> intMap;
//
//   intMap.insert(pair<int, int>(1, 40));
//
//   intMap[7] = 10;
//
//   map<int, int>::iterator itr;
//   for (itr = intMap.begin(); itr != intMap.end(); ++itr) {
//     cout << '\t' << itr->first << '\t' << itr->second << endl;
//   }
//
//   //
//   // String Map (RB Tree)
//   //
//
//   string foo("foo");
//   map<string, int> strMap;
//
//   strMap.insert(pair<string, int>("foo", 42));
//
//   strMap["bar"] = 2122;
//
//   map<string, int>::iterator strIt;
//   for (strIt = strMap.begin(); strIt != strMap.end(); ++strIt) {
//     cout << '\t' << strIt->first << '\t' << strIt->second << endl;
//   }
//
//   //
//   // Int List (Doubly Linked List)
//   //
//
//   list<int> intList {1, 2, 3, 4};
//
//   for(int number : intList) {
//     cout << "for: " << number << endl;
//   }
//
//   for (list<int>::iterator listIt = intList.begin(); listIt != intList.end(); ++listIt) {
//     cout << "it: " << *listIt << endl;
//   }
//
//   return 0;
// }