#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>

using namespace std;

class Assembler
{

private:
    struct SymTabElem
    {
        string label;
        string section;
        int offset;
        bool local;
        int secSize;
        SymTabElem(string ll, string ss, int oo, bool lo)
        {
            label = ll;
            section = ss;
            offset = oo;
            local = lo;
            secSize = 0;
        }
    };

    struct SecTabElem
    {
        string instCode;
        string section;
        string position;
        
        SecTabElem(string i, string s, string p)
        {
            instCode = i;
            section = s;
            position = p;
        }
    };

    struct RelTabElem
    {
        string offset;
        int val;
        string type;
        string section;

        RelTabElem(string o, string t, int v, string s)
        {
            offset = o;
            type = t;
            val = v;
            section = s;
        }
    };

    int locCnt;
    string currSec;
    vector<SymTabElem *> symTab;
    vector<SecTabElem *> secTab;
    vector<RelTabElem *> relTab;

    int IsLabel(string &str);
    int IsDirective(string &str);
    int IsInstruction(string &str);
    int InsertIntoSymTab(const string &label, const string &section, int offset, bool local);

    int CodeInstruction(string &str);
    int CodeLabel(string &str);
    int CodeDirective(string &str);

    int GetSymbolLocal(const string &label) const;
    int GetSymbolID(const string &label) const;
    int GetSymbolOffset(const string &label) const;

    void SetSectionSize();
    void InsertIntoSymTab2Pass(const string &label);
    void SortSymTab();

    string OperandCodeLDR_STR(const string &str);
    string OperandCodeJMP(const string &str);
    string GetSymbolSection(const string &str) const;

    static void ToLower(string &str);
    static void Triming(string &str);
    static void RemoveComents(string &str);

    static int HexStringToInt(const string &str);
    static int IsRegister(const string &str);
    static int IsSymbol(const string &str);
    static int IsLiteral(const string &str);
    static int IsOperand(const string &str, int type);

    static string RegisterCode(const string &str);
    static string DecimalToHexString(int i);

public:
    Assembler();
    ~Assembler();

    Assembler(const Assembler &a) = delete;
    Assembler &operator=(const Assembler &a) = delete;

    int AsmLineFirstPass(string &str);
    int AsmLineSecondPass(string &str);

    int ParseFileName(const string &str, char c) const;

    void EndFirstPass();
    void PrintInOutputFile(char *fileName);
};