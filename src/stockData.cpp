#include "stockData.h"
#include <dirent.h>
#include "utils.h"

string option_1or3(int option, bool &option1_done_flag, string stock_name)
{
	string buffer = "";
	string curr_year = "2022";

	if (option == 1)
	{
		try
		{
			fetchStock(stock_name);
			option1_done_flag = true;
			buffer = "Fetch " + stock_name + " DONE\n";
		}
		catch (const exception &e)
		{
			cout << e.what() << '\n';
			return nullptr;
		}
	}
	else if (option == 3)
		buffer += printStockData(stock_name, curr_year);
	else
		throw "invalid command";

	return buffer;
}

void errorMsg(string msg, bool &flag)
{
	flag = false;
	cout << msg << endl;
}

void fetchStock(string stockName)
{
	string commandScript = "./get_stocks_data.sh ";
	commandScript = commandScript + stockName;
	int exitStat = system(commandScript.c_str());

	if (exitStat == 1)
		throw "cant fetch stock";
}

string printStockData(string stockName, string year)
{
	string buffer = "";
	int curr_year = stoi(year);
	for (int i = 0; i < 3; i++)
	{
		curr_year -= i;
		year = to_string(curr_year);

		stockDataNode stockData;
		stockData.name = stockName;
		buffer += sumOneYearData(year, stockData);
	}
	return buffer;
}

string sumOneYearData(string year, stockDataNode &stockData)
{
	string buffer = "";
	ifstream fStockData, fStockEps;
	ofstream fileA("stocksData.csv");

	fStockData.open(stockData.name + ".stock");
	fStockEps.open(stockData.name + ".eps");
	vector<stockDataNode> data;

	string delimiter = "-", line;
	size_t posYear;
	int currMonth = 0, prevMonth = 0;
	bool foundYear = false, isFirstIter = true;
	double yearlyEps = stockData.eps = get1YearEps(stockData.name, year);
	buffer += "stock " + stockData.name + " year " + year + " data: " + '\n';
	do
	{
		getline(fStockData, line);
		posYear = line.find(delimiter);

		string token = line.substr(0, posYear);
		if (year.compare(token) != 0 && prevMonth == 0)
			continue;

		else
		{ // correct year
			string monthToken;
			size_t posMonth = posYear;
			line.erase(0, posMonth + delimiter.length());
			posYear = line.find(delimiter);	 // advence to month (next mm-dd)
			token = line.substr(0, posYear); // tokenYear gets month

			if (stoi(token) != currMonth && !isFirstIter)
			{
				buffer += printOneMonth(stockData, year, currMonth);
				saveToFile(stockData, year, currMonth, fileA);

				stockData = stockDataNode();
				stockData.eps = yearlyEps;
			}
			currMonth = stoi(token);

			if (!isFirstIter)
			{
				if (currMonth == 12 && prevMonth == 1) // last month
					break;							   // endloop
			}
			else
				isFirstIter = false;

			prevMonth = currMonth;
			readLine(line, stockData);
		}
	} while ((posYear = line.find(delimiter)) != string::npos && foundYear == false && !fStockData.eof());

	fStockData.close();
	return buffer;
}

// format: date Y\M\D 2022-03-18 open 206.7000 high 216.8000 low 206.0000 close 216.4900  valume 51235040
void readLine(string line, stockDataNode &stockData)
{
	string delimiter = " ";
	size_t pos = 0;
	pos = line.find(delimiter);
	line.erase(0, pos + delimiter.length()); // delete date

	pos = line.find(delimiter);
	stockData.open += getOneParam(delimiter, line, pos);
	stockData.high += getOneParam(delimiter, line, pos);
	stockData.low += getOneParam(delimiter, line, pos);
	stockData.close += getOneParam(delimiter, line, pos);
	stockData.volume += getOneParam(delimiter, line, pos);
}

double getOneParam(string delimiter, string &line, size_t &pos)
{
	string token = line.substr(0, pos);
	line.erase(0, pos + delimiter.length());
	pos = line.find(delimiter);
	return stod(token);
}

string option_2()
{
	string buffer = "";
	vector<string> names = fetch_stock_names_in_dir();
	buffer += "Available stocks: ";
	for (int i = 0; i < names.size(); i++)
		buffer += names[i] + ' ';
	buffer += '\n';

	return buffer;
}
vector<string> fetch_stock_names_in_dir()
{
	vector<string> stock_names;

	DIR *dir;
	struct dirent *pointToName;

	string temp;

	dir = opendir("."); // open all directory
	if (dir)
	{
		while ((pointToName = readdir(dir)) != NULL)
		{
			temp = string(pointToName->d_name);
			if (has_suffix(temp, ".stock"))							  // if sstring file ends with .stock
				stock_names.push_back(temp.erase(temp.length() - 6)); // it's a valid stock, insert to names
		}
		closedir(dir); // close all directory
	}
	return stock_names;
}

bool has_suffix(const string &str, const string &suffix)
{
	return str.size() >= suffix.size() &&
		   str.compare(str.size() - suffix.size(), suffix.size(), suffix) == 0;
}

string percision(double num)
{
	int whole_part = int(num);

	string after_dec_point;
	after_dec_point = to_string(num - whole_part);

	return to_string(whole_part) + '.' + after_dec_point.substr(2, 4);
}

string printOneMonth(stockDataNode stockData, string year, int month)
{
	string buffer = "";
	buffer += " ";
	if (month < 10)
		buffer += '0';
	buffer += to_string(month) + ": { 1. open: " + percision(stockData.open) +
			  ", 2. high: " + percision(stockData.high) +
			  ", 3. low: " + percision(stockData.low) +
			  ", 4. close: " + percision(stockData.close) +
			  ", 5. volume: " + percision(stockData.volume) +
			  ", 6. reportedEPS: " + percision(stockData.eps) + " }" + '\n';
	return buffer;
}
void saveToFile(stockDataNode stockData, string year, int month, ofstream &fileA)
{
	fileA << " ";
	if (month < 10)
		fileA << '0';
	fileA << month << ", " << fixed << std::setprecision(4) << stockData.open << ", " << fixed << std::setprecision(4) << stockData.high << ", " << fixed << std::setprecision(4) << stockData.low << ", " << fixed << std::setprecision(4) << stockData.close << ", " << fixed << std::setprecision(0) << stockData.volume << ", " << fixed << std::setprecision(4) << stockData.eps << endl;
}
