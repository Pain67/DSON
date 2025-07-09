#pragma once

#include <string>
#include <cstdint>
#include <vector>
#include "DSON.h"


namespace DSON {

    void SetLogCallbackFunction(DSON_LogFunction IN_Function);
    void SetMinLogLevel(int IN_MinLogLevel);

    class Node {
    private:

    protected:
        DSON_Node* RawNode = nullptr;
        std::vector<std::string> BasePath;
    public:
        Node();
        Node(DSON_Node* IN_RawNode);
        ~Node();

        bool AddValue(std::string IN_Key, std::string IN_Value, bool IN_isAllowOverride = false);
        bool AddValue(std::string IN_Key, int8_t IN_Value, bool IN_isAllowOverride = false);
        bool AddValue(std::string IN_Key, int16_t IN_Value, bool IN_isAllowOverride = false);
        bool AddValue(std::string IN_Key, int32_t IN_Value, bool IN_isAllowOverride = false);
        bool AddValue(std::string IN_Key, int64_t IN_Value, bool IN_isAllowOverride = false);
        bool AddValue(std::string IN_Key, uint8_t IN_Value, bool IN_isAllowOverride = false);
        bool AddValue(std::string IN_Key, uint16_t IN_Value, bool IN_isAllowOverride = false);
        bool AddValue(std::string IN_Key, uint32_t IN_Value, bool IN_isAllowOverride = false);
        bool AddValue(std::string IN_Key, uint64_t IN_Value, bool IN_isAllowOverride = false);
        bool AddValue(std::string IN_Key, float IN_Value, bool IN_isAllowOverride = false);

        bool AddEmptyGroup(std::string IN_GroupName);

        bool GetValue(std::string IN_Key, std::string& OUT_Value);
        bool GetValue(std::string IN_Key, int8_t& OUT_Value);
        bool GetValue(std::string IN_Key, int16_t& OUT_Value);
        bool GetValue(std::string IN_Key, int32_t& OUT_Value);
        bool GetValue(std::string IN_Key, int64_t& OUT_Value);
        bool GetValue(std::string IN_Key, uint8_t& OUT_Value);
        bool GetValue(std::string IN_Key, uint16_t& OUT_Value);
        bool GetValue(std::string IN_Key, uint32_t& OUT_Value);
        bool GetValue(std::string IN_Key, uint64_t& OUT_Value);
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

        std::string GetBasePath();
        void SetBasePath(std::string IN_BasePath);
        void AddBasePath(std::string IN_Path);
        void BasePathUp(int IN_Num);

        static Node ParseFromTextFile(std::string IN_FileName);
        static Node ParseFromString(std::string IN_String);
        static Node CreateEmptyNode();
    };
}
