#pragma once

#include <string>
#include "DSON.h"


namespace DSON {


    class Node {
    private:
        DSON_Node* RawNode = nullptr;
    protected:

    public:
        Node();
        Node(DSON_Node* IN_RawNode);
        ~Node();

        void AddChild(Node& REF_Node);

        bool AddValue(std::string IN_Key, std::string IN_Value, bool IN_isAllowOverride);
        bool AddValue(std::string IN_Key, long long IN_Value, bool IN_isAllowOverride);
        bool AddValue(std::string IN_Key, unsigned long long IN_Value, bool IN_isAllowOverride);
        bool AddValue(std::string IN_Key, float IN_Value, bool IN_isAllowOverride);

        bool GetValue(std::string IN_Key, std::string& OUT_Value);
        bool GetValue(std::string IN_Key, long long& OUT_Value);
        bool GetValue(std::string IN_Key, unsigned long long& OUT_Value);
        bool GetValue(std::string IN_Key, float& OUT_Value);

        bool Remove(std::string IN_Key);

        std::string ToString();
        std::string ToCompactString();
        std::string ToBinaryString();

        bool SaveToTextFile(std::string IN_FileName);
        bool SaveToCompactTextFile(std::string IN_FileName);
        bool SaveToBinaryFile(std::string IN_FileName);

        void Print();
        void PrintCompact();
        void PrintBinary();

        size_t GetKeyIndex(std::string IN_Key);
        bool GetisValue();
        bool GetisGroup();
        bool GetisEmpty();
        int GetCount();
        DSON_Node* GetRawNode();

        static Node ParseFromTextFile(std::string IN_FileName);
        static Node ParseFromString(std::string IN_String);
        static Node CreateEmptyNode();
    };
}
