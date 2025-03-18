#include "DSON.hpp"

#include <iostream>
#include <fstream>

// ------------------------------------------------------------------------------------------------------------------
// DSON NameSpace
// ------------------------------------------------------------------------------------------------------------------

namespace DSON {
    void Log_Error(std::string IN_Msg) {
        IN_Msg = "[ERROR] " + IN_Msg;
        if (LogCallback) { LogCallback(IN_Msg); }
        else { std::cout << IN_Msg << std::endl; }
    }

    void Log_Debug(std::string IN_Msg) {
        IN_Msg = "[DEBUG] " + IN_Msg;
        if (LogCallback) { LogCallback(IN_Msg); }
        else { std::cout << IN_Msg << std::endl; }
    }

    DSON_Node* DefaultAllocator(void* IN_Ptr) {
        (void)IN_Ptr;
        return new DSON_Node();
    }

    std::vector<std::string> SplitString(std::string IN_Str, char IN_Delimiter) {
        std::vector<std::string> Result;

        UInt L = IN_Str.length();
        if (L > 0) {
            std::string Temp = "";
            for (UInt X = 0; X < L; X++) {
                if (IN_Str[X] == IN_Delimiter) {
                    if (Temp.length() > 0) {
                        Result.push_back(Temp);
                        Temp = "";
                    }
                    continue;
                }
                Temp += IN_Str[X];
            }
            if (Temp.length() > 0) {Result.push_back(Temp);}
        }

        return Result;
    }

    std::string ReplaceString(std::string IN_Str, std::string IN_From, std::string IN_To) {
        // https://stackoverflow.com/questions/5878775/how-to-find-and-replace-string
        std::string buf;
        std::size_t pos = 0;
        std::size_t prevPos;

        // Reserves rough estimate of final size of string.
        buf.reserve(IN_Str.size() * 2);

        while (true) {
            prevPos = pos;
            pos = IN_Str.find(IN_From, pos);
            if (pos == std::string::npos) { break; }
            buf.append(IN_Str, prevPos, pos - prevPos);
            buf += IN_To;
            pos += IN_From.size();
        }

        buf.append(IN_Str, prevPos, IN_Str.size() - prevPos);
        IN_Str.swap(buf);

        return IN_Str;
    }
}

// ------------------------------------------------------------------------------------------------------------------
// DSON_Node
// ------------------------------------------------------------------------------------------------------------------

DSON_Node::~DSON_Node() {
    UInt Num = Childs.size();
    if (Num > 0) {
        for (UInt X = 0; X < Num; X++) { delete Childs[X]; }
    }
}

UInt DSON_Node::GetIndexOf(std::string IN_Name) {
    UInt Num = Childs.size();
    if (Num == 0) { return DSON_UINT_MAX; }

    for (UInt X = 0; X < Num; X++) {
        if (Childs[X]->Name == IN_Name) { return X; }
    }

    return DSON_UINT_MAX;
}


DSON_Node* DSON_Node::GetNodeOf(std::string IN_Name) {
    if (IN_Name.length() == 0) {
        DSON::Log_Error("GetNodeOf() => Key cannot be empty");
        return nullptr;
    }

    std::vector<std::string> Parts = DSON::SplitString(IN_Name,'/');
    UInt I = GetIndexOf(Parts[0]);
    if (I == DSON_UINT_MAX) {
        DSON::Log_Error(DSON_MAKE_STRING(
            "GetNodeOf() => Key [{0}] not found in Node [{1}]",
            IN_Name,
            Name
        ));
        return nullptr;
    }

    UInt Num = Parts.size();
    if (Num == 1) { return Childs[I]; }

    std::string NewValue = "";
    for (UInt X = 1; X < Num; X++) {
        if (X > 1) { NewValue += "/"; }
        NewValue += Parts[X];
    }

    return Childs[I]->GetNodeOf(NewValue);
}


bool DSON_Node::GetValueOf(std::string IN_Name, std::string& OUT_Value) {
    OUT_Value = "";

    if (IN_Name.length() == 0) {
        DSON::Log_Error("GetValueOf() => Key Cannot be empty");
        return false;
    }

    std::vector<std::string> Parts = DSON::SplitString(IN_Name,'/');
    UInt I = GetIndexOf(Parts[0]);
    if (I == DSON_UINT_MAX) {
        DSON::Log_Error(DSON_MAKE_STRING(
            "GetValueOf() => Key [{0}] not found in Node [{1}]",
            IN_Name,
            Name
        ));
        return false;
    }

    UInt Num = Parts.size();
    if (Num == 1) {
        if (Childs[I]->isValue()) { OUT_Value = Childs[I]->Value; }
        else { OUT_Value = DSON_STR_COUNT; }
        return true;
    }

    std::string NewValue = "";
    for (UInt X = 1; X < Num; X++) {
        if (X > 1) { NewValue += "/"; }
        NewValue += Parts[X];
    }

    return Childs[I]->GetValueOf(NewValue, OUT_Value);
}


bool DSON_Node::AddValue(std::string IN_Name, std::string IN_Value, bool IN_isAllowOverride) {
    if (IN_Name.length() == 0) {
        DSON::Log_Error("Failed to add Value. Key cannot be empty.");
        return false;
    }

    std::vector<std::string> Parts = DSON::SplitString(IN_Name,'/');
    UInt Num = Parts.size();

    if (Num == 1) {
        if (isValue()) {
            DSON::Log_Error(DSON_MAKE_STRING(
                "Failed to add Value. Key [{0}] is not a group.",
                Name
            ));
            return false;
        }

        UInt Index = GetIndexOf(Parts[0]);
        if (Index < DSON_UINT_MAX) {
            if (!IN_isAllowOverride) {
                DSON::Log_Error(DSON_MAKE_STRING(
                    "Failed to add new Subvalue to [{0}]. Key [{1}] already exist",
                    Name,
                    IN_Name
                ));
                return false;
            }
           else {
                if (!Childs[Index]->isValue()) {
                    DSON::Log_Error(DSON_MAKE_STRING(
                        "Failed to Override key [{0}] in Node [{1}]. Key is a group not a value.",
                        IN_Name,
                        Name
                    ));
                    return false;
                }
                else {
                    DSON::Log_Debug(DSON_MAKE_STRING(
                        "Override Key [{0}] in Node [{1}] to [{2}]",
                        IN_Name,
                        Name,
                        IN_Value
                    ));
                    Childs[Index]->Value = IN_Value;
                    return true;
                }
            }
        }

        DSON::Log_Debug(DSON_MAKE_STRING(
            "Adding new Subvalue to [{0}] (Key = {1}, Value = {2})",
            Name,
            IN_Name,
            IN_Value
        ));
        DSON_Node* NewNode = DSON::MemAllocCallback(DSON::MemAllocParam);
        NewNode->Name = IN_Name;
        NewNode->Value = IN_Value;
        NewNode->ParentPtr = this;
        NewNode->Level = Level + 1;
        Childs.push_back(NewNode);
        return true;
    }
    else {
        UInt Index = GetIndexOf(Parts[0]);
        if (Index == DSON_UINT_MAX) {
            DSON::Log_Error(DSON_MAKE_STRING(
                "Failed to add value. Key [{0}] not found in node [{1}]",
                Parts[0],
                Name
            ));
            return false;
        }
        std::string NewName = "";
        for (UInt X = 1; X < Num; X++) { if (X > 1) {NewName += "/";} NewName += Parts[X]; }
        return Childs[Index]->AddValue(NewName,IN_Value, IN_isAllowOverride);
    }
}
bool DSON_Node::AddEmptyGroup(std::string IN_Name) {
    if (IN_Name.length() == 0) {
        DSON::Log_Error("Failed to add empty group. Key cannot be empty.");
        return false;
    }

    std::vector<std::string> Parts = DSON::SplitString(IN_Name,'/');
    UInt Num = Parts.size();

    if (Num == 1) {
        if (isValue()) {
            DSON::Log_Error(DSON_MAKE_STRING(
                "Failed to add empyy group to Node [{0}]. Node is not a group.",
                Name
            ));
            return false;
        }

        DSON_Node* NewNode = DSON::MemAllocCallback(DSON::MemAllocParam);
        NewNode->Name = IN_Name;
        NewNode->ParentPtr = this;
        NewNode->Level = Level + 1;
        Childs.push_back(NewNode);

        DSON::Log_Debug(DSON_MAKE_STRING(
            "Adding empty Group to [{0}] (Name = {1})",
            Name,
            IN_Name
        ));
        return true;
    }
    else {
        UInt Index = GetIndexOf(Parts[0]);
        if (Index != DSON_UINT_MAX) {
            DSON::Log_Error(DSON_MAKE_STRING(
                "Failed to add empty group. Key [{0}] already exist in node [{1}]",
                Parts[0],
                Name
            ));
            return false;
        }
        if (!AddEmptyGroup(Parts[0])) { return false; }

        std::string NewName = "";
        for (UInt X = 1; X < Num; X++) { if (X > 1) {NewName += "/";} NewName += Parts[X]; }
        return Childs[Childs.size() - 1]->AddEmptyGroup(NewName);
    }
}

bool DSON_Node::RemoveElement(std::string IN_Name) {
    if (IN_Name.length() == 0) {
        DSON::Log_Error("Failed to add RemoveElement. Key cannot be empty.");
        return false;
    }

    std::vector<std::string> Parts = DSON::SplitString(IN_Name,'/');
    UInt Num = Parts.size();

    UInt Index = GetIndexOf(Parts[0]);
    if (Index == DSON_UINT_MAX) {
        DSON::Log_Error(DSON_MAKE_STRING(
            "Failed to RemoveElement. Key [{0}] does not exist in node [{1}]",
            Parts[0],
            Name
        ));
        return false;
    }

    if (Num == 1) {
        delete Childs[Index];
        Childs.erase(Childs.begin() + Index);

        DSON::Log_Debug(DSON_MAKE_STRING(
            "Removing Elmenet [{0}] from Node [{1}]",
            Parts[0],
            Name
        ));

        return true;
    }
    else {
        std::string NewName = "";
        for (UInt X = 1; X < Num; X++) { if (X > 1) {NewName += "/";} NewName += Parts[X]; }
        return Childs[Index]->RemoveElement(NewName);
    }
}

bool DSON_Node::GetisKeyExist(std::string IN_Name) {
    std::string Temp = "";
    GetValueOf(IN_Name, Temp);
    if (Temp == DSON_STR_NULL) { return false; }
    else { return true; }
}

bool DSON_Node::isValue() { return Value.length() > 0; }

bool DSON_Node::isGroup() { return Childs.size() > 0; }

bool DSON_Node::isEmpty() { if (!isValue() && !isGroup()) { return true; } else { return false; } }

std::string DSON_Node::ToString() {
    std::string Indent(Level * 2, ' ');

    std::string Result = Indent + Name + " = ";

    if (isValue()) { Result = Result + "[" + Value + "]"; }
    else if (isGroup()) {
        Result += "{\n";
        UInt Num = Childs.size();
        for (UInt X = 0; X < Num; X++) { Result += Childs[X]->ToString() + "\n"; }
        Result += Indent + "}\n";
    }
    else { Result += DSON_STR_EMPTY; }

    return Result;
}

// ------------------------------------------------------------------------------------------------------------------
// DSON_Tree
// ------------------------------------------------------------------------------------------------------------------

UInt DSON_Tree::GetIndexOf(std::string IN_Name) {
    if (Root == nullptr) {
        DSON::Log_Error("GetIndexOf() faile, DSON_Tree => RootNode is NULLPTR");
        return DSON_UINT_MAX;
    }
    return Root->GetIndexOf(IN_Name);
}
DSON_Node* DSON_Tree::GetNodeOf(std::string IN_Name) {
    if (Root == nullptr) {
        DSON::Log_Error("GetNodeOf() faile, DSON_Tree => RootNode is NULLPTR");
        return nullptr;
    }
    return Root->GetNodeOf(IN_Name);
}

bool DSON_Tree::GetValueOf(std::string IN_Name, std::string& OUT_Value) {
    if (Root == nullptr) {
        DSON::Log_Error("GetValueOf() faile, DSON_Tree => RootNode is NULLPTR");
        return false;
    }
    return Root->GetValueOf(IN_Name, OUT_Value);
}
bool DSON_Tree::AddValue(std::string IN_Name, std::string IN_Value, bool IN_isAllowOverride) {
    if (Root == nullptr) {
        DSON::Log_Error("AddValue() faile, DSON_Tree => RootNode is NULLPTR");
        return false;
    }
    return Root->AddValue(IN_Name, IN_Value, IN_isAllowOverride);
}
bool DSON_Tree::AddEmptyGroup(std::string IN_Name) {
    if (Root == nullptr) {
        DSON::Log_Error("AddEmptyGroup() faile, DSON_Tree => RootNode is NULLPTR");
        return false;
    }
    return Root->AddEmptyGroup(IN_Name);
}

bool DSON_Tree::RemoveElement(std::string IN_Name) {
    if (Root == nullptr) {
        DSON::Log_Error("RemoveElement() faile, DSON_Tree => RootNode is NULLPTR");
        return false;
    }
    return Root->RemoveElement(IN_Name);
}

bool DSON_Tree::GetisKeyExist(std::string IN_Name) {
    if (Root == nullptr) {
        DSON::Log_Error("GetisKeyExist() faile, DSON_Tree => RootNode is NULLPTR");
        return false;
    }
    return Root->GetisKeyExist(IN_Name);
}

// ------------------------------------------------------------------------------------------------------------------
// DSON_Parser
// ------------------------------------------------------------------------------------------------------------------

DSON_Parser::DSON_TokeType DSON_Parser::_Internal_GetTokenType(std::string IN_Token) {
    UInt Len = IN_Token.length();

    if (Len == 0) { return DSON_TokeType::UNDEFINED; }

    if (Len == 1) {
        if (IN_Token[0] == '=') { return DSON_TokeType::EQUAL; }
        if (IN_Token[0] == '[') { return DSON_TokeType::OPEN_B; }
        if (IN_Token[0] == ']') { return DSON_TokeType::CLOSE_B; }
        if (IN_Token[0] == '{') { return DSON_TokeType::OPEN_C; }
        if (IN_Token[0] == '}') { return DSON_TokeType::CLOSE_C; }
    }

    return DSON_TokeType::LITERAL;
}

bool DSON_Parser::_Internal_ParseLine(std::string IN_Line, UInt IN_LineNum) {
    DSON::Log_Debug(DSON_MAKE_STRING(
        "Parseing Line [{0}]: {1}",
        IN_LineNum,
        IN_Line
    ));

    if (IN_Line.length() == 0) { return true; }     // Skip Empty Line
    if (IN_Line[0] == '#') { return true; }         // Skip Comment Line

    // State Expection list:
    //	0 - Literal Name (Also handles '}')
    //	1 - '='
    //	2 - '[' or '{'
    //	3 - Literal Value
    //	4 - ']'
    UChar State = 0;

    std::vector<std::string> Tokens = DSON::SplitString(IN_Line,' ');
    UInt TokenNum = Tokens.size();
    UInt CharNum = 0;

    for (UInt X = 0; X < TokenNum; X++) {
        DSON_TokeType Type = _Internal_GetTokenType(Tokens[X]);

        if (State == 0) {
            if (Type == DSON_TokeType::CLOSE_C && CurrNode != RootNode) {
                if (CurrNode->ParentPtr != nullptr) {
                    CurrNode = CurrNode->ParentPtr;
                    goto EndOfTokenLoop;
                }
            }
            if (Type != DSON_TokeType::LITERAL) {
                DSON::Log_Error(DSON_MAKE_STRING(
                    "Unexpected '{0}' at line [{1}:{2}]. Expected a literal name.",
                    Tokens[X],
                    IN_LineNum,
                    CharNum
                ));
                return false;
            }

            DSON_Node* NewNode = DSON::MemAllocCallback(DSON::MemAllocParam);
            NewNode->ParentPtr = CurrNode;
            NewNode->Level = CurrNode->Level + 1;
            CurrNode->Childs.push_back(NewNode);
            CurrNode = NewNode;

            CurrNode->Name = Tokens[X];
            State = 1;
        }
        else if (State == 1) {
            if (Type != DSON_TokeType::EQUAL) {
                DSON::Log_Error(DSON_MAKE_STRING(
                    "Unexpected '{0}' at line [{1}:{2}]. Expected a '=' sign.",
                    Tokens[X],
                    IN_LineNum,
                    CharNum
                ));
                return false;
            }

            State = 2;
        }
        else if (State == 2) {
            if (Type == DSON_TokeType::OPEN_B) {
                State = 3;
                goto EndOfTokenLoop;
            }
            if (Type == DSON_TokeType::OPEN_C) {
                // Dont create a new Node here,
                // The next expected token is a Literal name
                // Taht will create a new node for itself anyway
                State = 0;
                goto EndOfTokenLoop;
            }

            DSON::Log_Error(DSON_MAKE_STRING(
                "Unexpected '{0}' at line [{1}:{2}]. Expected a value or a group definition ('[' or '{').",
                Tokens[X],
                IN_LineNum,
                CharNum
            ));
            return false;
        }
        else if (State == 3) {
            if (Type != DSON_TokeType::LITERAL) {
                DSON::Log_Error(DSON_MAKE_STRING(
                    "Unexpected '{0}' at line [{1}:{2}]. Expected a literal value.",
                    Tokens[X],
                    IN_LineNum,
                    CharNum
                ));
                return false;
            }
            CurrNode->Value = Tokens[X];
            State = 4;
        }
        else if (State == 4) {
            if (Type != DSON_TokeType::CLOSE_B) {
                DSON::Log_Error(DSON_MAKE_STRING(
                    "Unexpected '{0}' at line [{1}:{2}]. Expected an end of value definition (']').",
                    Tokens[X],
                    IN_LineNum,
                    CharNum
                ));
                return false;
            }
            CurrNode = CurrNode->ParentPtr;
            State = 0;
        }

        EndOfTokenLoop:
        CharNum += Tokens[X].length();
    }

    switch (State) {
        case 0: return true;
        case 1:
            DSON::Log_Error(DSON_MAKE_STRING(
                "Unexpected end of line {1}. Expected a literal name.",
                IN_LineNum
            ));
            return false;
        case 2:
            DSON::Log_Error(DSON_MAKE_STRING(
                "Unexpected end of line {1}. Expecting value of group definition ('[' or '{').",
                IN_LineNum
            ));
            return false;
        case 3:
            DSON::Log_Error(DSON_MAKE_STRING(
                "Unexpected end of line {1}. Expecting Literal Value.",
                IN_LineNum
            ));
            return false;
        case 4:
            DSON::Log_Error(DSON_MAKE_STRING(
                "Unexpected end of line {1}. Expecting and end of value definition (']').",
                IN_LineNum
            ));
            return false;
        default: return false;
    }
}

void DSON_Parser::_Internal_Init() {
    RootNode = DSON::MemAllocCallback(DSON::MemAllocParam);
    RootNode->Name = "Root";
    CurrNode = RootNode;
}


DSON_Node* DSON_Parser::ParseFile(std::string IN_FileName) {
    DSON::Log_Debug(DSON_MAKE_STRING(
        "Parsing File [{0}]",
        IN_FileName
    ));
    std::ifstream InFile(IN_FileName.c_str());

    if (!InFile.is_open()) {
        DSON::Log_Error(DSON_MAKE_STRING("Unable to open source file [{0}]", IN_FileName));
        return nullptr;
    }

    _Internal_Init();

    std::string CurrLine = "";
    UInt LNum = 1;

    while (std::getline(InFile, CurrLine)) {
        if (!_Internal_ParseLine(CurrLine, LNum)) { return nullptr; }
        LNum++;
    }

    DSON::Log_Debug("File parsed Successfully");
    return RootNode;
}
DSON_Node* DSON_Parser::ParseString(std::string IN_Str) {
    if (IN_Str.length() == 0) {
        DSON::Log_Error("Unable to parse empty string.");
        return nullptr;
    }

    std::vector<std::string> Lines = DSON::SplitString(IN_Str,'\n');
    UInt Num = Lines.size();

    _Internal_Init();

    for (UInt X = 0; X < Num; X++) {
         if(!_Internal_ParseLine(Lines[X],X)) { return nullptr; }
    }

    return RootNode;
}


int main() { return 0; }
