#include <iostream>
#include <fstream>
#include <cctype>
#include <vector>
#include <string>
#include <map>
#include <queue>
using namespace std;

/* Design Parameters:
 * Word Size = 16 bit
 * 8 General Purpose Registers R0 to R7. 16-bits each. R0 always contains a read-only zero.
 * Memory is word addressable. 16-bit address. Memory capacity is 128 KB.
 * Supported 16-bit instructions: LOAD, STORE, BNE, CALL, RET, ADD, ADDI, NAND, DIV
 */

/* Notes and Assumptions:
 * PC starts at zero
 * PC increments by 1
 * Each label is assigned a value that refers to the index of the desired location in the instructions vector
 */

/* TODO:
 * note: R0 is a read-only zero
 * implement PC
 * implement issueInstruction()
 * implement executeInstruction()
 * implement writeInstruction()
 * investigate reorder buffer
 */

class instruction
{
public:
    int instCode; // can be used to filter cases by type uniquely
    string instName;
    int rA;
    int rB;
    int rC;
    char imm_offset_label;
    int index;
    int issuedCycle;
    int executedCycleStart;
    int executedCycleEnd;
    int writeCycle;
    void printInstruction();
};

class reservationStation
{
public:
    string name;
    int numCyclesNeeded;

    char busy;
    string op;

    string Vj;
    int Vj_calculated; // Value of Source operands

    string Vk;
    int Vk_calculated; // Value of Source operands

    string Qj; // Reservation stations producing source registers (value to be written)
    string Qk; // Reservation stations producing source registers (value to be written)

    string A; // Address information for Load and Store; initially contains the immediate value; then the full address
    int A_calculated;
};

string uppercase(string s);
void getInstructionsFromFile(string filename);
void getReservationStationsParameters();
void printReservationStationsParameters();

vector<instruction> instructions;
vector<short> reg(8); // 8 General Purpose Registers R0 to R7. 16-bits each. R0 always contains a read-only zero.
vector<reservationStation> load_ReservationStations;
vector<reservationStation> store_ReservationStations;
vector<reservationStation> bne_ReservationStations;
vector<reservationStation> call_ret_ReservationStations;
vector<reservationStation> add_addi_ReservationStations;
vector<reservationStation> nand_ReservationStations;
vector<reservationStation> div_ReservationStations;
map<string, int> labelMap;
queue<instruction> writebackQueue; // TODO: use when more than one instruction needs to be written back at the same cycle

short mem[65536]; // 16 bit * 65536 entry = 128 KB memory

int main()
{
    getInstructionsFromFile("input.txt");
    for (int i = 0; i < instructions.size(); i++)
        instructions[i].printInstruction();

    cout << endl
         << endl
         << endl;

    // input: 2 3 2 3 1 1 1 1 3 2 1 1 1 10
    getReservationStationsParameters();

    cout << endl
         << endl
         << endl;
    printReservationStationsParameters();

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
        cout << "\n\nError: unable to open inputFile. Program terminated.";
        exit(0);
    }

    string s, s2, temp;
    instruction i;
    int temp2;
    bool wasLabel = false;

    while (!inputFile.eof())
    {
        inputFile >> s;
        s2 = s;
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
            temp2 = stoi(s);
            temp2 = temp2 & 63;
            i.imm_offset_label = (temp2 & 32) ? (temp2 | 193) : temp2; // set to signed 6 bits
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
            temp2 = stoi(s);
            temp2 = temp2 & 63;
            i.imm_offset_label = (temp2 & 32) ? (temp2 | 193) : temp2; // set to signed 6 bits
        }
        else if (s == "CALL")
        {
            i.instCode = 4;
            i.instName = s;
            inputFile >> s;
            // temp2 = stoi(s);
            // temp2 = temp2 & 63;
            // i.imm_offset_label = (temp2 & 32) ? (temp2 | 193) : temp2; // set to signed 6 bits
            i.imm_offset_label = labelMap[s];
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
        else // LABEL
        {
            // inputFile >> s;
            s2.erase(s2.length() - 1, 1); // delete ':'
            labelMap.insert(pair<string, int>(s2, instructions.size()));
            wasLabel = true;
        }

        if (!wasLabel)
        {
            i.index = instructions.size();
            instructions.push_back(i);
        }

        wasLabel = false;
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
        cout << instName << " R" << rA << ", " << (int)imm_offset_label << "(R" << rB << ")";
        break;

    case 2: // STORE, same formatting as case LOAD
        cout << instName << " R" << rA << ", " << (int)imm_offset_label << "(R" << rB << ")";
        break;

    case 3: // BNE
        cout << instName << " R" << rA << ", R" << rB << ", " << (int)imm_offset_label;
        break;

    case 4: // CALL

        cout << instName << " ";
        for (auto &it : labelMap)
            if (it.second == imm_offset_label)
                cout << it.first;

        cout << "  // PC=" << (int)imm_offset_label;
        break;

    case 5: // RET
        cout << instName;
        break;

    case 6: // ADD
        cout << instName << " R" << rA << ", R" << rB << ", R" << rC;
        break;

    case 7: // ADDI, same formatting as BNE
        cout << instName << " R" << rA << ", R" << rB << ", " << (int)imm_offset_label;
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

void getReservationStationsParameters()
{
    // input: 2 3 2 3 1 1 1 1 3 2 1 1 1 10
    int n, c;
    cout << "\n\nEnter number of available LOAD units: ";
    cin >> n;
    cout << "\nEnter number of cycles needed for LOAD units: ";
    cin >> c;
    load_ReservationStations.resize(n);
    for (int i = 0; i < n; i++)
    {
        load_ReservationStations[i].name = "LOAD" + to_string(i + 1);
        load_ReservationStations[i].numCyclesNeeded = c;
    }

    cout << "\n\nEnter number of available STORE units: ";
    cin >> n;
    cout << "\nEnter number of cycles needed for STORE units: ";
    cin >> c;
    store_ReservationStations.resize(n);
    for (int i = 0; i < n; i++)
    {
        store_ReservationStations[i].name = "STORE" + to_string(i + 1);
        store_ReservationStations[i].numCyclesNeeded = c;
    }

    cout << "\n\nEnter number of available BNE units: ";
    cin >> n;
    cout << "\nEnter number of cycles needed for BNE units: ";
    cin >> c;
    bne_ReservationStations.resize(n);
    for (int i = 0; i < n; i++)
    {
        bne_ReservationStations[i].name = "BNE" + to_string(i + 1);
        bne_ReservationStations[i].numCyclesNeeded = c;
    }

    cout << "\n\nEnter number of available CALL/RET units: ";
    cin >> n;
    cout << "\nEnter number of cycles needed for CALL/RET units: ";
    cin >> c;
    call_ret_ReservationStations.resize(n);
    for (int i = 0; i < n; i++)
    {
        call_ret_ReservationStations[i].name = "CALL/RET" + to_string(i + 1);
        call_ret_ReservationStations[i].numCyclesNeeded = c;
    }

    cout << "\n\nEnter number of available ADD/ADDI units: ";
    cin >> n;
    cout << "\nEnter number of cycles needed for ADD/ADDI units: ";
    cin >> c;
    add_addi_ReservationStations.resize(n);
    for (int i = 0; i < n; i++)
    {
        add_addi_ReservationStations[i].name = "ADD/ADDI" + to_string(i + 1);
        add_addi_ReservationStations[i].numCyclesNeeded = c;
    }

    cout << "\n\nEnter number of available NAND units: ";
    cin >> n;
    cout << "\nEnter number of cycles needed for NAND units: ";
    cin >> c;
    nand_ReservationStations.resize(n);
    for (int i = 0; i < n; i++)
    {
        nand_ReservationStations[i].name = "NAND" + to_string(i + 1);
        nand_ReservationStations[i].numCyclesNeeded = c;
    }

    cout << "\n\nEnter number of available DIV units: ";
    cin >> n;
    cout << "\nEnter number of cycles needed for DIV units: ";
    cin >> c;
    div_ReservationStations.resize(n);
    for (int i = 0; i < n; i++)
    {
        div_ReservationStations[i].name = "DIV" + to_string(i + 1);
        div_ReservationStations[i].numCyclesNeeded = c;
    }
}

void printReservationStationsParameters()
{
    cout << "Unique reservation units: " << load_ReservationStations[0].name << " " << store_ReservationStations[0].name << " " << bne_ReservationStations[0].name << " " << call_ret_ReservationStations[0].name << " " << add_addi_ReservationStations[0].name << " " << nand_ReservationStations[0].name << " " << div_ReservationStations[0].name;
    cout << endl;
    cout << "Number of available units: " << load_ReservationStations.size() << " " << store_ReservationStations.size() << " " << bne_ReservationStations.size() << " " << call_ret_ReservationStations.size() << " " << add_addi_ReservationStations.size() << " " << nand_ReservationStations.size() << " " << div_ReservationStations.size();
    cout << endl;
    cout << "Number of cycles needed: " << load_ReservationStations[0].numCyclesNeeded << " " << store_ReservationStations[0].numCyclesNeeded << " " << bne_ReservationStations[0].numCyclesNeeded << " " << call_ret_ReservationStations[0].numCyclesNeeded << " " << add_addi_ReservationStations[0].numCyclesNeeded << " " << nand_ReservationStations[0].numCyclesNeeded << " " << div_ReservationStations[0].numCyclesNeeded;
}