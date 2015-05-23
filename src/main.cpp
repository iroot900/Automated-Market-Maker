#include "marketMaker.h"
int main(int argc, char** argv)
{
	MarketMaker marketMaker(argc, argv);
	cout << "Succefully Initiated!" << endl << endl;
	marketMaker.start();
}