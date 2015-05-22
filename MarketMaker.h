#ifndef MARKETMAKER_H
#define MARKETMAKER_H
#include "dataType.h"
#include <time.h>
#include <iostream>
#include <random>
#include <fstream>
class MarketMaker
{
public:
	MarketMaker(int argc, char** argv);
	void start();
private:
	void orderGenerator(Order& order, mt19937& gen);
	void orderMatch(Order& order);
	void processTime(const Order& order);
	void orderFlow(string orderInfo, const string& delimiter, Order& order);
	void help() const;
	void print() const;
	void notEnd(size_t pos) const;
	void isNum(const string& str) const;
	void medi(const string & symbol, long long newPrice);
	SellBooks sellBooks;
	BuyBooks buyBooks;
	long long currentStamp;
	//options
	bool verbose;
	bool median;
	bool midPoint;
	bool transfers;
	bool VWAP;
	bool randomOrder;
	string delimiter;
	string filename;
	//stats
	long long commissions;
	long long moneyTransfered;
	long long noTrades;
	long long sharesTraded;
	ClientsInfo clientsInfo;
	EquitysInfo equitysInfo;
	unordered_map<string, long long > medians;
	MinHeaps minHeaps;
	MaxHeaps maxHeaps;
	set<string> equities;
};
#endif
