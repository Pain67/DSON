#include "DSON.hpp"

#include <iostream>
#include <fstream>

namespace DSON {

void SetLogCallbackFunction(DSON_LogFunction IN_Function) {
    DSON_LogCallback = IN_Function;
}
void SetMinLogLevel(int IN_MinLogLevel) {
    DSON_MinLogLevel = IN_MinLogLevel;
}

// ----------------------------------------------------------------------------
// DSON Node
// ----------------------------------------------------------------------------

Node::Node() { RawNode = DSON_CreateEmptyNode(); }
Node::Node(DSON_Node* IN_RawNode) { RawNode = IN_RawNode; }
Node::~Node() { if (RawNode != nullptr) { DSON_FreeNode(RawNode); } }

bool Node::AddValue(std::string IN_Key, std::string IN_Value, bool IN_isAllowOverride) {
    std::string Path = GetBasePath();
    if (Path.length() > 0) { Path += "/"; }
    Path += IN_Value;
    return DSON_Node_AddValueString(RawNode, &Path[0], &IN_Value[0], IN_isAllowOverride);
}
bool Node::AddValue(std::string IN_Key, int8_t IN_Value, bool IN_isAllowOverride) {
    std::string Path = GetBasePath();
    if (Path.length() > 0) { Path += "/"; }
    Path += IN_Value;
    return DSON_Node_AddValueInt(RawNode, &Path[0], (long long)IN_Value, IN_isAllowOverride);
}
bool Node::AddValue(std::string IN_Key, int16_t IN_Value, bool IN_isAllowOverride) {
    std::string Path = GetBasePath();
    if (Path.length() > 0) { Path += "/"; }
    Path += IN_Value;
    return DSON_Node_AddValueInt(RawNode, &Path[0], (long long)IN_Value, IN_isAllowOverride);
}
bool Node::AddValue(std::string IN_Key, int32_t IN_Value, bool IN_isAllowOverride) {
    std::string Path = GetBasePath();
    if (Path.length() > 0) { Path += "/"; }
    Path += IN_Value;
    return DSON_Node_AddValueInt(RawNode, &Path[0], (long long)IN_Value, IN_isAllowOverride);
}
bool Node::AddValue(std::string IN_Key, int64_t IN_Value, bool IN_isAllowOverride) {
    std::string Path = GetBasePath();
    if (Path.length() > 0) { Path += "/"; }
    Path += IN_Value;
    return DSON_Node_AddValueInt(RawNode, &Path[0], (long long)IN_Value, IN_isAllowOverride);
}
bool Node::AddValue(std::string IN_Key, uint8_t IN_Value, bool IN_isAllowOverride) {
    std::string Path = GetBasePath();
    if (Path.length() > 0) { Path += "/"; }
    Path += IN_Value;
    return DSON_Node_AddValueUInt(RawNode, &Path[0], (unsigned long long)IN_Value, IN_isAllowOverride);
}
bool Node::AddValue(std::string IN_Key, uint16_t IN_Value, bool IN_isAllowOverride) {
    std::string Path = GetBasePath();
    if (Path.length() > 0) { Path += "/"; }
    Path += IN_Value;
    return DSON_Node_AddValueUInt(RawNode, &Path[0], (unsigned long long)IN_Value, IN_isAllowOverride);
}
bool Node::AddValue(std::string IN_Key, uint32_t IN_Value, bool IN_isAllowOverride) {
    std::string Path = GetBasePath();
    if (Path.length() > 0) { Path += "/"; }
    Path += IN_Value;
    return DSON_Node_AddValueUInt(RawNode, &Path[0], (unsigned long long)IN_Value, IN_isAllowOverride);
}
bool Node::AddValue(std::string IN_Key, uint64_t IN_Value, bool IN_isAllowOverride) {
    std::string Path = GetBasePath();
    if (Path.length() > 0) { Path += "/"; }
    Path += IN_Value;
    return DSON_Node_AddValueUInt(RawNode, &Path[0], (unsigned long long)IN_Value, IN_isAllowOverride);
}
bool Node::AddValue(std::string IN_Key, float IN_Value, bool IN_isAllowOverride) {
    std::string Path = GetBasePath();
    if (Path.length() > 0) { Path += "/"; }
    Path += IN_Value;
    return DSON_Node_AddValueFloat(RawNode, &Path[0], IN_Value, IN_isAllowOverride);
}

bool Node::AddEmptyGroup(std::string IN_GroupName) {
    std::string Path = GetBasePath();
    DSON_Node* TargetPtr = RawNode;
    if (Path.length() > 0) {
        char* Temp = (char*)malloc(sizeof(char) * (Path.length() + 1));
        strcpy(Temp, Path.c_str());
        TargetPtr = DSON_GetSubNode(RawNode, Temp);
        free(Temp);
        if (TargetPtr == NULL) { return false; }
    }

    DSON_Node* Temp = DSON_CreateEmptyNode();
    Temp->Name = (char*)malloc(sizeof(char) * (IN_GroupName.length() + 1));
    strcpy(Temp->Name, IN_GroupName.c_str());

    DSON_Node_AddChild(TargetPtr, Temp);

    return true;
}

bool Node::GetValue(std::string IN_Key, std::string& OUT_Value) {
    std::string Path = GetBasePath();
    DSON_Node* TargetPtr = RawNode;
    if (Path.length() > 0) {
        char* Temp = (char*)malloc(sizeof(char) * (Path.length() + 1));
        strcpy(Temp, Path.c_str());
        TargetPtr = DSON_GetSubNode(RawNode, Temp);
        free(Temp);
        if (TargetPtr == NULL) { return false; }
    }

    char* Result = DSON_Node_GetValueString(TargetPtr, &IN_Key[0]);
    if (!Result) { return false; }

    OUT_Value = std::string(Result);
    free(Result);

    return true;
}
bool Node::GetValue(std::string IN_Key, int8_t& OUT_Value) {
    std::string Path = GetBasePath();
    DSON_Node* TargetPtr = RawNode;
    if (Path.length() > 0) {
        char* Temp = (char*)malloc(sizeof(char) * (Path.length() + 1));
        strcpy(Temp, Path.c_str());
        TargetPtr = DSON_GetSubNode(RawNode, Temp);
        free(Temp);
        if (TargetPtr == NULL) { return false; }
    }

    long long Temp = 0;
    bool Result = DSON_Node_GetValueInt(TargetPtr, &IN_Key[0], &Temp);
    OUT_Value = Temp;
    return Result;
}
bool Node::GetValue(std::string IN_Key, int16_t& OUT_Value) {
    std::string Path = GetBasePath();
    DSON_Node* TargetPtr = RawNode;
    if (Path.length() > 0) {
        char* Temp = (char*)malloc(sizeof(char) * (Path.length() + 1));
        strcpy(Temp, Path.c_str());
        TargetPtr = DSON_GetSubNode(RawNode, Temp);
        free(Temp);
        if (TargetPtr == NULL) { return false; }
    }

    long long Temp = 0;
    bool Result = DSON_Node_GetValueInt(TargetPtr, &IN_Key[0], &Temp);
    OUT_Value = Temp;
    return Result;
}
bool Node::GetValue(std::string IN_Key, int32_t& OUT_Value) {
    std::string Path = GetBasePath();
    DSON_Node* TargetPtr = RawNode;
    if (Path.length() > 0) {
        char* Temp = (char*)malloc(sizeof(char) * (Path.length() + 1));
        strcpy(Temp, Path.c_str());
        TargetPtr = DSON_GetSubNode(RawNode, Temp);
        free(Temp);
        if (TargetPtr == NULL) { return false; }
    }

    long long Temp = 0;
    bool Result = DSON_Node_GetValueInt(TargetPtr, &IN_Key[0], &Temp);
    OUT_Value = Temp;
    return Result;
}
bool Node::GetValue(std::string IN_Key, int64_t& OUT_Value) {
    std::string Path = GetBasePath();
    DSON_Node* TargetPtr = RawNode;
    if (Path.length() > 0) {
        char* Temp = (char*)malloc(sizeof(char) * (Path.length() + 1));
        strcpy(Temp, Path.c_str());
        TargetPtr = DSON_GetSubNode(RawNode, Temp);
        free(Temp);
        if (TargetPtr == NULL) { return false; }
    }

    long long Temp = 0;
    bool Result = DSON_Node_GetValueInt(TargetPtr, &IN_Key[0], &Temp);
    OUT_Value = Temp;
    return Result;
}
bool Node::GetValue(std::string IN_Key, uint8_t& OUT_Value) {
    std::string Path = GetBasePath();
    DSON_Node* TargetPtr = RawNode;
    if (Path.length() > 0) {
        char* Temp = (char*)malloc(sizeof(char) * (Path.length() + 1));
        strcpy(Temp, Path.c_str());
        TargetPtr = DSON_GetSubNode(RawNode, Temp);
        free(Temp);
        if (TargetPtr == NULL) { return false; }
    }

    unsigned long long Temp = 0;
    bool Result = DSON_Node_GetValueUInt(TargetPtr, &IN_Key[0], &Temp);
    OUT_Value = Temp;
    return Result;
}
bool Node::GetValue(std::string IN_Key, uint16_t& OUT_Value) {
    std::string Path = GetBasePath();
    DSON_Node* TargetPtr = RawNode;
    if (Path.length() > 0) {
        char* Temp = (char*)malloc(sizeof(char) * (Path.length() + 1));
        strcpy(Temp, Path.c_str());
        TargetPtr = DSON_GetSubNode(RawNode, Temp);
        free(Temp);
        if (TargetPtr == NULL) { return false; }
    }

    unsigned long long Temp = 0;
    bool Result = DSON_Node_GetValueUInt(TargetPtr, &IN_Key[0], &Temp);
    OUT_Value = Temp;
    return Result;
}
bool Node::GetValue(std::string IN_Key, uint32_t& OUT_Value) {
    std::string Path = GetBasePath();
    DSON_Node* TargetPtr = RawNode;
    if (Path.length() > 0) {
        char* Temp = (char*)malloc(sizeof(char) * (Path.length() + 1));
        strcpy(Temp, Path.c_str());
        TargetPtr = DSON_GetSubNode(RawNode, Temp);
        free(Temp);
        if (TargetPtr == NULL) { return false; }
    }

    unsigned long long Temp = 0;
    bool Result = DSON_Node_GetValueUInt(TargetPtr, &IN_Key[0], &Temp);
    OUT_Value = Temp;
    return Result;
}
bool Node::GetValue(std::string IN_Key, uint64_t& OUT_Value) {
    std::string Path = GetBasePath();
    DSON_Node* TargetPtr = RawNode;
    if (Path.length() > 0) {
        char* Temp = (char*)malloc(sizeof(char) * (Path.length() + 1));
        strcpy(Temp, Path.c_str());
        TargetPtr = DSON_GetSubNode(RawNode, Temp);
        free(Temp);
        if (TargetPtr == NULL) { return false; }
    }

    unsigned long long Temp = 0;
    bool Result = DSON_Node_GetValueUInt(TargetPtr, &IN_Key[0], &Temp);
    OUT_Value = Temp;
    return Result;
}
bool Node::GetValue(std::string IN_Key, float& OUT_Value) {
    std::string Path = GetBasePath();
    DSON_Node* TargetPtr = RawNode;
    if (Path.length() > 0) {
        char* Temp = (char*)malloc(sizeof(char) * (Path.length() + 1));
        strcpy(Temp, Path.c_str());
        TargetPtr = DSON_GetSubNode(RawNode, Temp);
        free(Temp);
        if (TargetPtr == NULL) { return false; }
    }

    return DSON_Node_GetValueFloat(TargetPtr, &IN_Key[0], &OUT_Value);
}

bool Node::Remove(std::string IN_Key) {
    return DSON_Node_Remove(RawNode, &IN_Key[0]);
}

std::string Node::ToString() {
    char* Temp = DSON_Node_ToString(RawNode);
    std::string Result(Temp);
    free(Temp);
    return Result;
}
std::string Node::ToCompactString() {
    char* Temp = DSON_Node_ToCompactString(RawNode);
    std::string Result(Temp);
    free(Temp);
    return Result;
}
std::string Node::ToBinaryString() {
    char* Temp = DSON_Node_ToBinaryString(RawNode);
    std::string Result(Temp);
    free(Temp);
    return Result;
}


bool Node::SaveToTextFile(std::string IN_FileName) {
    return DSON_SaveToTextFile(RawNode, &IN_FileName[0]);
}
bool Node::SaveToCompactTextFile(std::string IN_FileName) {
    return DSON_SaveToCompactTextFile(RawNode, &IN_FileName[0]);
}
bool Node::SaveToBinaryFile(std::string IN_FileName) {
    return DSON_SaveToBinaryFile(RawNode, &IN_FileName[0]);
}

void Node::Print() { DSON_Node_Print(RawNode); }
void Node::PrintCompact() { DSON_Node_PrintCompact(RawNode); }
void Node::PrintBinary() { DSON_Node_PrintBinary(RawNode); }

size_t Node::GetKeyIndex(std::string IN_Key) {
    return DSON_Node_GetKeyIndex(RawNode, &IN_Key[0]);
}
bool Node::GetisValue() { return DSON_Node_isValue(RawNode); }
bool Node::GetisGroup() { return DSON_Node_isGroup(RawNode); }
bool Node::GetisEmpty() { return DSON_Node_isEmpty(RawNode); }
int Node::GetCount() { return DSON_Node_Count(RawNode); }
DSON_Node* Node::GetRawNode() { return RawNode; }

std::string Node::GetBasePath() {
    std::string Result;
    size_t Num = BasePath.size();
    if (Num > 0) {
        for (size_t X = 0; X < Num; X++) {
            if (X > 0) { Result += "/"; }
            Result += BasePath[X];
        }
    }
    return Result;
}
void Node::SetBasePath(std::string IN_BasePath) {
    BasePath.clear();

    if (IN_BasePath.length() == 0) { return; }

    char* Buffer = (char*)malloc(sizeof(char) * (IN_BasePath.length() + 1));
    strcpy(Buffer, IN_BasePath.c_str());
    DSON_StringList TempList = DSON_SplitString(Buffer, '/');
    size_t Num = TempList.Num;
    if (Num > 0) {
        for (size_t X = 0; X < Num; X++) {
            std::string Temp(TempList.Entries[X]);
            BasePath.push_back(Temp);
        }
    }
    DSON_FreeStringList(&TempList);
    free(Buffer);
}

Node Node::ParseFromTextFile(std::string IN_FileName) {
    return(Node(DSON_ParseTextFile(&IN_FileName[0])));

}
Node Node::ParseFromString(std::string IN_String) {
    return(Node(DSON_ParseString(&IN_String[0])));
}
Node Node::CreateEmptyNode() {
    return Node(DSON_CreateEmptyNode());
}

// ----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------








