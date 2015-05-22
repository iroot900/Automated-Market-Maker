#ifndef DATATYPE_H
#define DATATYPE_H
#include <queue>
#include <vector>
#include <map>
#include <set>
#include <unordered_map>
#include <string>
#include <functional>
using namespace std;
struct ClientInfo
{
	long long bought;
	long long sold;
	long long netTransfer;
};

struct EquityInfo
{
	vector<pair<long long, long long > > priceVolume;
};

class Order
{
public:
	Order() {}
	Order(long long timeStamp_, const string& clientName_, int side_, const string& equitySymbol_, long long  price_, long long quantity_, long long expiration_);
private:
	long long timeStamp;
	string clientName;
	string equitySymbol;
	long long  price;
	long long  quantity;
	long long  expiration;
	static long long ID;
	int side; // '0' buy , '1' sell
	friend class BuyOrderCmp;
	friend class SellOrderCmp;
	friend class MarketMaker;
};

class BuyOrderCmp
{
public:
	bool operator () (const Order& left, const Order& right) const
	{
		if (left.price < right.price) return true;
		if (left.price == right.price&&left.timeStamp > right.timeStamp) return true;
		return false;
	}
};

class SellOrderCmp
{
public:
	bool operator () (const Order& left, const Order& right) const
	{
		if (right.price < left.price) return true;
		if (right.price == left.price&&left.timeStamp>right.timeStamp) return true;
		return false;
	}
};

typedef priority_queue<long long, vector<long long>, less<long long> > MinHeap;
typedef priority_queue<long long, vector<long long>, greater<long long> > MaxHeap;
typedef unordered_map<string, MinHeap> MinHeaps;
typedef unordered_map<string, MaxHeap> MaxHeaps;
typedef unordered_map<string, ClientInfo> ClientsInfo;
typedef unordered_map<string, EquityInfo> EquitysInfo;
typedef priority_queue<Order, vector<Order>, SellOrderCmp>  SellBook;
typedef priority_queue<Order, vector<Order>, BuyOrderCmp>  BuyBook;
typedef unordered_map<string, SellBook> SellBooks;
typedef unordered_map<string, BuyBook> BuyBooks;
#endif
