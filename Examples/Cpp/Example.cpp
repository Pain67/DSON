#include <iostream>
#include "../../Sources/Cpp/DSON.hpp"

int main(int argc, char** argv) {
    std::cout << std::endl;

    if (argc == 1) {
        std::cout << "Missing required argument." << std::endl;
        std::cout << "Path to file to parse expected to be the 1st argument" << std::endl << std::endl;
        return -1;
    }
    char* Path = argv[1];
    std::cout << "Parsing text file [" << Path << "]..." << std::endl;

    DSON::Node Root = DSON::Node::ParseFromTextFile(Path);

    if (Root.GetRawNode() == nullptr) {
        std::cout << "Failed to parse source file. See details above" << std::endl << std::endl;
        return -1;
    }
    else {
        std::cout << "Source file parsed successfully. Loaded [" << std::to_string(Root.GetCount()) << "] nodes." << std::endl << std::endl;
    }
    std::cout << "\nPrinting Node Tree:\n-----------------------------------------\n\n";
    Root.Print();

    std::cout << "\n\nPrinting Node Tree (Compact):\n-----------------------------------------\n\n";
    Root.PrintCompact();

    std::cout << "\n\nPrinting Node Tree (Binary):\n-----------------------------------------\n\n";
    Root.PrintBinary();

    std::cout << "\nAdd new String Element (NewKey):\n-----------------------------------------\n\n";
    Root.AddValue("NewKey", "New Value", false);
    Root.Print();

    std::cout << "\nAdd new String Element (Chained keys):\n-----------------------------------------\n\n";
    Root.AddValue("NewChain/NewChain/NewChain/NewChain", "New Chain Value", false);
    Root.Print();

    "\nAttempt to add already existing key with no override allowed (NewKey):\n-----------------------------------------\n\n";
    Root.AddValue("NewKey", "New Value", false);

    std::cout << "\nAttempt to add already existing key with override allowed (NewKey):\n-----------------------------------------\n\n";
    Root.AddValue("NewKey", "Updated Value", true);

    std::cout << "\nAdd new Signed Integer Element (NewInt):\n-----------------------------------------\n\n";
    Root.AddValue("NewInt", (unsigned long long)-42, false);
    Root.Print();

    std::cout << "\nAdd new Unsigned Integer Element (NewUInt):\n-----------------------------------------\n\n";
    Root.AddValue("NewUInt", (long long)4294967295, false);
    Root.Print();

    std::cout << "\nAdd new floating point Element (NewFloat):\n-----------------------------------------\n\n";
    Root.AddValue("NewFloat", 12.324F, false);
    Root.Print();

    std::cout << "\nRead String Value:\n-----------------------------------------\n\n";
    std::string Str = "";
    if (Root.GetValue("name", Str)) {
        std::cout << "Value for key [name] = " << Str << std::endl;
    }
    else { std::cout << "Failed to read value for key [name]\n"; }

    std::string StrChain = "";
    if (Root.GetValue("bio/hobbies/0", Str)) {
        std::cout << "Value for key [bio/hobbies/0] = " << StrChain << std::endl;
    }
    else { std::cout << "Failed to read value for key [bio/hobbies/0]\n"; }


    std::cout << "\nRead Signed Integer Value:\n-----------------------------------------\n\n";
    long long TempInt = 0;
    if (Root.GetValue("NewInt", TempInt)) {
        std::string Temp = "Value for key [NewInt] = " + std::to_string(TempInt);
        std::cout << Temp << std::endl << std::endl;
    }
    else { std::cout << "Failed to read value for key [NewFloat]\n\n"; }


    std::cout << "\nRead Unsigned Integer Value:\n-----------------------------------------\n\n";
    unsigned long long TempUInt = 0;
    if (Root.GetValue("NewUInt", TempUInt)) {
        std::string Temp = "Value for key [NewUInt] = " + std::to_string(TempUInt);
        std::cout << Temp << std::endl << std::endl;
    }
    else { std::cout << "Failed to read value for key [NewUInt]\n\n"; }


    std::cout << "\nRead Unsigned Integer Value:\n-----------------------------------------\n\n";
    float TempFloat = 0.0F;
    if (Root.GetValue("NewFloat", TempFloat)) {
        std::string Temp = "Value for key [NewFloat] = " + std::to_string(TempFloat);
        std::cout << Temp << std::endl << std::endl;
    }
    else { std::cout << "Failed to read value for key [NewFloat]\n\n"; }

    std::cout << "\nRead Value for key that does not exist:\n-----------------------------------------\n\n";
    std::string NoKey = "";
    if (Root.GetValue("InvalidKey", NoKey)) {
        std::cout << "Failed to read value for key [InvalidKey]\n\n";
    }

    std::string NoChainKey = "";
    if (Root.GetValue("contacts/social/InvalidKey", NoChainKey)) {
        std::cout << "Failed to read value for key [contacts/social/InvalidKey]\n\n";
    }

    std::cout << "\nSave Node Tree\n-----------------------------------------\n\n";
    if (Root.SaveToTextFile("Text.dson")) {
        std::cout << "Save Node Tree to text file [Text.dson]\n";
    }
    else { std::cout << "Failed to Save Node Tree to binary file [Text.dson]\n"; }
    if (Root.SaveToCompactTextFile("CompactText.dson")) {
        std::cout << "Save Node Tree to compact text file [CompactText.dson]\n";
    }
    else { std::cout << "Failed to Save Node Tree to binary file [Binary.dson]\n"; }
    if (Root.SaveToBinaryFile("Binary.dson")) {
        std::cout << "Save Node Tree to binary file [Binary.dson]\n";
    }
    else { std::cout << "Failed to Save Node Tree to binary file [Binary.dson]\n"; }


    return 0;
}
