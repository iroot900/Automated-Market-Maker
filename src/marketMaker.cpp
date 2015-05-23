#include "marketMaker.h"
#include <getopt.h>
MarketMaker::MarketMaker(int argc, char** argv) :verbose(true), median(true), midPoint(true), transfers(true), VWAP(true), delimiter("   "), filename("test.txt"), randomOrder(false)
	{
		const struct option long_options[] =
		{
		{ "help", 0, 0, 'h' },
		{ "verbose", 0, 0, 'v' },
		{ "median", 0, 0, 'm' },
		{ "midpoint", 0, 0, 'p' },
		{ "transfers", 0, 0, 't' },
		{ "VWAP", 0, 0, 'w' },
		{ "filename", 1, 0, 'f' },
		{ "delimiter", 1, 0, 'd' },
		{ "randomOrder", 1, 0, 'r' },
		{ 0, 1, 0, 0 },
		};

		int c = 0;
		while ((c = getopt_long(argc, argv, "hvmpf:dtwr", long_options, NULL) != -1))
		{
		switch (c)
		{
		case 'h':
		help();
		break;
		case 'v':
		verbose = false;
		break;
		case 'm':
		median = false;
		break;
		case 'p':
		midPoint = false;
		break;
		case 'f':
		filename = optarg;
		break;
		case 'd':
		delimiter = optarg;
		break;
		case 't':
		transfers = false;
		break;
		case 'w':
		VWAP = false;
		case 'r':
		randomOrder = true;
		break;
		case '?':
		exit(-1);
		default:
		cout << "wrong options!" << endl;
		exit(-1);
		}
		}
		currentStamp = 0;
		commissions = 0;
		moneyTransfered = 0;
		noTrades = 0;
		sharesTraded = 0;
	}

void MarketMaker::start()
	{
		Order order;
		string orderInfo;

		if (!randomOrder)
		{
			string type;
			ifstream myfile(filename);
			if (myfile.is_open())
			{
				getline(myfile, type);
				if (type != "TL")
				{
					cout << "Input not supported!" << endl; exit(-1);
				}
				while (getline(myfile, orderInfo))
				{
					orderFlow(orderInfo, delimiter, order);
					processTime(order);
					orderMatch(order);
				}
				myfile.close();
			}
		}
		else
		{
			int i = 100;
			mt19937 gen((unsigned)time(0));
			while (i--)
			{
				orderGenerator(order, gen);
				processTime(order);
				orderMatch(order);
			}
		}
		//one more time
		order.timeStamp = -1;
		processTime(order);
		print();
	}

void MarketMaker::orderGenerator(Order& order, mt19937& gen)
	{
		static long long timeStamp;
		string clientName;
		string equitySymbol;
		long long price;
		long long quantity;
		long long expiration;
		int side; // '0' buy , '1' sell

		uniform_int_distribution<int> clientGen(0, 5);
		uniform_int_distribution<int> equityGen(0, 5);
		bernoulli_distribution sideGen(0.5);
		uniform_int_distribution<int> priceGen(50, 100);
		uniform_int_distribution<int> quantityGen(10, 20);
		uniform_int_distribution<int> duraGen(-1, 5);
		exponential_distribution<> stampGen(0.8);
		timeStamp += floor(stampGen(gen));

		char cli = clientGen(gen) + 'a';
		char eq = equityGen(gen) + 'a';
		clientName = "C_" + string(1, cli);
		equitySymbol = "E_" + string(1, eq);
		side = sideGen(gen);
		price = priceGen(gen);
		quantity = quantityGen(gen);
		expiration = duraGen(gen);
		equities.insert(equitySymbol);
		Order orderTemp(timeStamp, clientName, side, equitySymbol, price, quantity, expiration);
		order = orderTemp;
	}

void MarketMaker::orderMatch(Order& order)
	{
		auto symbol = order.equitySymbol;
		auto expiration = order.expiration;// need check for expiration of each order;

		if (order.side == 0)
		{
			while (!sellBooks[symbol].empty())// not empty. match it. 
			{
				auto bestSell = sellBooks[symbol].top();
				if (bestSell.expiration != -1 && !(currentStamp - bestSell.timeStamp<bestSell.expiration))
				{
					sellBooks[symbol].pop(); break;
				}
				if (order.price > bestSell.price || order.price == bestSell.price)
				{
					if (order.quantity < bestSell.quantity)
					{
						auto ord = sellBooks[symbol].top();
						ord.quantity = bestSell.quantity - order.quantity;
						sellBooks[symbol].pop();
						sellBooks[symbol].push(ord);
						if (verbose)
							cout << order.clientName << " purchased " << order.quantity << " share of " << symbol << " from " << bestSell.clientName << " for $" << bestSell.price << "/share" << endl;
						//market stats
						commissions += ((bestSell.price*order.quantity) / 100) * 2;
						moneyTransfered += bestSell.price*order.quantity;
						++noTrades;
						sharesTraded += order.quantity;
						//clients stats
						clientsInfo[order.clientName].bought += order.quantity;
						clientsInfo[order.clientName].netTransfer -= bestSell.price*order.quantity;
						clientsInfo[bestSell.clientName].sold += order.quantity;
						clientsInfo[bestSell.clientName].netTransfer += bestSell.price*order.quantity;
						//stock stats
						equitysInfo[symbol].priceVolume.push_back(make_pair(bestSell.price, order.quantity));
						// median
						medi(symbol, bestSell.price);
						return; // all matched; update the book; 
					}
					else if (order.quantity == bestSell.quantity)
					{
						sellBooks[symbol].pop();
						if (verbose)
							cout << order.clientName << " purchased " << order.quantity << " share of " << symbol << " from " << bestSell.clientName << " for $" << bestSell.price << "/share" << endl;
						commissions += ((bestSell.price*order.quantity) / 100) * 2;
						moneyTransfered += bestSell.price*order.quantity;
						++noTrades;
						sharesTraded += order.quantity;
						//clients stats
						clientsInfo[order.clientName].bought += order.quantity;
						clientsInfo[order.clientName].netTransfer -= bestSell.price*order.quantity;
						clientsInfo[bestSell.clientName].sold += order.quantity;
						clientsInfo[bestSell.clientName].netTransfer += bestSell.price*order.quantity;
						//stock stats
						equitysInfo[symbol].priceVolume.push_back(make_pair(bestSell.price, order.quantity));
						// median
						medi(symbol, bestSell.price);
						return; // all matched; update the book; 
					}
					else // partially matched, then update order and book. 
					{
						order.quantity = order.quantity - bestSell.quantity;
						sellBooks[symbol].pop(); // then for next round; like new order new book; 
						if (verbose)
							cout << order.clientName << " purchased " << bestSell.quantity << " share of " << symbol << " from " << bestSell.clientName << " for $" << bestSell.price << "/share" << endl;
						commissions += ((bestSell.price*bestSell.quantity) / 100) * 2;
						moneyTransfered += bestSell.price*bestSell.quantity;
						++noTrades;
						sharesTraded += bestSell.quantity;
						//clients stats
						clientsInfo[order.clientName].bought += bestSell.quantity;
						clientsInfo[order.clientName].netTransfer -= bestSell.price*bestSell.quantity;
						clientsInfo[bestSell.clientName].sold += bestSell.quantity;
						clientsInfo[bestSell.clientName].netTransfer += bestSell.price*bestSell.quantity;
						//stock stats
						equitysInfo[symbol].priceVolume.push_back(make_pair(bestSell.price, bestSell.quantity));
						// median
						static bool flag = true;
						if (flag)
						{
							medi(symbol, bestSell.price);
							flag = false;
						}
					}
				}
				else// no match for price; 
				{
					if (expiration != 0)
						buyBooks[symbol].push(order); return;
				}
			}
			// no match for empty book;
			if (expiration != 0)
				buyBooks[symbol].push(order); return;
		}
		else
		{
			while (!buyBooks[symbol].empty())
			{
				auto bestBuy = buyBooks[symbol].top();
				if (bestBuy.expiration != -1 && !(currentStamp - bestBuy.timeStamp<bestBuy.expiration))
				{
					buyBooks[symbol].pop(); break;
				}
				if (order.price < bestBuy.price || order.price == bestBuy.price)
				{
					if (order.quantity < bestBuy.quantity)
					{
						auto ord = buyBooks[symbol].top();
						ord.quantity = bestBuy.quantity - order.quantity;
						buyBooks[symbol].pop();
						buyBooks[symbol].push(ord);
						if (verbose)
							cout << bestBuy.clientName << " purchased " << order.quantity << " share of " << symbol << " from " << order.clientName << " for $" << bestBuy.price << "/share" << endl;
						commissions += ((bestBuy.price*order.quantity) / 100) * 2;
						moneyTransfered += bestBuy.price*order.quantity;
						++noTrades;
						sharesTraded += order.quantity;
						// clients stats
						clientsInfo[bestBuy.clientName].bought += order.quantity;
						clientsInfo[bestBuy.clientName].netTransfer -= bestBuy.price*order.quantity;
						clientsInfo[order.clientName].sold += order.quantity;
						clientsInfo[order.clientName].netTransfer += bestBuy.price*order.quantity;
						//stock stats
						equitysInfo[symbol].priceVolume.push_back(make_pair(bestBuy.price, order.quantity));
						// median
						medi(symbol, bestBuy.price);
						return; // all matched; update the book; 
					}
					else if (order.quantity == bestBuy.quantity)
					{
						buyBooks[symbol].pop();
						if (verbose)
							cout << bestBuy.clientName << " purchased " << order.quantity << " share of " << symbol << " from " << order.clientName << " for $" << bestBuy.price << "/share" << endl;
						commissions += ((bestBuy.price*order.quantity) / 100) * 2;
						moneyTransfered += bestBuy.price*order.quantity;
						++noTrades;
						sharesTraded += order.quantity;
						// clients stats
						clientsInfo[bestBuy.clientName].bought += order.quantity;
						clientsInfo[bestBuy.clientName].netTransfer -= bestBuy.price*order.quantity;
						clientsInfo[order.clientName].sold += order.quantity;
						clientsInfo[order.clientName].netTransfer += bestBuy.price*order.quantity;
						//stock stats
						equitysInfo[symbol].priceVolume.push_back(make_pair(bestBuy.price, order.quantity));
						// median
						medi(symbol, bestBuy.price);
						return;
					}
					else
					{
						order.quantity = order.quantity - bestBuy.quantity;
						buyBooks[symbol].pop();
						if (verbose)
							cout << bestBuy.clientName << " purchased " << bestBuy.quantity << " share of " << symbol << " from " << order.clientName << " for $" << bestBuy.price << "/share" << endl;
						commissions += ((bestBuy.price*bestBuy.quantity) / 100) * 2;
						moneyTransfered += bestBuy.price*bestBuy.quantity;
						++noTrades;
						sharesTraded += bestBuy.quantity;
						// clients stats
						clientsInfo[bestBuy.clientName].bought += bestBuy.quantity;
						clientsInfo[bestBuy.clientName].netTransfer -= bestBuy.price*bestBuy.quantity;
						clientsInfo[order.clientName].sold += bestBuy.quantity;
						clientsInfo[order.clientName].netTransfer += bestBuy.price*bestBuy.quantity;
						//stock stats
						equitysInfo[symbol].priceVolume.push_back(make_pair(bestBuy.price, bestBuy.quantity));
						// median
						static bool flag = true;
						if (flag)
						{
							medi(symbol, bestBuy.price);
							flag = false;
						}
					}
				}
				else // no match for price
				{
					if (expiration != 0)
						sellBooks[symbol].push(order); return;
				}
			}
			if (expiration != 0)
				sellBooks[symbol].push(order); return;
		}
	}

void MarketMaker::processTime(const Order& order)
	{
		if (order.timeStamp != currentStamp)
		{
			if(median)
			{
				for (auto medianPair : medians)
				{
					cout << "Median match price of " << medianPair.first << " at time " << currentStamp << " is " << medianPair.second << endl;
				}
			}
			if(midPoint)
			{
				for (auto equity : equities)
				{
					if (sellBooks[equity].size() == 0 || buyBooks[equity].size() == 0)
						cout << "Midpoint of " << equity << " at time " << currentStamp << " is undefine! " << endl;
					else
						cout << "Midpoint of " << equity << " at time " << currentStamp << " is $ " << ((sellBooks[equity].top().price + buyBooks[equity].top().price) >> 1) << endl;
				}
			}
			currentStamp = order.timeStamp;
		}
	}

void MarketMaker::orderFlow(string orderInfo, const string& delimiter, Order& order)
	{
		long long timeStamp;
		string clientName;
		string equitySymbol;
		long long price;
		long long quantity;
		long long expiration;
		int side; // '0' buy , '1' sell

		//timeStamp
		auto pleft = orderInfo.find_first_not_of(delimiter);
		auto pright = orderInfo.find_first_of(delimiter, pleft);
		auto temp = orderInfo.substr(pleft, pright - pleft);
		isNum(temp); notEnd(pright);
		timeStamp = stoll(temp);
		if (timeStamp<currentStamp) { cout << "NetWorking Issue!" << endl; exit(-1); }
		//clientName
		pleft = orderInfo.find_first_not_of(delimiter, pright);
		pright = orderInfo.find_first_of(delimiter, pleft);
		temp = orderInfo.substr(pleft, pright - pleft);
		notEnd(pright);
		clientName = temp;
		//side
		pleft = orderInfo.find_first_not_of(delimiter, pright);
		pright = orderInfo.find_first_of(delimiter, pleft);
		temp = orderInfo.substr(pleft, pright - pleft);
		notEnd(pright);
		if (temp == "BUY") side = 0;
		else if (temp == "SELL") side = 1;
		else { cout << "Order Side Type Not Supported!" << endl; exit(-1); }
		//equitySymbol
		pleft = orderInfo.find_first_not_of(delimiter, pright);
		pright = orderInfo.find_first_of(delimiter, pleft);
		temp = orderInfo.substr(pleft, pright - pleft);
		notEnd(pright);
		equitySymbol = temp;
		//price
		pleft = orderInfo.find_first_not_of(delimiter, pright);
		pright = orderInfo.find_first_of(delimiter, pleft);
		if (orderInfo[pleft] != '$') { cout << "$ Missing for Price info!" << endl; exit(-1); }
		temp = orderInfo.substr(pleft + 1, pright - pleft - 1);
		isNum(temp); notEnd(pright);
		price = stoi(temp);
		//quantity
		pleft = orderInfo.find_first_not_of(delimiter, pright);
		pright = orderInfo.find_first_of(delimiter, pleft);
		if (orderInfo[pleft] != '#') { cout << "# Missing for Quantity info!" << endl; exit(-1); }
		temp = orderInfo.substr(pleft + 1, pright - pleft - 1);
		isNum(temp); notEnd(pright);
		quantity = stoi(temp);
		//expiration
		pleft = orderInfo.find_first_not_of(delimiter, pright);
		pright = orderInfo.find_first_of(delimiter, pleft);
		temp = orderInfo.substr(pleft, pright - pleft);
		isNum(temp);
		expiration = stoi(temp);
		equities.insert(equitySymbol);
		//		cout << "     <Order comes in: " << timeStamp << " " << clientName << " " << side << " " << equitySymbol << " $" << price << " #" << quantity << " " << expiration << ">"<<endl;
		Order orderTemp(timeStamp, clientName, side, equitySymbol, price, quantity, expiration);
		order = orderTemp;
	}

void MarketMaker::help() const
	{
		cout << "-h or -help    : print out help menu!" << endl;
		cout << "-v or -verbose : disable print out each transaction!" << endl;
		cout << "-m or -median    : disable print out median price for each equity!" << endl;
		cout << "-p or -midpoint : disable print out midpoint for each equity!" << endl;
		cout << "-t or -transfers : disable print out transaction details for each client!" << endl;
		cout << "-w or -VWAP : disable print out volume weighted average price of the following equity!" << endl;
		cout << "-f or -filename : specify input filename, default one is  test.txt" << endl;
		cout << "-d or -delimiter    : specify delimiter for input file, default one is space and tab " << endl;
		cout << "-r or -randomOrder    : use possion process to generate random order flow instead of using the test.txt " << endl;
	}

void MarketMaker::print() const
	{
		cout << '\n' << "---End of Day--- " << endl;
		cout << "Commission Earnings: $" << commissions << endl;
		cout << "Total Amount of Money Transferred: $" << moneyTransfered << endl;
		cout << "Number of Completed Trades: " << noTrades << endl;
		cout << "Number of Shares Traded: " << sharesTraded << endl;
		if(transfers)
		{
			for (auto client : clientsInfo)
			{
				cout << client.first << " bought " << client.second.bought << " and sold " << client.second.sold << " for a net transfer of $" << client.second.netTransfer << endl;
			}
		}
		if(VWAP)
		{
			for (auto equity : equities)
			{
				if (equitysInfo.find(equity) != equitysInfo.end())
				{
					long long amountMoney = 0, noShares = 0;
					for (auto priceVolume : equitysInfo.at(equity).priceVolume)
					{
						amountMoney += priceVolume.first*priceVolume.second;
						noShares += priceVolume.second;
					}
					cout << equity << "'s volume weighted average price : $" << amountMoney / noShares << endl;
				}
				else
				{
					cout << equity << "'s volume weighted average price : $" << -1 << endl;
				}
			}
		}
	}

void MarketMaker::notEnd(size_t pos) const
	{
		if (pos == string::npos)
		{
			cout << "End premature!" << endl;
			exit(-1);
		}
	}

void MarketMaker::isNum(const string& str) const
	{
		if (str[0] == '-')
		{
			if (str.size() == 1)
			{
				cout << "Not a valid Number!" << endl;
				exit(-1);
			}
		}
		for (int i = 1; i < (int)str.size(); i++)
		{
			if (!isdigit(str[i]))
			{
				cout << "Not a valid Number!" << endl;
				exit(-1);
			}
		}
	}

void MarketMaker::medi(const string & symbol, long long newPrice)
	{
		if (minHeaps[symbol].size() == maxHeaps[symbol].size())
		{
			if (newPrice<medians[symbol])
			{
				minHeaps[symbol].push(newPrice);
				medians[symbol] = minHeaps[symbol].top();
			}
			else
			{
				maxHeaps[symbol].push(newPrice);
				medians[symbol] = maxHeaps[symbol].top();
			}
		}
		else if (minHeaps[symbol].size()<maxHeaps[symbol].size())
		{
			if (newPrice<medians[symbol] || newPrice == medians[symbol])
			{
				minHeaps[symbol].push(newPrice);
				medians[symbol] = (minHeaps[symbol].top() + maxHeaps[symbol].top()) >> 1;
			}
			else
			{
				maxHeaps[symbol].push(newPrice);
				minHeaps[symbol].push(maxHeaps[symbol].top());
				maxHeaps[symbol].pop();
				medians[symbol] = (minHeaps[symbol].top() + maxHeaps[symbol].top()) >> 1;
			}
		}
		else
		{
			if (newPrice>medians[symbol] || newPrice == medians[symbol])
			{
				maxHeaps[symbol].push(newPrice);
				medians[symbol] = (minHeaps[symbol].top() + maxHeaps[symbol].top()) >> 1;
			}
			else
			{
				minHeaps[symbol].push(newPrice);
				maxHeaps[symbol].push(minHeaps[symbol].top());
				minHeaps[symbol].pop();
				medians[symbol] = (minHeaps[symbol].top() + maxHeaps[symbol].top()) >> 1;
			}
		}
	}
