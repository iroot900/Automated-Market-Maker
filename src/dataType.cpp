#include "dataType.h"
Order::Order(long long timeStamp_, const string& clientName_, int side_, const string& equitySymbol_, long long  price_, long long quantity_, long long expiration_) :
timeStamp(timeStamp_), clientName(clientName_), side(side_), equitySymbol(equitySymbol_), price(price_), quantity(quantity_), expiration(expiration_)
{
	ID++;
}
long long Order::ID = 0;