#pragma once

#include <string>
#include <vector>
#include <cstdint>


#define UInt uint32_t                   // 32 bit integral type
#define UChar uint8_t                   // 8 bit integral type
#define DSON_UINT_MAX 4294967295        // 2^32 - 1
#define DSON_STR_NULL   "{#NULL}"
#define DSON_STR_GROUP  "{#GROUP}"
#define DSON_STR_EMPTY  "{#EMPTY}"
#define DSON_STR_COUNT  "Count"
#define DSON_MAKE_STRING(...) DSON::MakeString(__VA_ARGS__)

class DSON_Node;

namespace DSON {
    typedef void(*DSON_Log_Callback)(std::string);
    typedef DSON_Node*(*DSON_Mem_Alloc)(void*);

    enum class LogLevel {
        UNDEFINED = 0,
        DEBUG,
        INFO,
        WARNING,
        ERROR
    };

    std::string EnumToString(LogLevel IN_Value);

    // ------------------------------------------------------
    // Log stuff
    DSON_Log_Callback LogCallback = nullptr;
    void Log_Error(std::string IN_Msg);
    void Log_Debug(std::string IN_Msg);
    // ------------------------------------------------------
    // Memory Alloc Stuff

    // Can be used for custom memory allocation
    // When a new DSON node created, this param will be passed
    // to the MemAllocCallback function
    void* MemAllocParam = nullptr;
    DSON_Node* DefaultAllocator(void* IN_Ptr);
    DSON_Mem_Alloc MemAllocCallback = &DefaultAllocator;

    // ------------------------------------------------------
    // String related functions
    std::vector<std::string> SplitString(std::string IN_Str, char IN_Delimiter);
    std::string ReplaceString(std::string IN_Str, std::string IN_From, std::string IN_To);

    template <class Type>
    std::string GetAsString(Type IN_Value) {
        if constexpr (std::is_same_v<std::string, Type>) { return IN_Value; }
        else { return std::to_string(IN_Value); }
    }

    template <class TypeA>
    std::string MakeString(std::string IN_Template, TypeA IN_A) {
        IN_Template = DSON::ReplaceString(IN_Template, "{0}", GetAsString(IN_A));
        return IN_Template;
    }

    template <class TypeA, class TypeB>
    std::string MakeString(std::string IN_Template, TypeA IN_A, TypeB IN_B) {
        IN_Template = DSON::ReplaceString(IN_Template, "{0}", GetAsString(IN_A));
        IN_Template = DSON::ReplaceString(IN_Template, "{1}", GetAsString(IN_B));
        return IN_Template;
    }
    template <class TypeA, class TypeB, class TypeC>
    std::string MakeString(std::string IN_Template, TypeA IN_A, TypeB IN_B, TypeC IN_C) {
        IN_Template = DSON::ReplaceString(IN_Template, "{0}", GetAsString(IN_A));
        IN_Template = DSON::ReplaceString(IN_Template, "{1}", GetAsString(IN_B));
        IN_Template = DSON::ReplaceString(IN_Template, "{2}", GetAsString(IN_C));
        return IN_Template;
    }
    template <class TypeA, class TypeB, class TypeC, class TypeD>
    std::string MakeString(std::string IN_Template, TypeA IN_A, TypeB IN_B, TypeC IN_C, TypeD IN_D) {
        IN_Template = DSON::ReplaceString(IN_Template, "{0}", GetAsString(IN_A));
        IN_Template = DSON::ReplaceString(IN_Template, "{1}", GetAsString(IN_B));
        IN_Template = DSON::ReplaceString(IN_Template, "{2}", GetAsString(IN_C));
        IN_Template = DSON::ReplaceString(IN_Template, "{3}", GetAsString(IN_D));
        return IN_Template;
    }
    // ------------------------------------------------------
}

// Struct To represent each kay-Value par as a Node
// Any node is either a Group or a Value - It cannot be both
// Node With nullptr as Parent is the "Root"
struct DSON_Node {
    std::string Name = "";
    std::string Value = "";

    std::vector<DSON_Node*> Childs;
    DSON_Node* ParentPtr = nullptr;

    UInt Level = 0;

    DSON_Node() {}
    ~DSON_Node();

    // Return the Index (into Childs vector) of the "Name" or UINT_MAX if no such Node Found
    // Do not support nested Names
    UInt GetIndexOf(std::string IN_Name);

    // Returna a pointer to the Node with "IN_Nname"
    // Supports nested names separated by '/'
    // Return nullptr on failure
    DSON_Node* GetNodeOf(std::string IN_Name);

    // Return the value of a Node as a string
    // on failure return false and set OUT_Value to empty string
    // Supports nested names separated by '/'
    // Return DSON_STR_GROUP if Key is a group
    // Return DSON_STR_EMPTY if key not a value nor a group (Empty)
    bool GetValueOf(std::string IN_Name, std::string& OUT_Value);

    // Extended version of GetValueOf
    // handles the conversion to arithmetic types
    // Supports nested names separated by '/'
    template <class Type>
    bool GetValueOfEx(std::string IN_Name, Type& OUT_Value) {
        static_assert(std::is_arithmetic<Type>::value, "GetValueOfEx() => Templated type must be an arithmetic type");
        std::string Temp;
        if (!GetValueOf(IN_Name, Temp)) { return false; }
        try {
            if constexpr (std::is_floating_point<Type>::value) {
                OUT_Value = std::stod(Temp);
            }
            if constexpr (std::is_integral<Type>::value) {
                OUT_Value = std::stoll(Temp);
            }
        }
        catch(...) { return false; }

        return true;
    }

    // Alternative version of GetValueOf
    // Get groups as vectors
    // Supports nested names separated by '/'
    template <class VectorType>
    bool GetValueOfArray(std::string IN_Name, std::vector<VectorType>& REF_Vector) {
        if (!GetisKeyExist(IN_Name)) { return false; }

        UInt Num = 0;
        if (!GetValueOfEx(IN_Name + "/" + DSON_STR_COUNT, Num)) { return false; }

        if (Num > 0) {
            REF_Vector.reserve(Num);
            for (UInt X = 0; X < Num; X++) {
                VectorType Temp;
                if (!GetValueOfEx(DSON_MAKE_STRING("{0}/{1}", IN_Name, X),Temp)) { return false; }
                REF_Vector.push_back(Temp);
            }
        }

        return true;
    }


    // Add a Sub Value to this Node (Create a new Child)
    // It can override exiting value if IN_isAllowOverride
    // It cannot override group to be a value
    // Supports nested names separated by '/'
    // Return False on failure (i.e. if Value is already set)
    bool AddValue(std::string IN_Name, std::string IN_Value, bool IN_isAllowOverride = false);

    // Extended version of add value
    // Add any arithmetic type (converted to string)
    // It can override exiting value if IN_isAllowOverride
    // It cannot override group to be a value
    // Supports nested names separated by '/'
    // Return False on failure (i.e. if Value is already set)
    template <class Type>
    bool AddValueEx(std::string IN_Name, Type IN_Value, bool IN_isAllowOverride = false) {
        static_assert(std::is_arithmetic<Type>::value, "AddValueEx() => Templated type must be an arithmetic type");
        std::string Temp = std::to_string(IN_Value);
        return AddValue(IN_Name, Temp, IN_isAllowOverride);
    }

    // Alternative version of add value
    // insert any vector (element by element)
    // It cannot override existing value / group
    // Supports nested names separated by '/'
    // Return False on failure (i.e. if Value is already set)
    template <class VectorType>
    bool AddValueArray(std::string IN_Name, std::vector<VectorType>& REF_Vector) {
        static_assert(
            std::is_arithmetic<VectorType>::value ||
            std::is_same<std::string, VectorType>::value,
            "AddValueArray() => Currently only 'string' or 'arithmetic' type supported as VectorType"
        );

        if (!AddEmptyGroup(IN_Name)) { return false; }
        UInt Num = REF_Vector.size();
        if (!AddValueEx(DSON_MAKE_STRING("{0}/{1}/",IN_Name, DSON_STR_COUNT), Num)) { return false; }

        if (Num > 0) {
            std::string Base = DSON_MAKE_STRING("{0}/{1}/",IN_Name, DSON_STR_COUNT);
            if constexpr (std::is_same<std::string, VectorType>::value) {
                for (UInt X = 0; X < Num; X++) {
                    if (!AddValue(Base + std::to_string(X),REF_Vector[X],false)) { return false; }
                }
            }
            else {
                for (UInt X = 0; X < Num; X++) {
                    if (!AddValueEx(Base + std::to_string(X),REF_Vector[X],false)) { return false; }
                }
            }
        }

        return true;
    }

    // Add a new Emtpy subgroup to this node
    // Supports nested names separated by '/'
    // Return False on failure
    bool AddEmptyGroup(std::string IN_Name);

    // Removes the specified element
    // Can be used on both values and groups
    // If element is a group all of its childres will
    // be removed as well
    // Supports nested names separated by '/'
    bool RemoveElement(std::string IN_Name);

    // Check whether the give key exist or not
    // Supports nested names separated by '/'
    bool GetisKeyExist(std::string IN_Name);

    // Return true if Value is not empty, false otherwise
    bool isValue();

    // Return true if Childs has more than 0 element, false otherwise
    bool isGroup();

    // Return true if both isValue and isGroup are false, false otherwise
    bool isEmpty();

    // It may be surprising but this function is turning this node into a string :O
    std::string ToString();
};

// Create a class to warp around a Node
// It is not really doing anything, only exist to make sure
// When Node goes out of scope, it will be deleted properly
class DSON_Tree {
private:
    DSON_Node* Root = nullptr;
protected:

public:
    DSON_Tree() { Root = new DSON_Node(); Root->Name = "Root"; }
    DSON_Tree(DSON_Node* IN_Ptr) { Root = IN_Ptr; }
    ~DSON_Tree() { if (Root != nullptr) { delete Root; } }

    operator DSON_Node*() { return Root; }

    // --------------------------------------------------------------------------------------------------------------------------------
    // DSON_Tree exposes all function from DSON_Node

    UInt GetIndexOf(std::string IN_Name);
    DSON_Node* GetNodeOf(std::string IN_Name);

    bool GetValueOf(std::string IN_Name, std::string& OUT_Value);

    template <class Type>
    bool GetValueOfEx(std::string IN_Name, Type& OUT_Value) {
        if (Root == nullptr) {
            DSON::Log_Error("GetValueOfEx() faile, DSON_Tree => RootNode is NULLPTR");
            return false;
        }
        return Root->GetValueOfEx(IN_Name, OUT_Value);
    }

    template <class VectorType>
    bool GetValueOfArray(std::string IN_Name, std::vector<VectorType>& OUT_Value) {
        if (Root == nullptr) {
            DSON::Log_Error("GetValueOfArray() faile, DSON_Tree => RootNode is NULLPTR");
            return false;
        }
        return Root->GetValueOfArray(IN_Name, OUT_Value);
    }


    bool AddValue(std::string IN_Name, std::string IN_Value, bool IN_isAllowOverride = false);

    template<class Type>
    bool AddValueEx(std::string IN_Name, Type IN_Value, bool IN_isAllowOverride = false) {
        if (Root == nullptr) {
            DSON::Log_Error("AddValueEx() faile, DSON_Tree => RootNode is NULLPTR");
            return false;
        }
        return Root->AddValueEx(IN_Name, IN_Value, IN_isAllowOverride);
    }

    template <class VectorType>
    bool AddValueArray(std::string IN_Name, std::vector<VectorType>& REF_Value) {
        if (Root == nullptr) {
            DSON::Log_Error("AddValueArray() faile, DSON_Tree => RootNode is NULLPTR");
            return false;
        }
        return Root->AddValueArray(IN_Name, REF_Value);
    }

    bool AddEmptyGroup(std::string IN_Name);

    bool RemoveElement(std::string IN_Name);
    bool GetisKeyExist(std::string IN_Name);

    // --------------------------------------------------------------------------------------------------------------------------------

    DSON_Node* GetRoot() { return Root; }
};

class DSON_Parser {
private:

protected:

    enum class DSON_TokeType {
        UNDEFINED,
        LITERAL,
        EQUAL,
        OPEN_B, OPEN_C,
        CLOSE_B, CLOSE_C
    };

    DSON_Node* RootNode = nullptr;
    DSON_Node* CurrNode = RootNode;

    DSON_TokeType _Internal_GetTokenType(std::string IN_Token);
    bool _Internal_ParseLine(std::string IN_Line, UInt IN_LineNum);
    void _Internal_Init();

public:

    DSON_Node* ParseFile(std::string IN_FileName);
    DSON_Node* ParseString(std::string IN_Str);

};



