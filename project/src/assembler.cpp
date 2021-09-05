#include "../inc/assembler.h"

Assembler::Assembler()
{
    this->locCnt = 0;
    this->symTab.push_back(new SymTabElem("", "", 0, false));
    this->currSec = "";
}

Assembler::~Assembler()
{
    for (SymTabElem *elem : this->symTab)
    {
        delete elem;
    }

    for (SecTabElem *elem : this->secTab)
    {
        delete elem;
    }

    for (RelTabElem *elem : this->relTab)
    {
        delete elem;
    }
    
    this->symTab.clear();
    this->secTab.clear();
    this->relTab.clear();
}

void Assembler::ToLower(string &str)
{
    for (int i = 0; i < str.length(); i++)
    {
        str[i] = tolower(str[i]);
    }
}

void Assembler::Triming(string &str)
{
    int i;
    for (i = 0; i < str.length(); i++)
    {
        if (!isspace(str[i]))
        {
            break;
        }
    }
    if (i > 0)
    {
        str.erase(0, i);
    }
    for (i = str.length() - 1; i >= 0; i--)
    {
        if (!isspace(str[i]))
        {
            break;
        }
    }
    if (i < str.length() - 1)
    {
        str.erase(i + 1);
    }
}

void Assembler::RemoveComents(string &str)
{
    (str.find('#') != string::npos) ? str.erase(str.find('#')) : "";
}

int Assembler::AsmLineFirstPass(string &str)
{
    Assembler::RemoveComents(str);
    Assembler::Triming(str);

    if (str == "")
    {
        return 0;
    }

    if (str.find(":") != string::npos)
    {
        return this->IsLabel(str);
    }

    else if (str.find(".") == 0)
    {
        return this->IsDirective(str);
    }

    else
    {
        return this->IsInstruction(str);
    }
}

void Assembler::SortSymTab()
{
    vector<SymTabElem *> sections;
    vector<SymTabElem *> defSymb;
    vector<SymTabElem *> unDefSymb;

    for (SymTabElem *elem : this->symTab)
    {
        if (elem->section == elem->label)
        {
            sections.push_back(elem);
            continue;
        }
        if (elem->section != "")
        {
            defSymb.push_back(elem);
            continue;
        }
        if (elem->section == "")
        {
            unDefSymb.push_back(elem);
            continue;
        }
    }

    for (SymTabElem *elem : this->symTab)
    {
        this->symTab.pop_back();
    }

    for (SymTabElem *elem : sections)
    {
        this->symTab.push_back(elem);
    }

    for (SymTabElem *elem : defSymb)
    {
        this->symTab.push_back(elem);
    }

    for (SymTabElem *elem : unDefSymb)
    {
        this->symTab.push_back(elem);
    }

    for (SymTabElem *elem : sections)
    {
        sections.pop_back();
    }

    for (SymTabElem *elem : defSymb)
    {
        defSymb.pop_back();
    }

    for (SymTabElem *elem : unDefSymb)
    {
        unDefSymb.pop_back();
    }
}

void Assembler::EndFirstPass()
{
    this->SortSymTab();
    this->SetSectionSize();
    this->locCnt = 0;
    this->currSec = "";
}

int Assembler::IsDirective(string &str)
{
    string tmp = str;
    if (tmp.find(' ') == string::npos)
    {
        Assembler::ToLower(tmp);
        if (tmp == ".end")
        {
            return 1;
        }
        else
        {
            cout << "Ne postojeca direktiva" << endl;
            return -1;
        }
    }
    else
    {
        char *buff = new char[tmp.find(' ')];
        size_t len = tmp.copy(buff, (tmp.find(' ')), 0);
        buff[len] = '\0';
        string tmp1(buff);
        delete buff;
        Assembler::ToLower(tmp1);

        if (tmp1 == ".global")
        {
            buff = new char[tmp.size() - tmp.find(' ') - 1];
            len = tmp.copy(buff, (tmp.size() - tmp.find(' ') - 1), (tmp.find(' ') + 1));
            buff[len] = '\0';
            string symLst(buff);
            delete buff;
            Assembler::Triming(symLst);
            if (symLst == "")
            {
                cout << "Niste naveli argumente" << endl;
                return -1;
            }

            do
            {
                Assembler::Triming(symLst);
                if (symLst.find(',') == string::npos)
                {
                    if (Assembler::IsSymbol(symLst) == 0)
                    {
                        //Dodati simbol u listu simbola
                        if (this->InsertIntoSymTab(symLst, "", 0, false) == -1)
                        {
                            cout << "Vec postoji simbol sa datim nazivom" << endl;
                            return -1;
                        }

                        return 0;
                    }
                    else
                    {
                        cout << "Lose ste nazvali simbol" << endl;
                        return -1;
                    }
                }
                else
                {
                    if (symLst.find(',') == symLst.size() - 1 ||
                        symLst.find(',') == 0)
                    {
                        cout << "Lose ste nazvali simbol" << endl;
                        return -1;
                    }
                    buff = new char[symLst.find(',')];
                    len = symLst.copy(buff, symLst.find(','), 0);
                    buff[len] = '\0';
                    string label(buff);
                    delete buff;
                    Assembler::Triming(label);
                    if (Assembler::IsSymbol(label) == -1)
                    {
                        cout << "Lose ste nazvli simbol" << endl;
                        return -1;
                    }
                    //Dodati simbol u listu simbola
                    if (this->InsertIntoSymTab(label, "", 0, false) == -1)
                    {
                        cout << "Vec postoji simbol sa datim nazivom" << endl;
                        return -1;
                    }
                    symLst.erase(0, symLst.find(',') + 1);
                }
            } while (symLst != "");
            return 0;
        }

        if (tmp1 == ".extern")
        {
            buff = new char[tmp.size() - tmp.find(' ') - 1];
            len = tmp.copy(buff, (tmp.size() - tmp.find(' ') - 1), (tmp.find(' ') + 1));
            buff[len] = '\0';
            string symLst(buff);
            delete buff;
            Assembler::Triming(symLst);
            if (symLst == "")
            {
                cout << "Niste naveli argumente" << endl;
                return -1;
            }

            do
            {
                Assembler::Triming(symLst);
                if (symLst.find(',') == string::npos)
                {
                    if (Assembler::IsSymbol(symLst) == 0)
                    {
                        if (this->InsertIntoSymTab(symLst, "", 0, false) == -1)
                        {
                            cout << "Vec postoji simbol sa datim nazivom" << endl;
                            return -1;
                        }

                        return 0;
                    }
                    else
                    {
                        cout << "Lose ste nazvali simbol" << endl;
                        return -1;
                    }
                }
                else
                {
                    if (symLst.find(',') == symLst.size() - 1 ||
                        symLst.find(',') == 0)
                    {
                        cout << "Lose ste nazvali simbol" << endl;
                        return -1;
                    }
                    buff = new char[symLst.find(',')];
                    len = symLst.copy(buff, symLst.find(','), 0);
                    buff[len] = '\0';
                    string label(buff);
                    delete buff;
                    Assembler::Triming(label);
                    if (Assembler::IsSymbol(label) == -1)
                    {
                        cout << "Lose ste nazvli simbol" << endl;
                        return -1;
                    }

                    if (this->InsertIntoSymTab(label, "", 0, false) == -1)
                    {
                        cout << "Vec postoji simbol sa datim nazivom" << endl;
                        return -1;
                    }
                    symLst.erase(0, symLst.find(',') + 1);
                }
            } while (symLst != "");
            return 0;
        }

        if (tmp1 == ".section")
        {
            this->SetSectionSize();
            buff = new char[tmp.size() - tmp.find(' ') - 1];
            len = tmp.copy(buff, (tmp.size() - tmp.find(' ') - 1), (tmp.find(' ') + 1));
            buff[len] = '\0';
            string sectionName(buff);
            delete buff;

            if (Assembler::IsSymbol(sectionName) == -1)
            {
                cout << "Lose ste nazvali sekciju" << endl;
                return -1;
            }

            if (this->InsertIntoSymTab(sectionName, sectionName, 0, true) == -1)
            {
                cout << "Vec postoji simbol sa datim nazivom" << endl;
                return -1;
            }
            this->currSec = sectionName;
            this->locCnt = 0;
            return 0;
        }

        if (tmp1 == ".word")
        {
            buff = new char[tmp.size() - tmp.find(' ') - 1];
            len = tmp.copy(buff, (tmp.size() - tmp.find(' ') - 1), (tmp.find(' ') + 1));
            buff[len] = '\0';
            string symLst(buff);
            delete buff;
            Assembler::Triming(symLst);
            if (symLst == "")
            {
                cout << "Niste naveli argumente" << endl;
                return -1;
            }

            do
            {
                Assembler::Triming(symLst);
                if (symLst.find(',') == string::npos)
                {
                    if (Assembler::IsLiteral(symLst) == 0)
                    {
                        this->locCnt += 2;
                        return 0;
                    }
                    else if (Assembler::IsSymbol(symLst) == 0)
                    {
                        this->locCnt += 2;
                        return 0;
                    }
                    else
                    {
                        cout << "Lose ste nazvali simbol" << endl;
                        return -1;
                    }
                }
                else
                {
                    if (symLst.find(',') == symLst.size() - 1 ||
                        symLst.find(',') == 0)
                    {
                        cout << "Lose ste nazvali simbol" << endl;
                        return -1;
                    }
                    buff = new char[symLst.find(',')];
                    len = symLst.copy(buff, symLst.find(','), 0);
                    buff[len] = '\0';
                    string label(buff);
                    delete buff;
                    Assembler::Triming(label);
                    if (Assembler::IsSymbol(label) == 0)
                    {
                        symLst.erase(0, symLst.find(',') + 1);
                        this->locCnt += 2;
                        continue;
                    }
                    if (Assembler::IsLiteral(label) == 0)
                    {
                        symLst.erase(0, symLst.find(',') + 1);
                        this->locCnt += 2;
                        continue;
                    }
                    return -1;
                }
            } while (symLst != "");
            return 0;
        }

        if (tmp1 == ".skip")
        {
            buff = new char[tmp.size() - tmp.find(' ') - 1];
            len = tmp.copy(buff, (tmp.size() - tmp.find(' ') - 1), (tmp.find(' ') + 1));
            buff[len] = '\0';
            string literal(buff);
            delete buff;
            Assembler::Triming(literal);
            if (Assembler::IsLiteral(literal) == 0)
            {
                if (literal.find('x') == string::npos)
                {
                    int num = stoi(literal);
                    this->locCnt += num;
                }
                else
                {
                    int num = Assembler::HexStringToInt(literal);
                    this->locCnt += num;
                }
                return 0;
            }
            cout << "Ne ispravno unet literal" << endl;
            return -1;
        }

        if (tmp1 == ".equ")
        {
            buff = new char[tmp.size() - tmp.find(' ') - 1];
            len = tmp.copy(buff, (tmp.size() - tmp.find(' ') - 1), (tmp.find(' ') + 1));
            buff[len] = '\0';
            string arg(buff);
            delete buff;
            if (arg.find(',') == string::npos)
            {
                cout << "Nepravilno zadati argumenti" << endl;
                return -1;
            }

            string symbol;
            string literal;
            buff = new char[arg.find(',')];
            len = arg.copy(buff, (arg.find(',')), 0);
            buff[len] = '\0';
            symbol = (buff);
            delete buff;
            Assembler::Triming(symbol);

            buff = new char[arg.size() - arg.find(',') - 1];
            len = arg.copy(buff, (arg.size() - arg.find(',') - 1), (arg.find(',') + 1));
            buff[len] = '\0';
            literal = (buff);
            delete buff;
            Assembler::Triming(literal);

            if (Assembler::IsSymbol(symbol) == -1 || Assembler::IsLiteral(literal) == -1)
            {
                cout << "Uneli ste lose argumenta" << endl;
                return -1;
            }

            int num;
            num = stoi(literal);

            if (literal.find('x') != string::npos)
            {
                num = Assembler::HexStringToInt(literal);
            }

            if (this->InsertIntoSymTab(symbol, "*ABS*", num, true) == -1)
            {
                cout << "Vec postoji simbol sa datim nazivom" << endl;
                return -1;
            }

            return 0;
        }
    }
    cout << "Ne postojeca direktiva" << endl;
    return -1;
}

int Assembler::IsInstruction(string &str)
{
    string tmp = str;
    if (str.find(' ') == string::npos)
    {
        Assembler::ToLower(tmp);
        if (tmp == "halt")
        {
            this->locCnt++;
            return 0;
        }

        if (tmp == "iret")
        {
            this->locCnt++;
            return 0;
        }

        if (tmp == "ret")
        {
            this->locCnt++;
            return 0;
        }

        cout << "Ne postojeca instrukcija" << endl;
        return -1;
    }
    else
    {
        char *buff = new char[tmp.find(' ')];
        size_t len = tmp.copy(buff, (tmp.find(' ')), 0);
        buff[len] = '\0';
        string tmp1(buff);
        delete buff;
        Assembler::ToLower(tmp1);

        if (tmp1 == "int")
        {
            char *buff = new char[tmp.size() - tmp.find(' ') - 1];
            size_t len = tmp.copy(buff, (tmp.size() - tmp.find(' ') - 1), (tmp.find(' ') + 1));
            buff[len] = '\0';
            string regD(buff);
            delete buff;
            Assembler::Triming(regD);
            if (Assembler::IsRegister(regD) == -1)
            {
                cout << "Morate da navedete 1 od korisnicki dostupnih registara";
                cout << "kao argument instrukcije" << endl;
                return -1;
            }
            this->locCnt += 2;
            return 0;
        }

        if (tmp1 == "push")
        {
            char *buff = new char[tmp.size() - tmp.find(' ') - 1];
            size_t len = tmp.copy(buff, (tmp.size() - tmp.find(' ') - 1), (tmp.find(' ') + 1));
            buff[len] = '\0';
            string regD(buff);
            delete buff;
            Assembler::Triming(regD);
            if (Assembler::IsRegister(regD) == -1)
            {
                cout << "Morate da navedete 1 od korisnicki dostupnih registara";
                cout << "kao argument instrukcije" << endl;
                return -1;
            }
            this->locCnt += 3;
            return 0;
        }

        if (tmp1 == "pop")
        {
            char *buff = new char[tmp.size() - tmp.find(' ') - 1];
            size_t len = tmp.copy(buff, (tmp.size() - tmp.find(' ') - 1), (tmp.find(' ') + 1));
            buff[len] = '\0';
            string regD(buff);
            delete buff;
            Assembler::Triming(regD);
            if (Assembler::IsRegister(regD) == -1)
            {
                cout << "Morate da navedete 1 od korisnicki dostupnih registara";
                cout << "kao argument instrukcije" << endl;
                return -1;
            }
            this->locCnt += 3;
            return 0;
        }

        if (tmp1 == "not")
        {
            char *buff = new char[tmp.size() - tmp.find(' ') - 1];
            size_t len = tmp.copy(buff, (tmp.size() - tmp.find(' ') - 1), (tmp.find(' ') + 1));
            buff[len] = '\0';
            string regD(buff);
            delete buff;
            Assembler::Triming(regD);
            if (Assembler::IsRegister(regD) == -1)
            {
                cout << "Morate da navedete 1 od korisnicki dostupnih registara";
                cout << "kao argument instrukcije" << endl;
                return -1;
            }
            this->locCnt += 2;
            return 0;
        }

        if (tmp1 == "xchg")
        {
            char *buff = new char[tmp.size() - tmp.find(' ') - 1];
            size_t len = tmp.copy(buff, (tmp.size() - tmp.find(' ') - 1), (tmp.find(' ') + 1));
            buff[len] = '\0';
            tmp1 = (buff);
            delete buff;

            if (tmp1.find(',') == string::npos)
            {
                cout << "Nepravilno zadati argumenti" << endl;
                return -1;
            }
            string regD;
            string regS;

            buff = new char[tmp1.find(',')];
            len = tmp1.copy(buff, (tmp1.find(',')), 0);
            buff[len] = '\0';
            regD = (buff);
            delete buff;
            Assembler::Triming(regD);

            buff = new char[tmp1.size() - tmp1.find(',') - 1];
            len = tmp1.copy(buff, (tmp1.size() - tmp1.find(',') - 1), (tmp1.find(',') + 1));
            buff[len] = '\0';
            regS = (buff);
            delete buff;
            Assembler::Triming(regS);

            if (Assembler::IsRegister(regD) == -1 || Assembler::IsRegister(regS) == -1)
            {
                cout << "Nepravilno uneti argumenti" << endl;
                return -1;
            }
            this->locCnt += 2;
            return 0;
        }

        if (tmp1 == "add")
        {
            char *buff = new char[tmp.size() - tmp.find(' ') - 1];
            size_t len = tmp.copy(buff, (tmp.size() - tmp.find(' ') - 1), (tmp.find(' ') + 1));
            buff[len] = '\0';
            tmp1 = (buff);
            delete buff;

            if (tmp1.find(',') == string::npos)
            {
                cout << "Nepravilno zadati argumenti" << endl;
                return -1;
            }
            string regD;
            string regS;

            buff = new char[tmp1.find(',')];
            len = tmp1.copy(buff, (tmp1.find(',')), 0);
            buff[len] = '\0';
            regD = (buff);
            delete buff;
            Assembler::Triming(regD);

            buff = new char[tmp1.size() - tmp1.find(',') - 1];
            len = tmp1.copy(buff, (tmp1.size() - tmp1.find(',') - 1), (tmp1.find(',') + 1));
            buff[len] = '\0';
            regS = (buff);
            delete buff;
            Assembler::Triming(regS);

            if (Assembler::IsRegister(regD) == -1 || Assembler::IsRegister(regS) == -1)
            {
                cout << "Nepravilno uneti argumenti" << endl;
                return -1;
            }
            this->locCnt += 2;
            return 0;
        }

        if (tmp1 == "sub")
        {
            char *buff = new char[tmp.size() - tmp.find(' ') - 1];
            size_t len = tmp.copy(buff, (tmp.size() - tmp.find(' ') - 1), (tmp.find(' ') + 1));
            buff[len] = '\0';
            tmp1 = (buff);
            delete buff;

            if (tmp1.find(',') == string::npos)
            {
                cout << "Nepravilno zadati argumenti" << endl;
                return -1;
            }
            string regD;
            string regS;

            buff = new char[tmp1.find(',')];
            len = tmp1.copy(buff, (tmp1.find(',')), 0);
            buff[len] = '\0';
            regD = (buff);
            delete buff;
            Assembler::Triming(regD);

            buff = new char[tmp1.size() - tmp1.find(',') - 1];
            len = tmp1.copy(buff, (tmp1.size() - tmp1.find(',') - 1), (tmp1.find(',') + 1));
            buff[len] = '\0';
            regS = (buff);
            delete buff;
            Assembler::Triming(regS);

            if (Assembler::IsRegister(regD) == -1 || Assembler::IsRegister(regS) == -1)
            {
                cout << "Nepravilno uneti argumenti" << endl;
                return -1;
            }
            this->locCnt += 2;
            return 0;
        }

        if (tmp1 == "mul")
        {
            char *buff = new char[tmp.size() - tmp.find(' ') - 1];
            size_t len = tmp.copy(buff, (tmp.size() - tmp.find(' ') - 1), (tmp.find(' ') + 1));
            buff[len] = '\0';
            tmp1 = (buff);
            delete buff;

            if (tmp1.find(',') == string::npos)
            {
                cout << "Nepravilno zadati argumenti" << endl;
                return -1;
            }
            string regD;
            string regS;

            buff = new char[tmp1.find(',')];
            len = tmp1.copy(buff, (tmp1.find(',')), 0);
            buff[len] = '\0';
            regD = (buff);
            delete buff;
            Assembler::Triming(regD);

            buff = new char[tmp1.size() - tmp1.find(',') - 1];
            len = tmp1.copy(buff, (tmp1.size() - tmp1.find(',') - 1), (tmp1.find(',') + 1));
            buff[len] = '\0';
            regS = (buff);
            delete buff;
            Assembler::Triming(regS);

            if (Assembler::IsRegister(regD) == -1 || Assembler::IsRegister(regS) == -1)
            {
                cout << "Nepravilno uneti argumenti" << endl;
                return -1;
            }
            this->locCnt += 2;
            return 0;
        }

        if (tmp1 == "div")
        {
            char *buff = new char[tmp.size() - tmp.find(' ') - 1];
            size_t len = tmp.copy(buff, (tmp.size() - tmp.find(' ') - 1), (tmp.find(' ') + 1));
            buff[len] = '\0';
            tmp1 = (buff);
            delete buff;

            if (tmp1.find(',') == string::npos)
            {
                cout << "Nepravilno zadati argumenti" << endl;
                return -1;
            }
            string regD;
            string regS;

            buff = new char[tmp1.find(',')];
            len = tmp1.copy(buff, (tmp1.find(',')), 0);
            buff[len] = '\0';
            regD = (buff);
            delete buff;
            Assembler::Triming(regD);

            buff = new char[tmp1.size() - tmp1.find(',') - 1];
            len = tmp1.copy(buff, (tmp1.size() - tmp1.find(',') - 1), (tmp1.find(',') + 1));
            buff[len] = '\0';
            regS = (buff);
            delete buff;
            Assembler::Triming(regS);

            if (Assembler::IsRegister(regD) == -1 || Assembler::IsRegister(regS) == -1)
            {
                cout << "Nepravilno uneti argumenti" << endl;
                return -1;
            }
            this->locCnt += 2;
            return 0;
        }

        if (tmp1 == "cmp")
        {
            char *buff = new char[tmp.size() - tmp.find(' ') - 1];
            size_t len = tmp.copy(buff, (tmp.size() - tmp.find(' ') - 1), (tmp.find(' ') + 1));
            buff[len] = '\0';
            tmp1 = (buff);
            delete buff;

            if (tmp1.find(',') == string::npos)
            {
                cout << "Nepravilno zadati argumenti" << endl;
                return -1;
            }
            string regD;
            string regS;

            buff = new char[tmp1.find(',')];
            len = tmp1.copy(buff, (tmp1.find(',')), 0);
            buff[len] = '\0';
            regD = (buff);
            delete buff;
            Assembler::Triming(regD);

            buff = new char[tmp1.size() - tmp1.find(',') - 1];
            len = tmp1.copy(buff, (tmp1.size() - tmp1.find(',') - 1), (tmp1.find(',') + 1));
            buff[len] = '\0';
            regS = (buff);
            delete buff;
            Assembler::Triming(regS);

            if (Assembler::IsRegister(regD) == -1 || Assembler::IsRegister(regS) == -1)
            {
                cout << "Nepravilno uneti argumenti" << endl;
                return -1;
            }
            this->locCnt += 2;
            return 0;
        }

        if (tmp1 == "and")
        {
            char *buff = new char[tmp.size() - tmp.find(' ') - 1];
            size_t len = tmp.copy(buff, (tmp.size() - tmp.find(' ') - 1), (tmp.find(' ') + 1));
            buff[len] = '\0';
            tmp1 = (buff);
            delete buff;

            if (tmp1.find(',') == string::npos)
            {
                cout << "Nepravilno zadati argumenti" << endl;
                return -1;
            }
            string regD;
            string regS;

            buff = new char[tmp1.find(',')];
            len = tmp1.copy(buff, (tmp1.find(',')), 0);
            buff[len] = '\0';
            regD = (buff);
            delete buff;
            Assembler::Triming(regD);

            buff = new char[tmp1.size() - tmp1.find(',') - 1];
            len = tmp1.copy(buff, (tmp1.size() - tmp1.find(',') - 1), (tmp1.find(',') + 1));
            buff[len] = '\0';
            regS = (buff);
            delete buff;
            Assembler::Triming(regS);

            if (Assembler::IsRegister(regD) == -1 || Assembler::IsRegister(regS) == -1)
            {
                cout << "Nepravilno uneti argumenti" << endl;
                return -1;
            }
            this->locCnt += 2;
            return 0;
        }

        if (tmp1 == "or")
        {
            char *buff = new char[tmp.size() - tmp.find(' ') - 1];
            size_t len = tmp.copy(buff, (tmp.size() - tmp.find(' ') - 1), (tmp.find(' ') + 1));
            buff[len] = '\0';
            tmp1 = (buff);
            delete buff;

            if (tmp1.find(',') == string::npos)
            {
                cout << "Nepravilno zadati argumenti" << endl;
                return -1;
            }
            string regD;
            string regS;

            buff = new char[tmp1.find(',')];
            len = tmp1.copy(buff, (tmp1.find(',')), 0);
            buff[len] = '\0';
            regD = (buff);
            delete buff;
            Assembler::Triming(regD);

            buff = new char[tmp1.size() - tmp1.find(',') - 1];
            len = tmp1.copy(buff, (tmp1.size() - tmp1.find(',') - 1), (tmp1.find(',') + 1));
            buff[len] = '\0';
            regS = (buff);
            delete buff;
            Assembler::Triming(regS);

            if (Assembler::IsRegister(regD) == -1 || Assembler::IsRegister(regS) == -1)
            {
                cout << "Nepravilno uneti argumenti" << endl;
                return -1;
            }
            this->locCnt += 2;
            return 0;
        }

        if (tmp1 == "xor")
        {
            char *buff = new char[tmp.size() - tmp.find(' ') - 1];
            size_t len = tmp.copy(buff, (tmp.size() - tmp.find(' ') - 1), (tmp.find(' ') + 1));
            buff[len] = '\0';
            tmp1 = (buff);
            delete buff;

            if (tmp1.find(',') == string::npos)
            {
                cout << "Nepravilno zadati argumenti" << endl;
                return -1;
            }
            string regD;
            string regS;

            buff = new char[tmp1.find(',')];
            len = tmp1.copy(buff, (tmp1.find(',')), 0);
            buff[len] = '\0';
            regD = (buff);
            delete buff;
            Assembler::Triming(regD);

            buff = new char[tmp1.size() - tmp1.find(',') - 1];
            len = tmp1.copy(buff, (tmp1.size() - tmp1.find(',') - 1), (tmp1.find(',') + 1));
            buff[len] = '\0';
            regS = (buff);
            delete buff;
            Assembler::Triming(regS);

            if (Assembler::IsRegister(regD) == -1 || Assembler::IsRegister(regS) == -1)
            {
                cout << "Nepravilno uneti argumenti" << endl;
                return -1;
            }
            this->locCnt += 2;
            return 0;
        }

        if (tmp1 == "test")
        {
            char *buff = new char[tmp.size() - tmp.find(' ') - 1];
            size_t len = tmp.copy(buff, (tmp.size() - tmp.find(' ') - 1), (tmp.find(' ') + 1));
            buff[len] = '\0';
            tmp1 = (buff);
            delete buff;

            if (tmp1.find(',') == string::npos)
            {
                cout << "Nepravilno zadati argumenti" << endl;
                return -1;
            }
            string regD;
            string regS;

            buff = new char[tmp1.find(',')];
            len = tmp1.copy(buff, (tmp1.find(',')), 0);
            buff[len] = '\0';
            regD = (buff);
            delete buff;
            Assembler::Triming(regD);

            buff = new char[tmp1.size() - tmp1.find(',') - 1];
            len = tmp1.copy(buff, (tmp1.size() - tmp1.find(',') - 1), (tmp1.find(',') + 1));
            buff[len] = '\0';
            regS = (buff);
            delete buff;
            Assembler::Triming(regS);

            if (Assembler::IsRegister(regD) == -1 || Assembler::IsRegister(regS) == -1)
            {
                cout << "Nepravilno uneti argumenti" << endl;
                return -1;
            }
            this->locCnt += 2;
            return 0;
        }

        if (tmp1 == "shl")
        {
            char *buff = new char[tmp.size() - tmp.find(' ') - 1];
            size_t len = tmp.copy(buff, (tmp.size() - tmp.find(' ') - 1), (tmp.find(' ') + 1));
            buff[len] = '\0';
            tmp1 = (buff);
            delete buff;

            if (tmp1.find(',') == string::npos)
            {
                cout << "Nepravilno zadati argumenti" << endl;
                return -1;
            }
            string regD;
            string regS;

            buff = new char[tmp1.find(',')];
            len = tmp1.copy(buff, (tmp1.find(',')), 0);
            buff[len] = '\0';
            regD = (buff);
            delete buff;
            Assembler::Triming(regD);

            buff = new char[tmp1.size() - tmp1.find(',') - 1];
            len = tmp1.copy(buff, (tmp1.size() - tmp1.find(',') - 1), (tmp1.find(',') + 1));
            buff[len] = '\0';
            regS = (buff);
            delete buff;
            Assembler::Triming(regS);

            if (Assembler::IsRegister(regD) == -1 || Assembler::IsRegister(regS) == -1)
            {
                cout << "Nepravilno uneti argumenti" << endl;
                return -1;
            }
            this->locCnt += 2;
            return 0;
        }

        if (tmp1 == "shr")
        {
            char *buff = new char[tmp.size() - tmp.find(' ') - 1];
            size_t len = tmp.copy(buff, (tmp.size() - tmp.find(' ') - 1), (tmp.find(' ') + 1));
            buff[len] = '\0';
            tmp1 = (buff);
            delete buff;

            if (tmp1.find(',') == string::npos)
            {
                cout << "Nepravilno zadati argumenti" << endl;
                return -1;
            }
            string regD;
            string regS;

            buff = new char[tmp1.find(',')];
            len = tmp1.copy(buff, (tmp1.find(',')), 0);
            buff[len] = '\0';
            regD = (buff);
            delete buff;
            Assembler::Triming(regD);

            buff = new char[tmp1.size() - tmp1.find(',') - 1];
            len = tmp1.copy(buff, (tmp1.size() - tmp1.find(',') - 1), (tmp1.find(',') + 1));
            buff[len] = '\0';
            regS = (buff);
            delete buff;
            Assembler::Triming(regS);

            if (Assembler::IsRegister(regD) == -1 || Assembler::IsRegister(regS) == -1)
            {
                cout << "Nepravilno uneti argumenti" << endl;
                return -1;
            }
            this->locCnt += 2;
            return 0;
        }

        if (tmp1 == "ldr")
        {
            char *buff = new char[tmp.size() - tmp.find(' ') - 1];
            size_t len = tmp.copy(buff, (tmp.size() - tmp.find(' ') - 1), (tmp.find(' ') + 1));
            buff[len] = '\0';
            tmp1 = (buff);
            delete buff;

            if (tmp1.find(',') == string::npos)
            {
                cout << "Nepravilno zadati argumenti" << endl;
                return -1;
            }
            string regD;
            string operand;

            buff = new char[tmp1.find(',')];
            len = tmp1.copy(buff, (tmp1.find(',')), 0);
            buff[len] = '\0';
            regD = (buff);
            delete buff;
            Assembler::Triming(regD);

            buff = new char[tmp1.size() - tmp1.find(',') - 1];
            len = tmp1.copy(buff, (tmp1.size() - tmp1.find(',') - 1), (tmp1.find(',') + 1));
            buff[len] = '\0';
            operand = (buff);
            delete buff;
            Assembler::Triming(operand);

            if (Assembler::IsRegister(regD) == -1)
            {
                cout << "Nepravilno uneti argumenti" << endl;
                return -1;
            }

            int n = Assembler::IsOperand(operand, 0);
            if (n < 0)
            {
                cout << "Nepravilno uneti argumenti" << endl;
                return -1;
            }
            this->locCnt += n;
            return 0;
        }

        if (tmp1 == "str")
        {
            char *buff = new char[tmp.size() - tmp.find(' ') - 1];
            size_t len = tmp.copy(buff, (tmp.size() - tmp.find(' ') - 1), (tmp.find(' ') + 1));
            buff[len] = '\0';
            tmp1 = (buff);
            delete buff;

            if (tmp1.find(',') == string::npos)
            {
                cout << "Nepravilno zadati argumenti" << endl;
                return -1;
            }
            string regD;
            string operand;

            buff = new char[tmp1.find(',')];
            len = tmp1.copy(buff, (tmp1.find(',')), 0);
            buff[len] = '\0';
            regD = (buff);
            delete buff;
            Assembler::Triming(regD);

            buff = new char[tmp1.size() - tmp1.find(',') - 1];
            len = tmp1.copy(buff, (tmp1.size() - tmp1.find(',') - 1), (tmp1.find(',') + 1));
            buff[len] = '\0';
            operand = (buff);
            delete buff;
            Assembler::Triming(operand);

            if (Assembler::IsRegister(regD) == -1)
            {
                cout << "Nepravilno uneti argumenti" << endl;
                return -1;
            }

            int n = Assembler::IsOperand(operand, 0);
            if (n < 0)
            {
                cout << "Nepravilno uneti argumenti" << endl;
                return -1;
            }
            this->locCnt += n;
            return 0;
        }

        if (tmp1 == "call")
        {
            char *buff = new char[tmp.size() - tmp.find(' ') - 1];
            size_t len = tmp.copy(buff, (tmp.size() - tmp.find(' ') - 1), (tmp.find(' ') + 1));
            buff[len] = '\0';
            string operand(buff);
            delete buff;
            Assembler::Triming(operand);

            int n = Assembler::IsOperand(operand, 1);
            if (n < 0)
            {
                cout << "Nepravilno uneti argumenti" << endl;
                return -1;
            }

            this->locCnt += n;
            return 0;
        }

        if (tmp1 == "jmp")
        {
            char *buff = new char[tmp.size() - tmp.find(' ') - 1];
            size_t len = tmp.copy(buff, (tmp.size() - tmp.find(' ') - 1), (tmp.find(' ') + 1));
            buff[len] = '\0';
            string operand(buff);
            delete buff;
            Assembler::Triming(operand);

            int n = Assembler::IsOperand(operand, 1);
            if (n < 0)
            {
                cout << "Nepravilno uneti argumenti" << endl;
                return -1;
            }

            this->locCnt += n;
            return 0;
        }

        if (tmp1 == "jeq")
        {
            char *buff = new char[tmp.size() - tmp.find(' ') - 1];
            size_t len = tmp.copy(buff, (tmp.size() - tmp.find(' ') - 1), (tmp.find(' ') + 1));
            buff[len] = '\0';
            string operand(buff);
            delete buff;
            Assembler::Triming(operand);

            int n = Assembler::IsOperand(operand, 1);
            if (n < 0)
            {
                cout << "Nepravilno uneti argumenti" << endl;
                return -1;
            }
            this->locCnt += n;
            return 0;
        }

        if (tmp1 == "jne")
        {
            char *buff = new char[tmp.size() - tmp.find(' ') - 1];
            size_t len = tmp.copy(buff, (tmp.size() - tmp.find(' ') - 1), (tmp.find(' ') + 1));
            buff[len] = '\0';
            string operand(buff);
            delete buff;
            Assembler::Triming(operand);

            int n = Assembler::IsOperand(operand, 1);
            if (n < 0)
            {
                cout << "Nepravilno uneti argumenti" << endl;
                return -1;
            }
            this->locCnt += n;
            return 0;
        }

        if (tmp1 == "jgt")
        {
            char *buff = new char[tmp.size() - tmp.find(' ') - 1];
            size_t len = tmp.copy(buff, (tmp.size() - tmp.find(' ') - 1), (tmp.find(' ') + 1));
            buff[len] = '\0';
            string operand(buff);
            delete buff;
            Assembler::Triming(operand);

            int n = Assembler::IsOperand(operand, 1);
            if (n < 0)
            {
                cout << "Nepravilno uneti argumenti" << endl;
                return -1;
            }
            this->locCnt += n;
            return 0;
        }

        cout << "Ne postojeca instrukcija" << endl;
        return -1;
    }
}

int Assembler::IsLabel(string &str)
{
    string tmp = str;

    char *buff = new char[tmp.find(':')];
    size_t len = tmp.copy(buff, (tmp.find(':')), 0);
    buff[len] = '\0';
    string labelName(buff);
    delete buff;
    Assembler::Triming(labelName);

    buff = new char[tmp.size() - tmp.find(':') - 1];
    len = tmp.copy(buff, (tmp.size() - tmp.find(':') - 1), (tmp.find(':') + 1));
    buff[len] = '\0';
    string inst(buff);
    delete buff;
    Assembler::Triming(inst);

    if (Assembler::IsSymbol(labelName) == -1)
    {
        cout << "Lose ste nazvali simbol" << endl;
        return -1;
    }

    if (this->InsertIntoSymTab(labelName, this->currSec, this->locCnt, true) == -1)
    {
        cout << "Simbol vec postoji u tabeli simbola" << endl;
        return -1;
    }

    if (inst == "")
    {
        return 0;
    }

    if (Assembler::IsInstruction(inst) == -1)
    {
        cout << "Lose ste nazvali instrukciju" << endl;
        return -1;
    }

    return 0;
}

int Assembler::ParseFileName(const string &str, char c) const
{
    if ((str.size() - str.find('.')) == 2 && str[str.find('.') + 1] == c)
    {
        return 1;
    }
    else
    {
        return -1;
    }
}

int Assembler::IsRegister(const string &str)
{
    string REGISTERS[] = {"r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7", "sp", "pc", "psw"};
    string tmp = str;
    Assembler::ToLower(tmp);
    for (string reg : REGISTERS)
    {
        if (reg == tmp)
        {
            return 0;
        }
    }
    return -1;
}

int Assembler::IsSymbol(const string &str)
{
    if (str == "")
    {
        return -1;
    }
    if (isalpha(str[0]) || str[0] == '_' || str[0] == '.')
    {
        for (int i = 1; i < str.length(); i++)
        {
            if (isalnum(str[i]) || str[i] == '_')
            {
                continue;
            }
            return -1;
        }

        if (Assembler::IsRegister(str) == 0)
        {
            return -1;
        }

        string DIRECTIVES[] = {".global", ".extern", ".section", ".word", ".skip", ".equ", ".end"};

        for (string dir : DIRECTIVES)
        {
            if (str == dir)
            {
                return -1;
            }
        }

        return 0;
    }
    return -1;
}

int Assembler::IsLiteral(const string &str)
{
    string tmp = str;
    Assembler::ToLower(tmp);

    if (tmp.find('x') != string::npos)
    {
        if (tmp.find('x') == 1 && tmp[0] == '0')
        {
            for (int i = 2; i < tmp.length(); i++)
            {
                if (isdigit(tmp[i]) || tmp[i] == 'a' || tmp[i] == 'b' || tmp[i] == 'c' ||
                    tmp[i] == 'd' || tmp[i] == 'e' || tmp[i] == 'f')
                {
                    continue;
                }
                return -1;
            }

            return 0;
        }
        else
        {
            return -1;
        }
    }
    else
    {
        for (int i = 0; i < tmp.length(); i++)
        {
            if (isdigit(tmp[i]))
            {
                continue;
            }
            return -1;
        }
        return 0;
    }
}

int Assembler::IsOperand(const string &str, int type)
{
    string tmp = str;
    if (type == 0)
    {
        if (tmp[0] == '$')
        {
            tmp.erase(0, 1);
            if (Assembler::IsLiteral(tmp) == 0)
            {
                return 5;
            }
            else if (Assembler::IsSymbol(tmp) == 0)
            {
                return 5;
            }
            else
            {
                return -1;
            }
        }

        if (tmp[0] == '%')
        {
            tmp.erase(0, 1);
            if (Assembler::IsSymbol(tmp) == 0)
            {
                return 5;
            }
            else
            {
                return -1;
            }
        }

        if (Assembler::IsLiteral(tmp) == 0)
        {
            return 5;
        }

        if (Assembler::IsSymbol(tmp) == 0)
        {
            return 5;
        }

        if (Assembler::IsRegister(tmp) == 0)
        {
            return 3;
        }

        if (tmp[0] == '[' && tmp[tmp.size() - 1] == ']')
        {
            tmp.erase(0, 1);
            tmp.erase(tmp.size() - 1, 1);

            Assembler::Triming(tmp);

            if (Assembler::IsRegister(tmp) == 0)
            {
                return 3;
            }
            else if (tmp.find('+') != string::npos)
            {
                char *buff = new char[tmp.find('+')];
                size_t len = tmp.copy(buff, (tmp.find('+')), 0);
                buff[len] = '\0';
                string reg(buff);
                Assembler::Triming(reg);

                if (Assembler::IsRegister(reg) == -1)
                {
                    return -1;
                }

                buff = new char[tmp.size() - tmp.find('+') - 1];
                len = tmp.copy(buff, (tmp.size() - tmp.find('+') - 1), (tmp.find('+') + 1));
                buff[len] = '\0';
                string arg(buff);
                Assembler::Triming(arg);

                if (Assembler::IsLiteral(arg) == 0)
                {
                    return 5;
                }

                if (Assembler::IsSymbol(arg) == 0)
                {

                    return 5;
                }

                return -1;
            }
            else
            {
                return -1;
            }
        }

        return -1;
    }
    else
    {
        if (tmp[0] == '%')
        {
            tmp.erase(0, 1);
            if (Assembler::IsSymbol(tmp) == 0)
            {
                return 5;
            }
            else
            {
                return -1;
            }
        }

        if (Assembler::IsLiteral(tmp) == 0)
        {
            return 5;
        }

        if (Assembler::IsSymbol(tmp) == 0)
        {
            return 5;
        }

        if (tmp[0] == '*')
        {
            tmp.erase(0, 1);

            if (Assembler::IsLiteral(tmp) == 0)
            {
                return 5;
            }

            if (Assembler::IsSymbol(tmp) == 0)
            {
                return 5;
            }

            if (Assembler::IsRegister(tmp) == 0)
            {
                return 3;
            }

            if (tmp[0] == '[' && tmp[tmp.size() - 1] == ']')
            {
                tmp.erase(0, 1);
                tmp.erase(tmp.size() - 1, 1);
                Assembler::Triming(tmp);

                if (Assembler::IsRegister(tmp) == 0)
                {
                    return 3;
                }
                else if (tmp.find('+') != string::npos)
                {
                    char *buff = new char[tmp.find('+')];
                    size_t len = tmp.copy(buff, (tmp.find('+')), 0);
                    buff[len] = '\0';
                    string reg(buff);
                    Assembler::Triming(reg);

                    if (Assembler::IsRegister(reg) == -1)
                    {
                        return -1;
                    }

                    buff = new char[tmp.size() - tmp.find('+') - 1];
                    len = tmp.copy(buff, (tmp.size() - tmp.find('+') - 1), (tmp.find('+') + 1));
                    buff[len] = '\0';
                    string arg(buff);
                    Assembler::Triming(arg);

                    if (Assembler::IsLiteral(arg) == 0)
                    {
                        return 5;
                    }

                    if (Assembler::IsSymbol(arg) == 0)
                    {
                        return 5;
                    }

                    return -1;
                }
                else
                {
                    return -1;
                }
            }

            return -1;
        }

        return -1;
    }
}

int Assembler::InsertIntoSymTab(const string &label, const string &section, int offset, bool local)
{
    for (SymTabElem *elem : this->symTab)
    {
        if (elem->label == label)
        {
            if (!local && !elem->local)
            {
                return 0;
            }

            if (elem->section == elem->label)
            {
                cout << "Simbol se zove kao i sekcija" << endl;
                return -1;
            }

            if (!elem->local && elem->section == "" && local)
            {
                elem->section = section;
                elem->offset = offset;
                return 0;
            }

            if (elem->local && !local)
            {
                elem->local = false;
                return 0;
            }

            cout << "Dublo nazvan simbol" << endl;
            return -1;
        }
    }
    symTab.push_back(new SymTabElem(label, section, offset, local));
    return 0;
}

int Assembler::HexStringToInt(const string &str)
{
    int num;
    stringstream ss;
    ss << hex << str;
    ss >> num;
    return num;
}

int Assembler::AsmLineSecondPass(string &str)
{
    Assembler::RemoveComents(str);
    Assembler::Triming(str);

    if (str == ".end")
    {
        return 1;
    }

    if (str.find(":") != string::npos)
    {
        return this->CodeLabel(str);
    }

    else if (str.find(".") == 0)
    {
        return this->CodeDirective(str);
    }

    else
    {
        return this->CodeInstruction(str);
    }

    return 0;
}

string Assembler::RegisterCode(const string &str)
{
    string tmp = str;
    Assembler::Triming(tmp);
    Assembler::ToLower(tmp);

    if (tmp == "psw")
    {
        return "8";
    }

    else if (tmp == "sp")
    {
        return "6";
    }

    else if (tmp == "pc")
    {
        return "7";
    }

    else
    {
        if (tmp == "r0")
        {
            return "0";
        }

        if (tmp == "r1")
        {
            return "1";
        }

        if (tmp == "r2")
        {
            return "2";
        }

        if (tmp == "r3")
        {
            return "3";
        }

        if (tmp == "r4")
        {
            return "4";
        }

        if (tmp == "r5")
        {
            return "5";
        }

        if (tmp == "r6")
        {
            return "6";
        }

        if (tmp == "r7")
        {
            return "7";
        }
    }

    return "";
}

int Assembler::CodeInstruction(string &str)
{
    string posCde("" + to_string(this->locCnt) + " :: ");
    string tmp = str;

    if (str.find(' ') == string::npos)
    {
        Assembler::ToLower(tmp);
        if (tmp == "halt")
        {
            this->locCnt++;
            this->secTab.push_back(new SecTabElem("00", this->currSec, posCde));
            return 0;
        }

        if (tmp == "iret")
        {
            this->locCnt++;
            this->secTab.push_back(new SecTabElem("20", this->currSec, posCde));
            return 0;
        }

        if (tmp == "ret")
        {
            this->locCnt++;
            this->secTab.push_back(new SecTabElem("40", this->currSec, posCde));
            return 0;
        }
    }

    else
    {
        char *buff = new char[tmp.find(' ')];
        size_t len = tmp.copy(buff, (tmp.find(' ')), 0);
        buff[len] = '\0';
        string tmp1(buff);
        delete buff;
        Assembler::ToLower(tmp1);

        if (tmp1 == "int")
        {
            char *buff = new char[tmp.size() - tmp.find(' ') - 1];
            size_t len = tmp.copy(buff, (tmp.size() - tmp.find(' ') - 1), (tmp.find(' ') + 1));
            buff[len] = '\0';
            string regD(buff);
            delete buff;
            Assembler::Triming(regD);
            string cdReg = Assembler::RegisterCode(regD);

            this->locCnt += 2;
            string code("10 " + cdReg + "f");
            this->secTab.push_back(new SecTabElem(code, this->currSec, posCde));
            return 0;
        }

        if (tmp1 == "push")
        {
            char *buff = new char[tmp.size() - tmp.find(' ') - 1];
            size_t len = tmp.copy(buff, (tmp.size() - tmp.find(' ') - 1), (tmp.find(' ') + 1));
            buff[len] = '\0';
            string regD(buff);
            delete buff;
            Assembler::Triming(regD);
            string cdReg = Assembler::RegisterCode(regD);

            string cdSp = Assembler::RegisterCode("sp");

            this->locCnt += 3;
            string code("a0 " + cdReg + "" + cdSp + " 12");
            this->secTab.push_back(new SecTabElem(code, this->currSec, posCde));

            return 0;
        }

        if (tmp1 == "pop")
        {
            char *buff = new char[tmp.size() - tmp.find(' ') - 1];
            size_t len = tmp.copy(buff, (tmp.size() - tmp.find(' ') - 1), (tmp.find(' ') + 1));
            buff[len] = '\0';
            string regD(buff);
            delete buff;
            Assembler::Triming(regD);
            string cdReg = Assembler::RegisterCode(regD);

            string cdSp = Assembler::RegisterCode("sp");

            this->locCnt += 3;
            string code("b0 " + cdReg + "" + cdSp + " 42");

            this->secTab.push_back(new SecTabElem(code, this->currSec, posCde));
            return 0;
        }

        if (tmp1 == "not")
        {
            char *buff = new char[tmp.size() - tmp.find(' ') - 1];
            size_t len = tmp.copy(buff, (tmp.size() - tmp.find(' ') - 1), (tmp.find(' ') + 1));
            buff[len] = '\0';
            string regD(buff);
            delete buff;
            Assembler::Triming(regD);
            string cdReg = Assembler::RegisterCode(regD);

            this->locCnt += 2;
            string code("80 " + cdReg + "" + cdReg);
            this->secTab.push_back(new SecTabElem(code, this->currSec, posCde));
            return 0;
        }

        if (tmp1 == "xchg")
        {
            char *buff = new char[tmp.size() - tmp.find(' ') - 1];
            size_t len = tmp.copy(buff, (tmp.size() - tmp.find(' ') - 1), (tmp.find(' ') + 1));
            buff[len] = '\0';
            tmp1 = (buff);
            delete buff;

            string regD;
            string regS;

            buff = new char[tmp1.find(',')];
            len = tmp1.copy(buff, (tmp1.find(',')), 0);
            buff[len] = '\0';
            regD = (buff);
            delete buff;
            Assembler::Triming(regD);

            buff = new char[tmp1.size() - tmp1.find(',') - 1];
            len = tmp1.copy(buff, (tmp1.size() - tmp1.find(',') - 1), (tmp1.find(',') + 1));
            buff[len] = '\0';
            regS = (buff);
            delete buff;
            Assembler::Triming(regS);

            this->locCnt += 2;

            string cdReg1 = Assembler::RegisterCode(regD);
            string cdReg2 = Assembler::RegisterCode(regS);

            string code("60 " + cdReg1 + "" + cdReg2);
            this->secTab.push_back(new SecTabElem(code, this->currSec, posCde));
            return 0;
        }

        if (tmp1 == "add")
        {
            char *buff = new char[tmp.size() - tmp.find(' ') - 1];
            size_t len = tmp.copy(buff, (tmp.size() - tmp.find(' ') - 1), (tmp.find(' ') + 1));
            buff[len] = '\0';
            tmp1 = (buff);
            delete buff;

            string regD;
            string regS;

            buff = new char[tmp1.find(',')];
            len = tmp1.copy(buff, (tmp1.find(',')), 0);
            buff[len] = '\0';
            regD = (buff);
            delete buff;
            Assembler::Triming(regD);

            buff = new char[tmp1.size() - tmp1.find(',') - 1];
            len = tmp1.copy(buff, (tmp1.size() - tmp1.find(',') - 1), (tmp1.find(',') + 1));
            buff[len] = '\0';
            regS = (buff);
            delete buff;
            Assembler::Triming(regS);

            this->locCnt += 2;

            string cdReg1 = Assembler::RegisterCode(regD);
            string cdReg2 = Assembler::RegisterCode(regS);

            string code("70 " + cdReg1 + "" + cdReg2);
            this->secTab.push_back(new SecTabElem(code, this->currSec, posCde));
            return 0;
        }

        if (tmp1 == "sub")
        {
            char *buff = new char[tmp.size() - tmp.find(' ') - 1];
            size_t len = tmp.copy(buff, (tmp.size() - tmp.find(' ') - 1), (tmp.find(' ') + 1));
            buff[len] = '\0';
            tmp1 = (buff);
            delete buff;

            string regD;
            string regS;

            buff = new char[tmp1.find(',')];
            len = tmp1.copy(buff, (tmp1.find(',')), 0);
            buff[len] = '\0';
            regD = (buff);
            delete buff;
            Assembler::Triming(regD);

            buff = new char[tmp1.size() - tmp1.find(',') - 1];
            len = tmp1.copy(buff, (tmp1.size() - tmp1.find(',') - 1), (tmp1.find(',') + 1));
            buff[len] = '\0';
            regS = (buff);
            delete buff;
            Assembler::Triming(regS);

            this->locCnt += 2;

            string cdReg1 = Assembler::RegisterCode(regD);
            string cdReg2 = Assembler::RegisterCode(regS);

            string code("71 " + cdReg1 + "" + cdReg2);
            this->secTab.push_back(new SecTabElem(code, this->currSec, posCde));
            return 0;
        }

        if (tmp1 == "mul")
        {
            char *buff = new char[tmp.size() - tmp.find(' ') - 1];
            size_t len = tmp.copy(buff, (tmp.size() - tmp.find(' ') - 1), (tmp.find(' ') + 1));
            buff[len] = '\0';
            tmp1 = (buff);
            delete buff;

            string regD;
            string regS;

            buff = new char[tmp1.find(',')];
            len = tmp1.copy(buff, (tmp1.find(',')), 0);
            buff[len] = '\0';
            regD = (buff);
            delete buff;
            Assembler::Triming(regD);

            buff = new char[tmp1.size() - tmp1.find(',') - 1];
            len = tmp1.copy(buff, (tmp1.size() - tmp1.find(',') - 1), (tmp1.find(',') + 1));
            buff[len] = '\0';
            regS = (buff);
            delete buff;
            Assembler::Triming(regS);

            this->locCnt += 2;

            string cdReg1 = Assembler::RegisterCode(regD);
            string cdReg2 = Assembler::RegisterCode(regS);

            string code("72 " + cdReg1 + "" + cdReg2);
            this->secTab.push_back(new SecTabElem(code, this->currSec, posCde));
            return 0;
        }

        if (tmp1 == "div")
        {
            char *buff = new char[tmp.size() - tmp.find(' ') - 1];
            size_t len = tmp.copy(buff, (tmp.size() - tmp.find(' ') - 1), (tmp.find(' ') + 1));
            buff[len] = '\0';
            tmp1 = (buff);
            delete buff;

            string regD;
            string regS;

            buff = new char[tmp1.find(',')];
            len = tmp1.copy(buff, (tmp1.find(',')), 0);
            buff[len] = '\0';
            regD = (buff);
            delete buff;
            Assembler::Triming(regD);

            buff = new char[tmp1.size() - tmp1.find(',') - 1];
            len = tmp1.copy(buff, (tmp1.size() - tmp1.find(',') - 1), (tmp1.find(',') + 1));
            buff[len] = '\0';
            regS = (buff);
            delete buff;
            Assembler::Triming(regS);

            this->locCnt += 2;

            string cdReg1 = Assembler::RegisterCode(regD);
            string cdReg2 = Assembler::RegisterCode(regS);

            string code("73 " + cdReg1 + "" + cdReg2);
            this->secTab.push_back(new SecTabElem(code, this->currSec, posCde));
            return 0;
        }

        if (tmp1 == "cmp")
        {
            char *buff = new char[tmp.size() - tmp.find(' ') - 1];
            size_t len = tmp.copy(buff, (tmp.size() - tmp.find(' ') - 1), (tmp.find(' ') + 1));
            buff[len] = '\0';
            tmp1 = (buff);
            delete buff;

            string regD;
            string regS;

            buff = new char[tmp1.find(',')];
            len = tmp1.copy(buff, (tmp1.find(',')), 0);
            buff[len] = '\0';
            regD = (buff);
            delete buff;
            Assembler::Triming(regD);

            buff = new char[tmp1.size() - tmp1.find(',') - 1];
            len = tmp1.copy(buff, (tmp1.size() - tmp1.find(',') - 1), (tmp1.find(',') + 1));
            buff[len] = '\0';
            regS = (buff);
            delete buff;
            Assembler::Triming(regS);

            this->locCnt += 2;

            string cdReg1 = Assembler::RegisterCode(regD);
            string cdReg2 = Assembler::RegisterCode(regS);

            string code("74 " + cdReg1 + "" + cdReg2);
            this->secTab.push_back(new SecTabElem(code, this->currSec, posCde));
            return 0;
        }

        if (tmp1 == "and")
        {
            char *buff = new char[tmp.size() - tmp.find(' ') - 1];
            size_t len = tmp.copy(buff, (tmp.size() - tmp.find(' ') - 1), (tmp.find(' ') + 1));
            buff[len] = '\0';
            tmp1 = (buff);
            delete buff;

            string regD;
            string regS;

            buff = new char[tmp1.find(',')];
            len = tmp1.copy(buff, (tmp1.find(',')), 0);
            buff[len] = '\0';
            regD = (buff);
            delete buff;
            Assembler::Triming(regD);

            buff = new char[tmp1.size() - tmp1.find(',') - 1];
            len = tmp1.copy(buff, (tmp1.size() - tmp1.find(',') - 1), (tmp1.find(',') + 1));
            buff[len] = '\0';
            regS = (buff);
            delete buff;
            Assembler::Triming(regS);

            this->locCnt += 2;

            string cdReg1 = Assembler::RegisterCode(regD);
            string cdReg2 = Assembler::RegisterCode(regS);

            string code("81 " + cdReg1 + "" + cdReg2);
            this->secTab.push_back(new SecTabElem(code, this->currSec, posCde));
            return 0;
        }

        if (tmp1 == "or")
        {
            char *buff = new char[tmp.size() - tmp.find(' ') - 1];
            size_t len = tmp.copy(buff, (tmp.size() - tmp.find(' ') - 1), (tmp.find(' ') + 1));
            buff[len] = '\0';
            tmp1 = (buff);
            delete buff;

            string regD;
            string regS;

            buff = new char[tmp1.find(',')];
            len = tmp1.copy(buff, (tmp1.find(',')), 0);
            buff[len] = '\0';
            regD = (buff);
            delete buff;
            Assembler::Triming(regD);

            buff = new char[tmp1.size() - tmp1.find(',') - 1];
            len = tmp1.copy(buff, (tmp1.size() - tmp1.find(',') - 1), (tmp1.find(',') + 1));
            buff[len] = '\0';
            regS = (buff);
            delete buff;
            Assembler::Triming(regS);

            this->locCnt += 2;

            string cdReg1 = Assembler::RegisterCode(regD);
            string cdReg2 = Assembler::RegisterCode(regS);

            string code("82 " + cdReg1 + "" + cdReg2);
            this->secTab.push_back(new SecTabElem(code, this->currSec, posCde));
            return 0;
        }

        if (tmp1 == "xor")
        {
            char *buff = new char[tmp.size() - tmp.find(' ') - 1];
            size_t len = tmp.copy(buff, (tmp.size() - tmp.find(' ') - 1), (tmp.find(' ') + 1));
            buff[len] = '\0';
            tmp1 = (buff);
            delete buff;

            string regD;
            string regS;

            buff = new char[tmp1.find(',')];
            len = tmp1.copy(buff, (tmp1.find(',')), 0);
            buff[len] = '\0';
            regD = (buff);
            delete buff;
            Assembler::Triming(regD);

            buff = new char[tmp1.size() - tmp1.find(',') - 1];
            len = tmp1.copy(buff, (tmp1.size() - tmp1.find(',') - 1), (tmp1.find(',') + 1));
            buff[len] = '\0';
            regS = (buff);
            delete buff;
            Assembler::Triming(regS);

            this->locCnt += 2;

            string cdReg1 = Assembler::RegisterCode(regD);
            string cdReg2 = Assembler::RegisterCode(regS);

            string code("83 " + cdReg1 + "" + cdReg2);
            this->secTab.push_back(new SecTabElem(code, this->currSec, posCde));
            return 0;
        }

        if (tmp1 == "test")
        {
            char *buff = new char[tmp.size() - tmp.find(' ') - 1];
            size_t len = tmp.copy(buff, (tmp.size() - tmp.find(' ') - 1), (tmp.find(' ') + 1));
            buff[len] = '\0';
            tmp1 = (buff);
            delete buff;

            string regD;
            string regS;

            buff = new char[tmp1.find(',')];
            len = tmp1.copy(buff, (tmp1.find(',')), 0);
            buff[len] = '\0';
            regD = (buff);
            delete buff;
            Assembler::Triming(regD);

            buff = new char[tmp1.size() - tmp1.find(',') - 1];
            len = tmp1.copy(buff, (tmp1.size() - tmp1.find(',') - 1), (tmp1.find(',') + 1));
            buff[len] = '\0';
            regS = (buff);
            delete buff;
            Assembler::Triming(regS);

            this->locCnt += 2;

            string cdReg1 = Assembler::RegisterCode(regD);
            string cdReg2 = Assembler::RegisterCode(regS);

            string code("84 " + cdReg1 + "" + cdReg2);
            this->secTab.push_back(new SecTabElem(code, this->currSec, posCde));
            return 0;
        }

        if (tmp1 == "shl")
        {
            char *buff = new char[tmp.size() - tmp.find(' ') - 1];
            size_t len = tmp.copy(buff, (tmp.size() - tmp.find(' ') - 1), (tmp.find(' ') + 1));
            buff[len] = '\0';
            tmp1 = (buff);
            delete buff;

            string regD;
            string regS;

            buff = new char[tmp1.find(',')];
            len = tmp1.copy(buff, (tmp1.find(',')), 0);
            buff[len] = '\0';
            regD = (buff);
            delete buff;
            Assembler::Triming(regD);

            buff = new char[tmp1.size() - tmp1.find(',') - 1];
            len = tmp1.copy(buff, (tmp1.size() - tmp1.find(',') - 1), (tmp1.find(',') + 1));
            buff[len] = '\0';
            regS = (buff);
            delete buff;
            Assembler::Triming(regS);

            this->locCnt += 2;

            string cdReg1 = Assembler::RegisterCode(regD);
            string cdReg2 = Assembler::RegisterCode(regS);

            string code("90 " + cdReg1 + "" + cdReg2);
            this->secTab.push_back(new SecTabElem(code, this->currSec, posCde));
            return 0;
        }

        if (tmp1 == "shr")
        {
            char *buff = new char[tmp.size() - tmp.find(' ') - 1];
            size_t len = tmp.copy(buff, (tmp.size() - tmp.find(' ') - 1), (tmp.find(' ') + 1));
            buff[len] = '\0';
            tmp1 = (buff);
            delete buff;

            string regD;
            string regS;

            buff = new char[tmp1.find(',')];
            len = tmp1.copy(buff, (tmp1.find(',')), 0);
            buff[len] = '\0';
            regD = (buff);
            delete buff;
            Assembler::Triming(regD);

            buff = new char[tmp1.size() - tmp1.find(',') - 1];
            len = tmp1.copy(buff, (tmp1.size() - tmp1.find(',') - 1), (tmp1.find(',') + 1));
            buff[len] = '\0';
            regS = (buff);
            delete buff;
            Assembler::Triming(regS);

            this->locCnt += 2;

            string cdReg1 = Assembler::RegisterCode(regD);
            string cdReg2 = Assembler::RegisterCode(regS);

            string code("91 " + cdReg1 + "" + cdReg2);
            this->secTab.push_back(new SecTabElem(code, this->currSec, posCde));
            return 0;
        }

        if (tmp1 == "ldr")
        {
            char *buff = new char[tmp.size() - tmp.find(' ') - 1];
            size_t len = tmp.copy(buff, (tmp.size() - tmp.find(' ') - 1), (tmp.find(' ') + 1));
            buff[len] = '\0';
            tmp1 = (buff);
            delete buff;

            string regD;
            string operand;

            buff = new char[tmp1.find(',')];
            len = tmp1.copy(buff, (tmp1.find(',')), 0);
            buff[len] = '\0';
            regD = (buff);
            delete buff;
            Assembler::Triming(regD);

            buff = new char[tmp1.size() - tmp1.find(',') - 1];
            len = tmp1.copy(buff, (tmp1.size() - tmp1.find(',') - 1), (tmp1.find(',') + 1));
            buff[len] = '\0';
            operand = (buff);
            delete buff;
            Assembler::Triming(operand);

            string regCde = Assembler::RegisterCode(regD);
            string oprCde = this->OperandCodeLDR_STR(operand);
            string code("a0 " + regCde + "" + oprCde);

            this->secTab.push_back(new SecTabElem(code, this->currSec, posCde));
            return 0;
        }

        if (tmp1 == "str")
        {
            char *buff = new char[tmp.size() - tmp.find(' ') - 1];
            size_t len = tmp.copy(buff, (tmp.size() - tmp.find(' ') - 1), (tmp.find(' ') + 1));
            buff[len] = '\0';
            tmp1 = (buff);
            delete buff;

            string regD;
            string operand;

            buff = new char[tmp1.find(',')];
            len = tmp1.copy(buff, (tmp1.find(',')), 0);
            buff[len] = '\0';
            regD = (buff);
            delete buff;
            Assembler::Triming(regD);

            buff = new char[tmp1.size() - tmp1.find(',') - 1];
            len = tmp1.copy(buff, (tmp1.size() - tmp1.find(',') - 1), (tmp1.find(',') + 1));
            buff[len] = '\0';
            operand = (buff);
            delete buff;
            Assembler::Triming(operand);

            string regCde = Assembler::RegisterCode(regD);
            string oprCde = this->OperandCodeLDR_STR(operand);
            string code("b0 " + regCde + "" + oprCde);
            this->secTab.push_back(new SecTabElem(code, this->currSec, posCde));
            return 0;
        }

        if (tmp1 == "call")
        {
            char *buff = new char[tmp.size() - tmp.find(' ') - 1];
            size_t len = tmp.copy(buff, (tmp.size() - tmp.find(' ') - 1), (tmp.find(' ') + 1));
            buff[len] = '\0';
            string operand(buff);
            delete buff;
            Assembler::Triming(operand);

            string oprCode = this->OperandCodeJMP(operand);
            string code("30 f" + oprCode);
            this->secTab.push_back(new SecTabElem(code, this->currSec, posCde));
            return 0;
        }

        if (tmp1 == "jmp")
        {
            char *buff = new char[tmp.size() - tmp.find(' ') - 1];
            size_t len = tmp.copy(buff, (tmp.size() - tmp.find(' ') - 1), (tmp.find(' ') + 1));
            buff[len] = '\0';
            string operand(buff);
            delete buff;
            Assembler::Triming(operand);

            string oprCode = this->OperandCodeJMP(operand);
            string code("50 f" + oprCode);

            this->secTab.push_back(new SecTabElem(code, this->currSec, posCde));
            return 0;
        }

        if (tmp1 == "jeq")
        {
            char *buff = new char[tmp.size() - tmp.find(' ') - 1];
            size_t len = tmp.copy(buff, (tmp.size() - tmp.find(' ') - 1), (tmp.find(' ') + 1));
            buff[len] = '\0';
            string operand(buff);
            delete buff;
            Assembler::Triming(operand);

            string oprCode = this->OperandCodeJMP(operand);
            string code("51 f" + oprCode);
            this->secTab.push_back(new SecTabElem(code, this->currSec, posCde));
            return 0;
        }

        if (tmp1 == "jne")
        {
            char *buff = new char[tmp.size() - tmp.find(' ') - 1];
            size_t len = tmp.copy(buff, (tmp.size() - tmp.find(' ') - 1), (tmp.find(' ') + 1));
            buff[len] = '\0';
            string operand(buff);
            delete buff;
            Assembler::Triming(operand);

            string oprCode = this->OperandCodeJMP(operand);
            string code("52 f" + oprCode);
            this->secTab.push_back(new SecTabElem(code, this->currSec, posCde));
            return 0;
        }

        if (tmp1 == "jgt")
        {
            char *buff = new char[tmp.size() - tmp.find(' ') - 1];
            size_t len = tmp.copy(buff, (tmp.size() - tmp.find(' ') - 1), (tmp.find(' ') + 1));
            buff[len] = '\0';
            string operand(buff);
            delete buff;
            Assembler::Triming(operand);

            string oprCode = this->OperandCodeJMP(operand);
            string code("53 f" + oprCode);
            this->secTab.push_back(new SecTabElem(code, this->currSec, posCde));
            return 0;
        }
    }
    return 0;
}

int Assembler::CodeLabel(string &str)
{

    string tmp = str;
    if (tmp.find(':') != tmp.size() - 1)
    {
        char *buff = new char[tmp.size() - tmp.find(':') - 1];
        size_t len = tmp.copy(buff, (tmp.size() - tmp.find(':') - 1), (tmp.find(':') + 1));
        buff[len] = '\0';
        string inst(buff);
        delete buff;
        Assembler::Triming(inst);
        return this->CodeInstruction(inst);
    }
    return 0;
}

int Assembler::CodeDirective(string &str)
{
    int pos = this->locCnt;
    string tmp = str;

    char *buff = new char[tmp.find(' ')];
    size_t len = tmp.copy(buff, (tmp.find(' ')), 0);
    buff[len] = '\0';
    string tmp1(buff);
    delete buff;
    Assembler::ToLower(tmp1);

    if (tmp1 == ".global")
    {
        return 0;
    }

    if (tmp1 == ".extern")
    {
        return 0;
    }

    if (tmp1 == ".word")
    {
        buff = new char[tmp.size() - tmp.find(' ') - 1];
        len = tmp.copy(buff, (tmp.size() - tmp.find(' ') - 1), (tmp.find(' ') + 1));
        buff[len] = '\0';
        string symLst(buff);
        delete buff;
        Assembler::Triming(symLst);
        do
        {
            Assembler::Triming(symLst);
            this->locCnt += 2;
            if (symLst.find(',') == string::npos)
            {

                if (Assembler::IsLiteral(symLst) == 0)
                {

                    if (symLst.find('x') != string::npos)
                    {
                        symLst.erase(0, 2);
                        while (symLst.size() > 4)
                        {
                            symLst.erase(0, 1);
                        }

                        while (symLst.size() < 4)
                        {
                            symLst = '0' + symLst;
                        }

                        string first("");
                        first = first + symLst[2] + symLst[3];

                        string second("");
                        second = second + symLst[0] + symLst[1];

                        symLst = first + " " + second;
                        string posCde("" + to_string(pos) + " :: ");
                        this->secTab.push_back(new SecTabElem(symLst, this->currSec, posCde));
                        pos += 2;
                    }
                    else
                    {
                        int num = stoi(symLst);
                        string numCde = Assembler::DecimalToHexString(num);

                        while (numCde.size() > 4)
                        {
                            numCde.erase(0, 1);
                        }

                        while (numCde.size() < 4)
                        {
                            numCde = '0' + numCde;
                        }

                        string first("");
                        first = first + numCde[2] + numCde[3];

                        string second("");
                        second = second + numCde[0] + numCde[1];
                        numCde = first + " " + second;

                        string posCde("" + to_string(pos) + " :: ");
                        this->secTab.push_back(new SecTabElem(numCde, this->currSec, posCde));
                        pos += 2;
                    }

                    return 0;
                }
                else
                {
                    this->InsertIntoSymTab2Pass(symLst);

                    if (this->GetSymbolLocal(symLst))
                    {
                        string sec = this->GetSymbolSection(symLst);
                        int id = this->GetSymbolID(sec);
                        string p = Assembler::DecimalToHexString(this->locCnt - 2);
                        this->relTab.push_back(new RelTabElem(p, "aps_16", id, this->currSec));

                        int num = this->GetSymbolOffset(symLst);
                        string numCde = Assembler::DecimalToHexString(num);

                        while (numCde.size() > 4)
                        {
                            numCde.erase(0, 1);
                        }

                        while (numCde.size() < 4)
                        {
                            numCde = '0' + numCde;
                        }

                        string first("");
                        first = first + numCde[2] + numCde[3];

                        string second("");
                        second = second + numCde[0] + numCde[1];
                        numCde = first + " " + second;

                        string posCde("" + to_string(pos) + " :: ");
                        this->secTab.push_back(new SecTabElem(numCde, this->currSec, posCde));
                        pos += 2;
                    }
                    else
                    {
                        int id = this->GetSymbolID(symLst);
                        string p = Assembler::DecimalToHexString(this->locCnt - 2);
                        this->relTab.push_back(new RelTabElem(p, "aps_16", id, this->currSec));
                        string code = "00 00";
                        string posCde("" + to_string(pos) + " :: ");
                        this->secTab.push_back(new SecTabElem(code, this->currSec, posCde));
                        pos += 2;
                    }
                    return 0;
                }
            }
            else
            {
                buff = new char[symLst.find(',')];
                len = symLst.copy(buff, symLst.find(','), 0);
                buff[len] = '\0';
                string label(buff);
                delete buff;
                Assembler::Triming(label);

                symLst.erase(0, symLst.find(',') + 1);

                if (Assembler::IsLiteral(label) == 0)
                {
                    if (label.find('x') != string::npos)
                    {
                        label.erase(0, 2);
                        while (label.size() > 4)
                        {
                            label.erase(0, 1);
                        }

                        while (label.size() < 4)
                        {
                            label = '0' + label;
                        }

                        string first("");
                        first = first + label[2] + label[3];

                        string second("");
                        second = second + label[0] + label[1];

                        label = first + " " + second;
                        string posCde("" + to_string(pos) + " :: ");
                        this->secTab.push_back(new SecTabElem(label, this->currSec, posCde));
                        pos += 2;
                    }
                    else
                    {
                        int num = stoi(label);
                        string numCde = Assembler::DecimalToHexString(num);

                        while (numCde.size() > 4)
                        {
                            numCde.erase(0, 1);
                        }

                        while (numCde.size() < 4)
                        {
                            numCde = '0' + numCde;
                        }

                        string first("");
                        first = first + numCde[2] + numCde[3];

                        string second("");
                        second = second + numCde[0] + numCde[1];
                        numCde = first + " " + second;

                        string posCde("" + to_string(pos) + " :: ");
                        this->secTab.push_back(new SecTabElem(numCde, this->currSec, posCde));
                        pos += 2;
                    }
                }
                else
                {
                    this->InsertIntoSymTab2Pass(label);
                    if (this->GetSymbolLocal(label))
                    {
                        string sec = this->GetSymbolSection(label);
                        int id = this->GetSymbolID(sec);
                        string p = Assembler::DecimalToHexString(this->locCnt - 2);
                        this->relTab.push_back(new RelTabElem(p, "aps_16", id, this->currSec));

                        int num = this->GetSymbolOffset(label);
                        string numCde = Assembler::DecimalToHexString(num);

                        while (numCde.size() > 4)
                        {
                            numCde.erase(0, 1);
                        }

                        while (numCde.size() < 4)
                        {
                            numCde = '0' + numCde;
                        }

                        string first("");
                        first = first + numCde[2] + numCde[3];

                        string second("");
                        second = second + numCde[0] + numCde[1];
                        numCde = first + " " + second;

                        string posCde("" + to_string(pos) + " :: ");
                        this->secTab.push_back(new SecTabElem(numCde, this->currSec, posCde));
                        pos += 2;
                    }
                    else
                    {
                        int id = this->GetSymbolID(label);
                        string p = Assembler::DecimalToHexString(this->locCnt - 2);
                        this->relTab.push_back(new RelTabElem(p, "aps_16", id, this->currSec));
                        string code = "00 00";
                        string posCde("" + to_string(pos) + " :: ");
                        this->secTab.push_back(new SecTabElem(code, this->currSec, posCde));
                        pos += 2;
                    }
                }
            }
        } while (symLst != "");

        return 0;
    }

    if (tmp1 == ".skip")
    {
        buff = new char[tmp.size() - tmp.find(' ') - 1];
        len = tmp.copy(buff, (tmp.size() - tmp.find(' ') - 1), (tmp.find(' ') + 1));
        buff[len] = '\0';
        string literal(buff);
        delete buff;
        Assembler::Triming(literal);
        int num;
        if (literal.find('x') == string::npos)
        {
            num = stoi(literal);
        }
        else
        {
            num = Assembler::HexStringToInt(literal);
        }
        this->locCnt += num;
        for (int i = 0; i < num; i++)
        {
            string posCde("" + to_string(pos) + " :: ");
            this->secTab.push_back(new SecTabElem("00", this->currSec, posCde));
            pos++;
        }
        return 0;
    }

    if (tmp1 == ".section")
    {
        buff = new char[tmp.size() - tmp.find(' ') - 1];
        len = tmp.copy(buff, (tmp.size() - tmp.find(' ') - 1), (tmp.find(' ') + 1));
        buff[len] = '\0';
        string sectionName(buff);
        delete buff;

        this->locCnt = 0;
        this->currSec = sectionName;

        return 0;
    }

    if (tmp1 == ".equ")
    {
        return 0;
    }
    return 0;
}

string Assembler::OperandCodeLDR_STR(const string &str)
{
    string tmp = str;

    if (tmp[0] == '$')
    {
        tmp.erase(0, 1);
        if (Assembler::IsLiteral(tmp) == 0)
        {
            this->locCnt += 5;
            if (tmp.find('x') != string::npos)
            {
                tmp.erase(0, 2);
                while (tmp.size() > 4)
                {
                    tmp.erase(0, 1);
                }

                while (tmp.size() < 4)
                {
                    tmp = '0' + tmp;
                }
                return ("f 00 " + tmp);
            }
            else
            {
                int num = stoi(tmp);
                string numCde = Assembler::DecimalToHexString(num);

                while (numCde.size() > 4)
                {
                    numCde.erase(0, 1);
                }

                while (numCde.size() < 4)
                {
                    numCde = '0' + numCde;
                }
                return ("f 00 " + numCde);
            }
        }

        else
        {
            this->locCnt += 5;

            string absSec = this->GetSymbolSection(tmp);
            if (absSec == "*ABS*")
            {
                int offset = this->GetSymbolOffset(tmp);
                string offCde = DecimalToHexString(offset);

                while (offCde.size() > 4)
                {
                    offCde.erase(0, 1);
                }
                while (offCde.size() < 4)
                {
                    offCde = '0' + offCde;
                }

                return ("f 00 " + offCde);
            }

            this->InsertIntoSymTab2Pass(tmp);
            if (this->GetSymbolLocal(tmp) == 1)
            {
                string section = this->GetSymbolSection(tmp);
                int offset = this->GetSymbolOffset(tmp);
                int id = this->GetSymbolID(section);

                string p = Assembler::DecimalToHexString(this->locCnt - 2);

                this->relTab.push_back(new RelTabElem(p, "aps_16", id, this->currSec));
                string offCde = Assembler::DecimalToHexString(offset);
                while (offCde.size() > 4)
                {
                    offCde.erase(0, 1);
                }
                while (offCde.size() < 4)
                {
                    offCde = '0' + offCde;
                }
                return ("f 00 " + offCde);
            }
            else
            {
                string p = Assembler::DecimalToHexString(this->locCnt - 2);

                int id = this->GetSymbolID(tmp);
                this->relTab.push_back(new RelTabElem(p, "aps_16", id, this->currSec));
                return ("f 00 fffe");
            }
        }
    }

    if (tmp[0] == '%')
    {
        tmp.erase(0, 1);

        this->locCnt += 5;

        string checkSec = this->GetSymbolSection(tmp);

        if (checkSec == "*ABS*")
        {
            int offset = this->GetSymbolOffset(tmp);

            string p = Assembler::DecimalToHexString(this->locCnt - 2);

            this->relTab.push_back(new RelTabElem(p, "rel_16", -1, this->currSec));
            string offCde = Assembler::DecimalToHexString(offset - 2);

            while (offCde.size() > 4)
            {
                offCde.erase(0, 1);
            }

            while (offCde.size() < 4)
            {
                offCde = '0' + offCde;
            }

            string pcCde = Assembler::RegisterCode("pc");
            return (pcCde + " 03 " + offCde);
        }

        if (checkSec == this->currSec)
        {
            int offset = this->GetSymbolOffset(tmp);
            offset -= this->locCnt;
            string offCde = Assembler::DecimalToHexString(offset);

            while (offCde.size() > 4)
            {
                offCde.erase(0, 1);
            }

            while (offCde.size() < 4)
            {
                offCde = '0' + offCde;
            }

            string pcCde = Assembler::RegisterCode("pc");
            return (pcCde + " 03 " + offCde);
        }

        this->InsertIntoSymTab2Pass(tmp);
        if (this->GetSymbolLocal(tmp) == 1)
        {
            string section = this->GetSymbolSection(tmp);
            int offset = this->GetSymbolOffset(tmp);
            int id = this->GetSymbolID(section);

            string p = Assembler::DecimalToHexString(this->locCnt - 2);

            this->relTab.push_back(new RelTabElem(p, "rel_16", id, this->currSec));
            string offCde = Assembler::DecimalToHexString(offset);

            while (offCde.size() > 4)
            {
                offCde.erase(0, 1);
            }

            while (offCde.size() < 4)
            {
                offCde = '0' + offCde;
            }

            string pcCde = Assembler::RegisterCode("pc");
            return (pcCde + " 03 " + offCde);
        }
        else
        {
            int id = this->GetSymbolID(tmp);

            string p = Assembler::DecimalToHexString(this->locCnt - 2);

            this->relTab.push_back(new RelTabElem(p, "rel_16", id, this->currSec));
            string pcCde = Assembler::RegisterCode("pc");
            return (pcCde + " 03 fffe");
        }
    }

    if (Assembler::IsLiteral(tmp) == 0)
    {
        this->locCnt += 5;
        if (tmp.find('x') != string::npos)
        {
            tmp.erase(0, 2);
            while (tmp.size() > 4)
            {
                tmp.erase(0, 1);
            }

            while (tmp.size() < 4)
            {
                tmp = '0' + tmp;
            }
            return ("f 04 " + tmp);
        }
        else
        {
            int num = stoi(tmp);
            string numCde = Assembler::DecimalToHexString(num);

            while (numCde.size() > 4)
            {
                numCde.erase(0, 1);
            }

            while (numCde.size() < 4)
            {
                numCde = '0' + numCde;
            }

            return ("f 04 " + numCde);
        }
    }

    if (Assembler::IsSymbol(tmp) == 0)
    {

        this->locCnt += 5;

        string absSec = this->GetSymbolSection(tmp);
        if (absSec == "*ABS*")
        {
            int offset = this->GetSymbolOffset(tmp);
            string offCde = DecimalToHexString(offset);

            while (offCde.size() > 4)
            {
                offCde.erase(0, 1);
            }
            while (offCde.size() < 4)
            {
                offCde = '0' + offCde;
            }

            return ("f 04 " + offCde);
        }

        this->InsertIntoSymTab2Pass(tmp);
        if (this->GetSymbolLocal(tmp) == 1)
        {
            string section = this->GetSymbolSection(tmp);
            int offset = this->GetSymbolOffset(tmp);
            int id = this->GetSymbolID(section);

            string p = Assembler::DecimalToHexString(this->locCnt - 2);

            this->relTab.push_back(new RelTabElem(p, "aps_16", id, this->currSec));
            string offCde = Assembler::DecimalToHexString(offset);

            while (offCde.size() > 4)
            {
                offCde.erase(0, 1);
            }

            while (offCde.size() < 4)
            {
                offCde = '0' + offCde;
            }
            return ("f 04 " + offCde);
        }
        else
        {
            int id = this->GetSymbolID(tmp);
            string p = Assembler::DecimalToHexString(this->locCnt - 2);
            this->relTab.push_back(new RelTabElem(p, "aps_16", id, this->currSec));
            return ("f 04 fffe");
        }
    }

    if (Assembler::IsRegister(tmp) == 0)
    {
        this->locCnt += 3;
        string regCde = Assembler::RegisterCode(tmp);
        return (regCde + " 01");
    }

    if (tmp[0] == '[' && tmp[tmp.size() - 1] == ']')
    {
        tmp.erase(0, 1);
        tmp.erase(tmp.size() - 1, 1);

        Assembler::Triming(tmp);

        if (Assembler::IsRegister(tmp) == 0)
        {
            this->locCnt += 3;
            string regCde = Assembler::RegisterCode(tmp);
            return (regCde + " 02");
        }

        else
        {
            char *buff = new char[tmp.find('+')];
            size_t len = tmp.copy(buff, (tmp.find('+')), 0);
            buff[len] = '\0';
            string reg(buff);
            Assembler::Triming(reg);

            buff = new char[tmp.size() - tmp.find('+') - 1];
            len = tmp.copy(buff, (tmp.size() - tmp.find('+') - 1), (tmp.find('+') + 1));
            buff[len] = '\0';
            string arg(buff);
            Assembler::Triming(arg);

            if (Assembler::IsLiteral(arg) == 0)
            {
                this->locCnt += 5;
                string regCde = Assembler::RegisterCode(reg);
                if (arg.find('x') != string::npos)
                {
                    arg.erase(0, 2);
                    while (arg.size() > 4)
                    {
                        arg.erase(0, 1);
                    }

                    while (arg.size() < 4)
                    {
                        arg = '0' + arg;
                    }
                    return (regCde + " 03 " + arg);
                }
                else
                {
                    int num = stoi(arg);
                    string numCde = Assembler::DecimalToHexString(num);

                    while (numCde.size() > 4)
                    {
                        numCde.erase(0, 1);
                    }

                    while (numCde.size() < 4)
                    {
                        numCde = '0' + numCde;
                    }

                    return (regCde + " 03 " + numCde);
                }
            }

            else
            {
                this->locCnt += 5;

                string absSec = this->GetSymbolSection(arg);
                if (absSec == "*ABS*")
                {
                    int offset = this->GetSymbolOffset(arg);
                    string offCde = DecimalToHexString(offset);

                    while (offCde.size() > 4)
                    {
                        offCde.erase(0, 1);
                    }
                    while (offCde.size() < 4)
                    {
                        offCde = '0' + offCde;
                    }
                    string regCde = Assembler::RegisterCode(reg);
                    return (regCde + " 03 " + offCde);
                }

                this->InsertIntoSymTab2Pass(arg);
                if (this->GetSymbolLocal(arg) == 1)
                {
                    string section = this->GetSymbolSection(arg);
                    int offset = this->GetSymbolOffset(arg);
                    int id = this->GetSymbolID(section);
                    string p = Assembler::DecimalToHexString(this->locCnt - 2);
                    this->relTab.push_back(new RelTabElem(p, "aps_16", id, this->currSec));
                    string offCde = Assembler::DecimalToHexString(offset);
                    string regCde = Assembler::RegisterCode(reg);

                    while (offCde.size() > 4)
                    {
                        offCde.erase(0, 1);
                    }

                    while (offCde.size() < 4)
                    {
                        offCde = '0' + offCde;
                    }
                    return (regCde + " 03 " + offCde);
                }
                else
                {
                    int id = this->GetSymbolID(arg);
                    string p = Assembler::DecimalToHexString(this->locCnt - 2);
                    this->relTab.push_back(new RelTabElem(p, "aps_16", id, this->currSec));
                    string regCde = Assembler::RegisterCode(reg);
                    return (regCde + " 03 fffe");
                }
            }
        }
    }

    return "";
}

string Assembler::OperandCodeJMP(const string &str)
{
    string tmp = str;

    if (tmp[0] == '%')
    {
        tmp.erase(0, 1);

        this->locCnt += 5;
        string checkSec = this->GetSymbolSection(tmp);

        if (checkSec == "*ABS*")
        {
            int offset = this->GetSymbolOffset(tmp);

            string p = Assembler::DecimalToHexString(this->locCnt - 2);

            this->relTab.push_back(new RelTabElem(p, "rel_16", -1, this->currSec));
            string offCde = Assembler::DecimalToHexString(offset - 2);

            while (offCde.size() > 4)
            {
                offCde.erase(0, 1);
            }

            while (offCde.size() < 4)
            {
                offCde = '0' + offCde;
            }

            string pcCde = Assembler::RegisterCode("pc");
            return (pcCde + " 05 " + offCde);
        }

        if (checkSec == this->currSec)
        {
            int offset = this->GetSymbolOffset(tmp);
            offset -= this->locCnt;
            string offCde = Assembler::DecimalToHexString(offset);

            while (offCde.size() > 4)
            {
                offCde.erase(0, 1);
            }

            while (offCde.size() < 4)
            {
                offCde = '0' + offCde;
            }

            string pcCde = Assembler::RegisterCode("pc");
            return (pcCde + " 05 " + offCde);
        }

        this->InsertIntoSymTab2Pass(tmp);
        if (this->GetSymbolLocal(tmp) == 1)
        {
            string section = this->GetSymbolSection(tmp);
            int offset = this->GetSymbolOffset(tmp);
            int id = this->GetSymbolID(section);
            string p = Assembler::DecimalToHexString(this->locCnt - 2);
            this->relTab.push_back(new RelTabElem(p, "rel_16", id, this->currSec));
            string offCde = Assembler::DecimalToHexString(offset);

            while (offCde.size() > 4)
            {
                offCde.erase(0, 1);
            }

            while (offCde.size() < 4)
            {
                offCde = '0' + offCde;
            }
            string pcCde = Assembler::RegisterCode("pc");
            return (pcCde + " 05 " + offCde);
        }
        else
        {
            int id = this->GetSymbolID(tmp);
            string p = Assembler::DecimalToHexString(this->locCnt - 2);
            this->relTab.push_back(new RelTabElem(p, "rel_16", id, this->currSec));
            string pcCde = Assembler::RegisterCode("pc");
            return (pcCde + " 05 fffe");
        }
    }

    if (Assembler::IsLiteral(tmp) == 0)
    {
        this->locCnt += 5;
        if (tmp.find('x') != string::npos)
        {
            tmp.erase(0, 2);
            while (tmp.size() > 4)
            {
                tmp.erase(0, 1);
            }

            while (tmp.size() < 4)
            {
                tmp = '0' + tmp;
            }
            return ("f 00 " + tmp);
        }
        else
        {
            int num = stoi(tmp);
            string numCde = Assembler::DecimalToHexString(num);

            while (numCde.size() > 4)
            {
                numCde.erase(0, 1);
            }

            while (numCde.size() < 4)
            {
                numCde = '0' + numCde;
            }

            return ("f 00 " + numCde);
        }
    }

    if (Assembler::IsSymbol(tmp) == 0)
    {
        this->locCnt += 5;

        string absSec = this->GetSymbolSection(tmp);
        if (absSec == "*ABS*")
        {
            int offset = this->GetSymbolOffset(tmp);
            string offCde = DecimalToHexString(offset);

            while (offCde.size() > 4)
            {
                offCde.erase(0, 1);
            }
            while (offCde.size() < 4)
            {
                offCde = '0' + offCde;
            }

            return ("f 00 " + offCde);
        }

        this->InsertIntoSymTab2Pass(tmp);
        if (this->GetSymbolLocal(tmp) == 1)
        {
            string section = this->GetSymbolSection(tmp);
            int offset = this->GetSymbolOffset(tmp);
            int id = this->GetSymbolID(section);
            string p = Assembler::DecimalToHexString(this->locCnt - 2);
            this->relTab.push_back(new RelTabElem(p, "aps_16", id, this->currSec));
            string offCde = Assembler::DecimalToHexString(offset);

            while (offCde.size() > 4)
            {
                offCde.erase(0, 1);
            }

            while (offCde.size() < 4)
            {
                offCde = '0' + offCde;
            }
            return ("f 00 " + offCde);
        }
        else
        {
            int id = this->GetSymbolID(tmp);
            string p = Assembler::DecimalToHexString(this->locCnt - 2);
            this->relTab.push_back(new RelTabElem(p, "aps_16", id, this->currSec));
            return ("f 00 fffe");
        }
    }

    if (tmp[0] == '*')
    {
        tmp.erase(0, 1);

        if (Assembler::IsLiteral(tmp) == 0)
        {
            this->locCnt += 5;
            if (tmp.find('x') != string::npos)
            {
                tmp.erase(0, 2);
                while (tmp.size() > 4)
                {
                    tmp.erase(0, 1);
                }

                while (tmp.size() < 4)
                {
                    tmp = '0' + tmp;
                }
                return ("f 04 " + tmp);
            }
            else
            {
                int num = stoi(tmp);
                string numCde = Assembler::DecimalToHexString(num);

                while (numCde.size() > 4)
                {
                    numCde.erase(0, 1);
                }

                while (numCde.size() < 4)
                {
                    numCde = '0' + numCde;
                }

                return ("f 04 " + numCde);
            }
        }

        if (Assembler::IsSymbol(tmp) == 0)
        {
            this->locCnt += 5;

            string absSec = this->GetSymbolSection(tmp);
            if (absSec == "*ABS*")
            {
                int offset = this->GetSymbolOffset(tmp);
                string offCde = DecimalToHexString(offset);

                while (offCde.size() > 4)
                {
                    offCde.erase(0, 1);
                }
                while (offCde.size() < 4)
                {
                    offCde = '0' + offCde;
                }

                return ("f 04 " + offCde);
            }

            this->InsertIntoSymTab2Pass(tmp);
            if (this->GetSymbolLocal(tmp) == 1)
            {
                string section = this->GetSymbolSection(tmp);
                int offset = this->GetSymbolOffset(tmp);
                int id = this->GetSymbolID(section);
                string p = Assembler::DecimalToHexString(this->locCnt - 2);
                this->relTab.push_back(new RelTabElem(p, "aps_16", id, this->currSec));
                string offCde = Assembler::DecimalToHexString(offset);

                while (offCde.size() > 4)
                {
                    offCde.erase(0, 1);
                }

                while (offCde.size() < 4)
                {
                    offCde = '0' + offCde;
                }
                return ("f 04 " + offCde);
            }
            else
            {
                int id = this->GetSymbolID(tmp);
                string p = Assembler::DecimalToHexString(this->locCnt - 2);
                this->relTab.push_back(new RelTabElem(p, "aps_16", id, this->currSec));
                return ("f 04 fffe");
            }
        }

        if (Assembler::IsRegister(tmp) == 0)
        {
            this->locCnt += 3;
            string regCde = Assembler::RegisterCode(tmp);
            return (regCde + " 01");
        }

        if (tmp[0] == '[' && tmp[tmp.size() - 1] == ']')
        {
            tmp.erase(0, 1);
            tmp.erase(tmp.size() - 1, 1);
            Assembler::Triming(tmp);

            if (Assembler::IsRegister(tmp) == 0)
            {
                this->locCnt += 3;
                string regCde = Assembler::RegisterCode(tmp);
                return (regCde + " 02");
            }

            else
            {
                char *buff = new char[tmp.find('+')];
                size_t len = tmp.copy(buff, (tmp.find('+')), 0);
                buff[len] = '\0';
                string reg(buff);
                Assembler::Triming(reg);

                buff = new char[tmp.size() - tmp.find('+') - 1];
                len = tmp.copy(buff, (tmp.size() - tmp.find('+') - 1), (tmp.find('+') + 1));
                buff[len] = '\0';
                string arg(buff);
                Assembler::Triming(arg);

                if (Assembler::IsLiteral(arg) == 0)
                {
                    this->locCnt += 5;
                    string regCde = Assembler::RegisterCode(reg);
                    if (arg.find('x') != string::npos)
                    {
                        arg.erase(0, 2);
                        while (arg.size() > 4)
                        {
                            arg.erase(0, 1);
                        }

                        while (arg.size() < 4)
                        {
                            arg = '0' + arg;
                        }
                        return (regCde + " 03 " + arg);
                    }
                    else
                    {
                        int num = stoi(arg);
                        string numCde = Assembler::DecimalToHexString(num);

                        while (numCde.size() > 4)
                        {
                            numCde.erase(0, 1);
                        }

                        while (numCde.size() < 4)
                        {
                            numCde = '0' + numCde;
                        }

                        return (regCde + " 03 " + numCde);
                    }
                }

                if (Assembler::IsSymbol(arg) == 0)
                {
                    this->locCnt += 5;

                    string absSec = this->GetSymbolSection(arg);
                    if (absSec == "*ABS*")
                    {
                        int offset = this->GetSymbolOffset(arg);
                        string offCde = DecimalToHexString(offset);

                        while (offCde.size() > 4)
                        {
                            offCde.erase(0, 1);
                        }
                        while (offCde.size() < 4)
                        {
                            offCde = '0' + offCde;
                        }

                        string regCde = Assembler::RegisterCode(reg);
                        return (regCde + " 03 " + offCde);
                    }

                    this->InsertIntoSymTab2Pass(arg);
                    if (this->GetSymbolLocal(arg) == 1)
                    {
                        string section = this->GetSymbolSection(arg);
                        int offset = this->GetSymbolOffset(arg);
                        int id = this->GetSymbolID(section);
                        string p = Assembler::DecimalToHexString(this->locCnt - 2);
                        this->relTab.push_back(new RelTabElem(p, "aps_16", id, this->currSec));
                        string offCde = Assembler::DecimalToHexString(offset);

                        while (offCde.size() > 4)
                        {
                            offCde.erase(0, 1);
                        }

                        while (offCde.size() < 4)
                        {
                            offCde = '0' + offCde;
                        }
                        string regCde = Assembler::RegisterCode(reg);
                        return (regCde + " 03 " + offCde);
                    }

                    else
                    {
                        int id = this->GetSymbolID(arg);
                        string p = Assembler::DecimalToHexString(this->locCnt - 2);
                        this->relTab.push_back(new RelTabElem(p, "aps_16", id, this->currSec));
                        string regCde = Assembler::RegisterCode(reg);
                        return (regCde + " 03 fffe");
                    }
                }
            }
        }
    }

    return "";
}

string Assembler::DecimalToHexString(int num)
{
    stringstream ss;
    ss << hex << num;
    string str = ss.str();
    return str;
}

void Assembler::SetSectionSize()
{
    if (this->currSec == "")
    {
        return;
    }
    for (SymTabElem *elem : this->symTab)
    {
        if (elem->section == this->currSec)
        {
            elem->secSize = this->locCnt;

            return;
        }
    }
}

void Assembler::InsertIntoSymTab2Pass(const string &label)
{
    for (SymTabElem *elem : this->symTab)
    {
        if (elem->label == label)
        {
            return;
        }
    }

    this->symTab.push_back(new SymTabElem(label, "", 0, false));
}

int Assembler::GetSymbolLocal(const string &label) const
{
    for (SymTabElem *elem : this->symTab)
    {
        if (elem->label == label)
        {
            return elem->local;
        }
    }

    return -1;
}

int Assembler::GetSymbolID(const string &label) const
{
    for (int i = 0; i < this->symTab.size(); i++)
    {
        if (this->symTab[i]->label == label)
        {
            return i;
        }
    }
    return 0;
}

int Assembler::GetSymbolOffset(const string &label) const
{
    for (SymTabElem *elem : this->symTab)
    {
        if (elem->label == label)
        {
            return elem->offset;
        }
    }
    return -1;
}

string Assembler::GetSymbolSection(const string &label) const
{
    for (SymTabElem *elem : this->symTab)
    {
        if (elem->label == label)
        {
            return elem->section;
        }
    }
    return "";
}

void Assembler::PrintInOutputFile(char *fileName)
{
    ofstream output(fileName, ofstream::out | ofstream::trunc);
    if (output.is_open())
    {
        output << ":::TABELA SIMBOLA:::" << endl
               << endl;
        int i = 0;
        char loc = '-';
        for (SymTabElem *elem : this->symTab)
        {
            if (i > 0)
            {
                if (elem->local)
                {
                    loc = 'l';
                }
                else
                {
                    loc = 'g';
                }
                string sec(elem->section);
                if (sec == "")
                {
                    sec = "*UND*";
                }
                if (elem->label == elem->section)
                {
                    string off = DecimalToHexString(elem->offset);
                    string secS = DecimalToHexString(elem->secSize);
                    output << i << "   " << elem->label << "   " << sec << "   " << loc << "   " << off << "  " << secS << endl;
                }
                else
                {
                    string off = DecimalToHexString(elem->offset);
                    output << i << "   " << elem->label << "   " << sec << "   " << loc << "   " << off << endl;
                }
            }
            i++;
        }

        output << endl
               << endl;

        output << ":::TABELA REALOKACIJE:::" << endl
               << endl;

        for (RelTabElem *elem : this->relTab)
        {
            string sec(elem->section);
            if (sec == "")
            {
                sec = "*UND*";
            }

            string val = to_string(elem->val);

            if (val == "-1")
            {
                val = "*ABS*";
            }

            output << elem->offset << "   " << elem->type << "  " << val << "   " << sec << endl;
        }

        output << endl
               << endl;

        output << ":::IZLAZ:::" << endl
               << endl;

        for (SecTabElem *elem : this->secTab)
        {
            output << elem->position << "  " << elem->instCode << "  " << elem->section << endl;
        }
    }
    else
    {
        cout << "Neuspesno otvaranje izlaznog fajla" << endl;
    }
    output.close();
}
