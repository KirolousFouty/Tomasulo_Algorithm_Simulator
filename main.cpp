#include <iostream>
#include <fstream>
#include <cctype>
#include <vector>
#include <string>
using namespace std;

/* Design Parameters:
 * Word Size = 16 bit
 * 8 General Purpose Registers R0 to R7. 16-bits each. R0 always contains a read-only zero.
 * Memory is word addressable. 16-bit address. Memory capacity is 128 KB.
 * Supported 16-bit instructions: LOAD, STORE, BNE, CALL, RET, ADD, ADDI, NAND, DIV
 */

class instruction
{
public:
    int instCode;
    string instName;
    int rA;
    int rB;
    int rC;
    int imm_offset_label;
    void printInstruction();
};

string uppercase(string s);
void getInstructionsFromFile(string filename);

vector<instruction> instructions;

int main()
{

    getInstructionsFromFile("input.txt");

    for (int i = 0; i < instructions.size(); i++)
    {
        instructions[i].printInstruction();
    }

    return 0;
}

string uppercase(string s)
{
    for (auto &c : s)
        c = toupper(c);
    return s;
}

void getInstructionsFromFile(string filename)
{
    ifstream inputFile(filename);
    if (!inputFile)
    {
        cout << "\n\nUnable to open inputFile. Program terminated.";
        exit(0);
    }

    string s, temp;
    instruction i;

    while (!inputFile.eof())
    {
        inputFile >> s;
        s = uppercase(s);

        if (s == "LOAD" || s == "STORE")
        {
            i.instCode = (s == "LOAD") ? 1 : 2;
            i.instName = s;
            inputFile >> s;
            s.erase(0, 1);              // delete 'R'
            s.erase(s.length() - 1, 1); // delete ','
            i.rA = stoi(s);
            inputFile >> s;
            temp = s;
            s.erase(s.find('('), s.length() - s.find('(')); // delete "(R#)"
            i.imm_offset_label = stoi(s);
            temp.erase(0, temp.find('R') + 1);
            temp.erase(temp.find(')'), temp.length() - temp.find(')')); // delete "#(R" and ")"
            i.rB = stoi(temp);
        }
        else if (s == "BNE" || s == "ADDI")
        {
            i.instCode = (s == "BNE") ? 3 : 7;
            i.instName = s;
            inputFile >> s;
            s.erase(0, 1);
            s.erase(s.length() - 1, 1);
            i.rA = stoi(s);
            inputFile >> s;
            s.erase(0, 1);
            s.erase(s.length() - 1, 1);
            i.rB = stoi(s);
            inputFile >> s;
            i.imm_offset_label = stoi(s);
        }
        else if (s == "CALL")
        {
            i.instCode = 4;
            i.instName = s;
            inputFile >> s;
            i.imm_offset_label = stoi(s);
        }
        else if (s == "RET")
        {
            i.instCode = 5;
            i.instName = s;
        }
        else if (s == "ADD" || s == "NAND" || s == "DIV")
        {
            i.instCode = (s == "ADD") ? 6 : ((s == "NAND") ? 8 : 9);
            i.instName = s;
            inputFile >> s;
            s.erase(0, 1);
            s.erase(s.length() - 1, 1);
            i.rA = stoi(s);
            inputFile >> s;
            s.erase(0, 1);
            s.erase(s.length() - 1, 1);
            i.rB = stoi(s);
            inputFile >> s;
            s.erase(0, 1);
            i.rC = stoi(s);
        }
        else
        {
            cout << "\n\nError: unknown instruction. Program terminated.";
            exit(0);
        }
        instructions.push_back(i);
    }

    inputFile.close();
}

void instruction::printInstruction()
{
    cout << "\n"
         << instCode << " ";

    switch (instCode)
    {
    case 1: // LOAD
        cout << instName << " R" << rA << ", " << imm_offset_label << "(R" << rB << ")";
        break;

    case 2: // STORE, same formatting as case LOAD
        cout << instName << " R" << rA << ", " << imm_offset_label << "(R" << rB << ")";
        break;

    case 3: // BNE
        cout << instName << " R" << rA << ", R" << rB << ", " << imm_offset_label;
        break;

    case 4: // CALL
        cout << instName << " " << imm_offset_label;
        break;

    case 5: // RET
        cout << instName;
        break;

    case 6: // ADD
        cout << instName << " R" << rA << ", R" << rB << ", R" << rC;
        break;

    case 7: // ADDI, same formatting as BNE
        cout << instName << " R" << rA << ", R" << rB << ", " << imm_offset_label;
        break;

    case 8: // NAND, same formatting as ADD
        cout << instName << " R" << rA << ", R" << rB << ", R" << rC;
        break;

    case 9: // NAND, same formatting as ADD
        cout << instName << " R" << rA << ", R" << rB << ", R" << rC;
        break;

    default:
        cout << "\n\nError: unsupported instCode. Program terminated.";
        exit(0);
        break;
    }
}