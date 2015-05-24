# Market-Maker
A C++ program simulating Automated Market Making  
Simply run make and use command option -h or -help to see what this program could do

		"-h or -help    : print out help menu!" ;
		"-v or -verbose : disable print out each transaction!" ;
		"-m or -median    : disable print out median price for each equity!" ;
		"-p or -midpoint : disable print out midpoint for each equity!" ;
		"-t or -transfers : disable print out transaction details for each client!" ;
		"-w or -VWAP : disable print out volume weighted average price of the following equity!" ;
	     "-f or -filename : specify input filename, default one is  test.txt" ;
		"-d or -delimiter    : specify delimiter for input file, default one is space and tab " ;
		"-r or -randomOrder    : use possion process to generate random order flow instead of using the test.txt " ;

Take in buy and sell orders with differnt types (IOC, GTC..) for a variety of equities as they arrive. Generated by poisson process or predefined stream. Maintain order book to match buyers with sellers to execute trades as quickly as possible. Charge commisions while maintaining various market statistics(median, volume, money transfer, volume weighted average price) for all equities and participants. 
