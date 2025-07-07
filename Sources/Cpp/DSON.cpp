#include "DSON.hpp"

#include <iostream>
#include <fstream>

namespace DSON {

// ----------------------------------------------------------------------------
// DSON Node
// ----------------------------------------------------------------------------

Node::Node() { RawNode = DSON_CreateEmptyNode(); }
Node::Node(DSON_Node* IN_RawNode) { RawNode = IN_RawNode; }
Node::~Node() { if (RawNode != nullptr) { DSON_FreeNode(RawNode); } }

void Node::AddChild(Node& REF_Node) {
    DSON_Node_AddChild(RawNode, REF_Node.GetRawNode());
}

bool Node::AddValue(std::string IN_Key, std::string IN_Value, bool IN_isAllowOverride) {
    return DSON_Node_AddValueString(RawNode, &IN_Key[0], &IN_Value[0], IN_isAllowOverride);
}
bool Node::AddValue(std::string IN_Key, long long IN_Value, bool IN_isAllowOverride) {
    return DSON_Node_AddValueInt(RawNode, &IN_Key[0], IN_Value, IN_isAllowOverride);
}
bool Node::AddValue(std::string IN_Key, unsigned long long IN_Value, bool IN_isAllowOverride) {
    return DSON_Node_AddValueUInt(RawNode, &IN_Key[0], IN_Value, IN_isAllowOverride);
}
bool Node::AddValue(std::string IN_Key, float IN_Value, bool IN_isAllowOverride) {
    return DSON_Node_AddValueFloat(RawNode, &IN_Key[0], IN_Value, IN_isAllowOverride);
}

bool Node::GetValue(std::string IN_Key, std::string& OUT_Value) {
    char* Result = DSON_Node_GetValueString(RawNode, &IN_Key[0]);
    if (!Result) { return false; }

    OUT_Value = std::string(Result);
    free(Result);

    return true;
}
bool Node::GetValue(std::string IN_Key, long long& OUT_Value) {
    return DSON_Node_GetValueInt(RawNode, &IN_Key[0], &OUT_Value);
}
bool Node::GetValue(std::string IN_Key, unsigned long long& OUT_Value) {
    return DSON_Node_GetValueUInt(RawNode, &IN_Key[0], &OUT_Value);
}
bool Node::GetValue(std::string IN_Key, float& OUT_Value) {
    return DSON_Node_GetValueFloat(RawNode, &IN_Key[0], &OUT_Value);
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

}
