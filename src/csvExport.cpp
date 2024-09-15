#include "csvExport.h"
using namespace std;
int main1()
{
    editFileIntoCSVFornat();

    return 0;
}

void editFileIntoCSVFornat()
{
    ifstream fileA("stockData.csv");
    ifstream fileB("epsSorted.csv");
    ofstream finalFile("data.csv");

    string line;

    while (getline(fileA, line))
        finalFile << line << endl;

    finalFile << endl;

    while (getline(fileB, line))
        finalFile << line << endl;

    fileA.close();
    fileB.close();
    finalFile.close();

    remove("stockData.csv");
    remove("epsSorted.csv");
    cout << "done." << endl;
}

void filesEndingWithStockToCSV(vector<string> files_names)
{
    for (int i = 0; i < files_names.size(); i++)
    {
        // std::fstream fs(files_names[i] + ".stock", std::fstream::in | std::fstream::out);
        // if (fs.is_open())
        // {
        //     while (!fs.eof())
        //         if (fs.get() == ' ')
        //         {
        //             fs.seekp((fs.tellp() - static_cast<streampos>(1)));
        //             fs.put(',');
        //             fs.seekp(fs.tellp());
        //         }
        //     fs.close();
        // }

        ifstream in(files_names[i] + ".stock", ios::in | ios::binary);

        ofstream out(files_names[i] + ".csv", ios::out | ios::binary);

        for (char c; in.get(c); out.put(c))
            if (c == ' ')
                c = ',';
    }
}
