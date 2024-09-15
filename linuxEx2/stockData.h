#pragma once
#include <iostream>
#include <filesystem>
#include <istream>
#include <fstream>
#include <sstream>
#include <stdio.h>
#include <string>
#include <vector>
#include <ctime>
#include <iomanip>
#include "utils.h"

using namespace std;

#pragma once
using std::ifstream;
using std::ios;
using std::cout;
using std::cin;
using std::endl;
using std::string;
using std::ofstream;

typedef struct stockDataNode {
	double open = 0;
	double high = 0;
	double low = 0;
	double close = 0;
	double volume = 0;
	string name;
	double eps = 0;
} stockDataNode;


void errorMsg(string msg, bool& flag);
void fetchStock(string stockName);
string printStockData(string stockName, string year);
string sumOneYearData(string year, stockDataNode& stockData);

string option_1or3(int option, bool& option1_done_flag, string stock_name);
void readLine(string line, stockDataNode& stockData);
double getOneParam(string delimiter, string& line, size_t& pos);
string printOneMonth(stockDataNode stockData, string year, int month);
void saveToFile(stockDataNode stockData, string year, int month, ofstream& fileA);

string option_2();
vector<string> fetch_stock_names_in_dir();
string extract_stock_name(string fileName);
bool has_suffix(const string &str, const string &suffix);

string percision(double num);
