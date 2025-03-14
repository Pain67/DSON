#include <string>
#include <vector>
#include <cstdint>


#define UInt uint32_t                   //
#define UChar uint8_t                   //
#define DSON_UINT_MAX 4294967295        // 2^32 - 1
#define DSON_MAKE_STRING(...) DSON::MakeString(__VA_ARGS__)

namespace DSON {
    typedef void(*DSON_Log_Callback)(std::string);

    namespace {
        DSON_Log_Callback LogCallback = nullptr;
    };

    void Log_Error(std::string IN_Msg);
    void Log_Debug(std::string IN_Msg);

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

    // Return a value of the Node with "IN_Name"
    // Supports nested names separated by '/'
    // Return "{#NULL}" on failure
	std::string GetValueOf(std::string IN_Name);

    // Returna a pointer to the Node with "IN_Nname"
    // Supports nested names separated by '/'
    // Return nullptr on failure
	DSON_Node* GetNodeOf(std::string IN_Name);

    // Similar to GetValueOf() but removes the need to return "{#NULL}"
    // on failure return false and set OUT_Value to empty string
    // Supports nested names separated by '/'
	bool CheckAndGetValueOf(std::string IN_Name, std::string& OUT_Value);

    // Add a Sub Value to this Node (Create a new Child)
    // It can override exiting value if IN_isAllowOverride
    // It cannot override group to be a value
    // Supports nested names separated by '/'
    // Return False on failure (i.e. if Value is already set)
	bool AddValue(std::string IN_Name, std::string IN_Value, bool IN_isAllowOverride = false);

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
    // Noode this tree represent
    // deleted in destructor
    // Never null, if constructed with no argument
    // A new Node will be created and names "Root"
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
	std::string GetValueOf(std::string IN_Name);
	DSON_Node* GetNodeOf(std::string IN_Name);

	bool CheckAndGetValueOf(std::string IN_Name, std::string& OUT_Value);
	bool AddValue(std::string IN_Name, std::string IN_Value, bool IN_isAllowOverride = false);
	bool AddEmptyGroup(std::string IN_Name);

    bool RemoveElement(std::string IN_Name);

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



