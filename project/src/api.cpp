#include "../inc/api.h"

extern void secondPass(char *command, char *inputFileName, char *outputFileName, Assembler *a);

void firstPass(int numOfArg, char *inputFileName, char *command, char *outputFileName, Assembler *a)
{
    if (numOfArg != 4)
    {
        cout << "Treba navesti 3 argumenta od kojih je prvi argument komanda,";
        cout << "drugi argument odredisni fajl, a treci argument izvorisni fajl" << endl;
        return;
    }

    string cmdName = (command);
    if (cmdName != "-o")
    {
        cout << "Ne odgovarajuca komanda. Mora se navesti -o kao komanda" << endl;
        return;
    }

    string filNme = (inputFileName);

    if (a->ParseFileName(filNme, 's') <= 0)
    {
        cout << "Izvorisni fajl mora imati ekstenziju .s" << endl;
        return;
    }

    filNme = (outputFileName);
    if (a->ParseFileName(filNme, 'o') <= 0)
    {
        cout << "Odredistni fajl mora imati ekstenziju .o" << endl;
        return;
    }

    int lineNum = 0;
    ifstream input(inputFileName);
    if (input.is_open())
    {
        string line;
        int pass = 0;
        while (getline(input, line) && pass == 0)
        {
            pass = a->AsmLineFirstPass(line);
            lineNum++;
        }
        input.close();
        if (pass < 0)
        {
            cout << "Greska na liniji " << lineNum << endl;

            return;
        }
        
        a->EndFirstPass();

        secondPass(command, inputFileName, outputFileName, a);
    }

    else
    {
        cout << "Fajl se nije otvorio" << endl;
    }
}

void secondPass(char *command, char *inputFileName, char *outputFileName, Assembler *a)
{
    int lineNum = 0;
    ifstream input(inputFileName);
    if (input.is_open())
    {
        string line;
        int pass = 0;
        while (getline(input, line) && pass == 0)
        {
            pass = a->AsmLineSecondPass(line);
            lineNum++;
        }
        input.close();
        
        a->PrintInOutputFile(outputFileName);
    }

    else
    {
        cout << "Fajl se nije otvorio" << endl;
    }
}

void assembler(int numOfArg, char *command, char *inputFileName, char *outputFileName)
{
    Assembler *a = new Assembler();
    firstPass(numOfArg, inputFileName, command, outputFileName, a);
    delete a;
}