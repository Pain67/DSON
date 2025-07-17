#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>
#include <stdarg.h>
#include <math.h>

// ------------------------------------------------------------------------------------------------
// Include guard
#ifndef DSON_HEADER
#define DSON_HEADER

// ------------------------------------------------------------------------------------------------
// Define const values

#define true 1
#define false 0
#define DSON_STRING_BUFFER_SIZE 1024
#define DSON_UINT_MAX 4294967295

#define DSON_LOG_LEVEL_DEBUG	0
#define DSON_LOG_LEVEL_INFO		1
#define DSON_LOG_LEVEL_WARNING	2
#define DSON_LOG_LEVEL_ERROR	3

#define DSON_STR_GROUP "{#GROUP}"
#define DSON_STR_EMPTY "{#EMPTY}"
#define DSON_STR_NULL "{#NULL}"

#define DSON_STR_INDENT_SIZE 2

// ------------------------------------------------------------------------------------------------
// TypeDefs

typedef struct DSON_Node {
    char* Name;
    char* Value;

    struct DSON_Node* Parent;
    struct DSON_NodeList* Childs;

    size_t Level;
} DSON_Node;

typedef struct DSON_StringList {
    char** Entries;
    int* Offsets;
    size_t Num;
} DSON_StringList;

typedef struct DSON_NodeList {
    DSON_Node** Entries;
    size_t Num;
} DSON_NodeList;

typedef enum {
    DSON_TOKEN_UNDEFINED,
    DSON_TOKEN_LITERAL,
    DSON_TOKEN_EQUAL,
    DSON_TOKEN_OPEN_B,
    DSON_TOKEN_OPEN_C,
    DSON_TOKEN_CLOSE_B,
    DSON_TOKEN_CLOSE_C,
    DSON_TOKEN_COMMENT
} DSON_Token_Type;

typedef void(*DSON_LogFunction)(int, char*);

extern DSON_LogFunction DSON_LogCallback;
extern int DSON_MinLogLevel;

// ------------------------------------------------------------------------------------------------
// Headers

// ----------------------------------------------------------------------
// Util Functions
#ifdef __cplusplus
extern "C" {
#endif

    char* DSON_CreateUniformString(char IN_Char, size_t IN_Num);
    void DSON_FreeStringList(DSON_StringList* IN_List);
    void DSON_FreeNodeList(DSON_NodeList* IN_List);
    DSON_StringList DSON_SplitString(char* IN_String, char IN_Delimiter);
    char* DSON_MergeString(DSON_StringList* IN_List, char IN_Delimiter, bool IN_isSkipFirst);
    char* DSON_CopyString(char* IN_String);
    char* DSON_MakeString(char* IN_Format, ...);
    char* DSON_ReplaceString(char* IN_From, char* IN_To, char* IN_String);
    char* DSON_RemoveSpace(char* IN_String);
    char* DSON_AddSpace(char* IN_String);
    char* DSON_IntToString(long long IN_Value);
    char* DSON_UIntToString(unsigned long long IN_Value);
    char* DSON_FloatToString(float IN_Value);
    void DSON_Log(int IN_LogLevel, char* IN_Format, va_list IN_Args);
    void DSON_LogDebug(char* IN_Format, ...);
    void DSON_LogInfo(char* IN_Format, ...);
    void DSON_LogWarning(char* IN_Format, ...);
    void DSON_LogError(char* IN_Format, ...);

    // ----------------------------------------------------------------------
    // DSON List Function

    DSON_StringList* DSON_CreateEmptyStringList();
    DSON_NodeList* DSON_CreateEmptyNodeList();
    DSON_NodeList* DSON_ResizeNodeList(DSON_NodeList* IN_List, size_t IN_NewSize);

    // ----------------------------------------------------------------------
    // DSON Node Functions

    bool DSON_Node_isValue(DSON_Node* IN_Ptr);
    bool DSON_Node_isGroup(DSON_Node* IN_Ptr);
    bool DSON_Node_isEmpty(DSON_Node* IN_Ptr);
    size_t DSON_Node_GetKeyIndex(DSON_Node* IN_Ptr, char* IN_Key);
    DSON_Node* DSON_CreateEmptyNode();
    void DSON_FreeNode(DSON_Node* IN_Node);
    void DSON_Node_Print(DSON_Node* IN_Node);
    void DSON_Node_PrintCompact(DSON_Node* IN_Node);
    void DSON_Node_PrintBinary(DSON_Node* IN_Node);
    char* DSON_Node_ToString(DSON_Node* IN_Node);
    char* DSON_Node_ToCompactString(DSON_Node* IN_Node);
    char* DSON_Node_ToBinaryString(DSON_Node* IN_Node);
    int DSON_Node_Count(DSON_Node* IN_Node);
    void DSON_Node_AddChild(DSON_Node* IN_Parent, DSON_Node* IN_Child);
    bool DSON_Node_AddValueString(DSON_Node* IN_Node, char* IN_Key, char* IN_Value, bool IN_isAllowOverride);
    bool DSON_Node_AddValueInt(DSON_Node* IN_Node, char* IN_Key, long long IN_Value, bool IN_isAllowOverride);
    bool DSON_Node_AddValueUInt(DSON_Node* IN_Node, char* IN_Key, unsigned long long IN_Value, bool IN_isAllowOverride);
    bool DSON_Node_AddValueFloat(DSON_Node* IN_Node, char* IN_Key, float IN_Value, bool IN_isAllowOverride);
    char* DSON_Node_GetValueString(DSON_Node* IN_Node, char* IN_Key);
    bool DSON_Node_GetValueInt(DSON_Node* IN_Node, char* IN_Key, long long* OUT_Value);
    bool DSON_Node_GetValueUInt(DSON_Node* IN_Node, char* IN_Key, unsigned long long* OUT_Value);
    bool DSON_Node_GetValueFloat(DSON_Node* IN_Node, char* IN_Key, float* OUT_Value);
    bool DSON_Node_Remove(DSON_Node* IN_Node, char* IN_Key);
    DSON_Node* DSON_GetSubNode(DSON_Node* IN_Node, char* IN_Key);
    size_t DSON_CountValues(DSON_Node* IN_Node);
    char* DSON_GetNodePath(DSON_Node* IN_Node);
    DSON_StringList DSON_GetKeyList(DSON_Node* IN_Node);

    // ----------------------------------------------------------------------
    // DSON Token

    DSON_Token_Type DSON_GetTokenType(char* IN_Token);

    // ----------------------------------------------------------------------
    // DSON Parser

    DSON_Node* DSON_ParseLine(DSON_Node* IN_RootNode, DSON_Node* IN_CurrNode,int IN_LineNum, char* IN_Line);
    DSON_Node* DSON_ParseTextFile(char* IN_FileName);
    DSON_Node* DSON_ParseString(char* IN_String);

    bool DSON_SaveToTextFile(DSON_Node* IN_Node, char* IN_FileName);
    bool DSON_SaveToCompactTextFile(DSON_Node* IN_Node, char* IN_FileName);
    bool DSON_SaveToBinaryFile(DSON_Node* IN_Node, char* IN_FileName);

#ifdef __cplusplus
}
#endif


// ------------------------------------------------------------------------------------------------
// Include guard
#endif // DSON_HEADER
