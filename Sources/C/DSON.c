#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>
#include <stdarg.h>
#include <math.h>

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

#define DSON_IMPLEMENTATION
#define DSON_DEVELOPMENT

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


// ------------------------------------------------------------------------------------------------
// Globals

int DSON_LogLevel = DSON_LOG_LEVEL_WARNING;

// ------------------------------------------------------------------------------------------------
// Headers

// ----------------------------------------------------------------------
// Util Functions
#ifdef cplusplus
	extern "C" {
#endif

char* DSON_CreateUniformString(char IN_Char, size_t IN_Num);
void DSON_FreeStringList(DSON_StringList* IN_List);
void DSON_FreeNodeList(DSON_NodeList* IN_List);
DSON_StringList DSON_SplitString(char* IN_String, char IN_Delimiter);
char* DSON_MergeString(DSON_StringList* IN_List, char IN_Delimiter, bool IN_isSkipFirst);
char* DSON_CopyString(char* IN_String);
char* DSON_MakeString(char* IN_Format, ...);
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
void DSON_Node_Print(DSON_Node* IN_Node);
char* DSON_Node_ToString(DSON_Node* IN_Node);
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

#ifdef cplusplus
	}
#endif

// End Headers
// ------------------------------------------------------------------------------------------------

// ------------------------------------------------------------------------------------------------
// Implementation

#ifdef DSON_IMPLEMENTATION

// ----------------------------------------------------------------------
// Util Functions

#ifdef cplusplus
	extern "C" {
#endif

char* DSON_CreateUniformString(char IN_Char, size_t IN_Num) {
    // +1 for the terminateing 0
    char* Result = malloc(sizeof(char) * (IN_Num + 1));

    for (size_t X = 0; X < IN_Num; X++) { Result[X] = IN_Char; }
    Result[IN_Num] = 0;

    return Result;
}

void DSON_FreeStringList(DSON_StringList* IN_List) {
	if (IN_List->Num == 0) { return; }

	for (size_t X = 0; X < IN_List->Num; X++) {
		free(IN_List->Entries[X]);
	}
	free (IN_List->Offsets);
	free(IN_List->Entries);
}

void DSON_FreeNodeList(DSON_NodeList* IN_List) {
    if (IN_List->Num == 0) { return; }

    for (size_t X = 0; X < IN_List->Num; X++) {
        free(IN_List->Entries[X]);
    }

    free(IN_List->Entries);
}

DSON_StringList DSON_SplitString(char* IN_String, char IN_Delimiter) {
	DSON_StringList Result;
	Result.Num = 0;
	Result.Entries = NULL;
    size_t Len = strlen(IN_String);
	int CharNum = 0;
    char* Buffer = 0;
    size_t BufferIndex = 0;
    size_t ResultIndex = 0;
	// Starts with true, so if first char is a delimiter it will just be ignored
	bool isLastDelimiter = true;

    // Count the number of Tokens
    for (size_t X = 0; X < Len; X++) {
		if (IN_String[X] == IN_Delimiter) {
			if (isLastDelimiter == false) { Result.Num++; }
			isLastDelimiter = true;
		}
		else { isLastDelimiter = false; }
	}

    //  +1 as we need one more token than number of delimiters
    Result.Num ++;

    // Allocate Space for Result Array
    Result.Entries = malloc(sizeof(char*) * Result.Num);
	Result.Offsets = malloc(sizeof(int) * Result.Num);

    // Allocate Space for Buffer
    Buffer = malloc(sizeof(char) * DSON_STRING_BUFFER_SIZE);
	memset(Buffer, 0, DSON_STRING_BUFFER_SIZE);

    // Building Result Array
    for (size_t X = 0; X < Len; X++) {
		if (IN_String[X] == IN_Delimiter) {
			if (BufferIndex > 0) {
				// Keep as buffer index so we have room to a terminateing 0
				Result.Entries[ResultIndex] = malloc(sizeof(char) * BufferIndex);
				strcpy(Result.Entries[ResultIndex], Buffer);
				Result.Offsets[ResultIndex] = CharNum;
				ResultIndex++;
				BufferIndex = 0;
				memset(Buffer, 0, DSON_STRING_BUFFER_SIZE);
			}
		}
		else {
			Buffer[BufferIndex] = IN_String[X];
			BufferIndex++;
		}
		CharNum++;
	}

    // Add last Element if Exist
    if (BufferIndex > 0) {
        // Keep as buffer index so we have room to a terminateing 0
        Result.Entries[ResultIndex] = malloc(sizeof(char) * BufferIndex);
        strcpy(Result.Entries[ResultIndex], Buffer);
		Result.Offsets[ResultIndex] = CharNum;
        ResultIndex++;
    }

    free(Buffer);

    // All Done :D
    return Result;
}

char* DSON_MergeString(DSON_StringList* IN_List, char IN_Delimiter, bool IN_isSkipFirst) {
	if (IN_List == NULL) { return NULL; }
	if (IN_List->Num == 0) { return NULL; }

	char* Buffer = 0;
	char* Result = 0;
	size_t BufferIndex = 0;

	Buffer = malloc(sizeof(char) * DSON_STRING_BUFFER_SIZE);
	memset(Buffer,0,DSON_STRING_BUFFER_SIZE);

	size_t From = 0;
	if (IN_isSkipFirst == true) { From = 1; }

	for (size_t X = From; X < IN_List->Num; X++) {
		if (X > From) {
			Buffer[BufferIndex] = IN_Delimiter;
			BufferIndex++;
		}
		size_t TokenLen = strlen(IN_List->Entries[X]);
		for (size_t Y = 0; Y < TokenLen; Y++) {
			Buffer[BufferIndex + Y] = IN_List->Entries[X][Y];
		}
		BufferIndex += TokenLen;
	}

	Result = malloc(sizeof(char) * BufferIndex);
	strcpy(Result, Buffer);
	free(Buffer);

	return Result;
}
char* DSON_CopyString(char* IN_String) {
	if (IN_String == NULL) { return NULL; }
	size_t Len = strlen(IN_String);

	if (Len == 0) {
		char* Result = malloc(sizeof(char));
		Result[0] = 0;
		return Result;
	}
	else {
		char* Result = malloc(sizeof(char) * (Len + 1));
		strcpy(Result, IN_String);
		return Result;
	}
}
char* DSON_MakeString(char* IN_Format, ...) {
	char* TempBuffer = malloc(sizeof(char) * DSON_STRING_BUFFER_SIZE);
	memset(TempBuffer, 0, DSON_STRING_BUFFER_SIZE);

	va_list Args;
	va_start(Args, IN_Format);
		size_t Size = vsprintf(TempBuffer, IN_Format, Args);
	va_end(Args);

	// For the terminateing 0;
	Size++;

	char* Result = malloc(sizeof(char) * Size);
	memset(Result, 0, Size);

	strcpy(Result, TempBuffer);
	free(TempBuffer);

	return Result;
}


char* DSON_IntToString(long long IN_Value) {
	char* Buffer = malloc(sizeof(char) * DSON_STRING_BUFFER_SIZE);
	char* Result = NULL;

	size_t CharNum = sprintf(Buffer,"%lld", IN_Value);

	// +1 as charnum returned from sprintf not include the terminating 0
	Result = malloc(sizeof(char) * (CharNum + 1));

	strcpy(Result, Buffer);
	free(Buffer);

	return Result;
}
char* DSON_UIntToString(unsigned long long IN_Value) {
	char* Buffer = malloc(sizeof(char) * DSON_STRING_BUFFER_SIZE);
	char* Result = NULL;

	size_t CharNum = sprintf(Buffer,"%llu", IN_Value);

	// +1 as charnum returned from sprintf not include the terminating 0
	Result = malloc(sizeof(char) * (CharNum + 1));

	strcpy(Result, Buffer);
	free(Buffer);

	return Result;
}
char* DSON_FloatToString(float IN_Value) {
	char* Buffer = malloc(sizeof(char) * DSON_STRING_BUFFER_SIZE);
	char* Result = NULL;

	size_t CharNum = sprintf(Buffer,"%f", IN_Value);

	// +1 as charnum returned from sprintf not include the terminating 0
	Result = malloc(sizeof(char) * (CharNum + 1));

	strcpy(Result, Buffer);
	free(Buffer);

	return Result;
}


void DSON_Log(int IN_LogLevel, char* IN_Format, va_list IN_Args) {
	char* Buffer = malloc(sizeof(char) * 1024);
	char* LogType = malloc(sizeof(char) * 10);
	switch(IN_LogLevel) {
		case 0: strcpy(LogType, "DEBUG"); break;
		case 1: strcpy(LogType, "INFO"); break;
		case 2: strcpy(LogType, "WARNING"); break;
		case 3: strcpy(LogType, "ERROR"); break;
		default: strcpy(LogType, "UNDEFINED"); break;
	}

	sprintf(Buffer, "[%s] => %s\n", LogType, IN_Format);
	vprintf(Buffer, IN_Args);
	fflush(stdout);

	free(Buffer);
}

void DSON_LogDebug(char* IN_Format, ...) {
	if (DSON_LogLevel > DSON_LOG_LEVEL_DEBUG) { return; }
	va_list args;
	va_start(args, IN_Format);
	DSON_Log(DSON_LOG_LEVEL_DEBUG, IN_Format, args);
	va_end(args);
}
void DSON_LogInfo(char* IN_Format, ...) {
	if (DSON_LogLevel > DSON_LOG_LEVEL_INFO) { return; }
	va_list args;
	va_start(args, IN_Format);
	DSON_Log(DSON_LOG_LEVEL_INFO, IN_Format, args);
	va_end(args);
}
void DSON_LogWarning(char* IN_Format, ...) {
	if (DSON_LogLevel > DSON_LOG_LEVEL_WARNING) { return; }
	va_list args;
	va_start(args, IN_Format);
	DSON_Log(DSON_LOG_LEVEL_WARNING, IN_Format, args);
	va_end(args);
}
void DSON_LogError(char* IN_Format, ...) {
	va_list args;
	va_start(args, IN_Format);
	DSON_Log(DSON_LOG_LEVEL_ERROR, IN_Format, args);
	va_end(args);
}

// ----------------------------------------------------------------------
// DSON List Function

DSON_StringList* DSON_CreateEmptyStringList() {
    DSON_StringList* NewList = malloc(sizeof(DSON_StringList));

    NewList->Entries = NULL;
    NewList->Num = 0;

    return NewList;
}

DSON_NodeList* DSON_CreateEmptyNodeList() {
    DSON_NodeList* NewList = malloc(sizeof(DSON_NodeList));

    NewList->Entries = NULL;
    NewList->Num = 0;

    return NewList;
}

DSON_NodeList* DSON_ResizeNodeList(DSON_NodeList* IN_List, size_t IN_NewSize) {
	if (IN_NewSize == IN_List->Num) { return IN_List; }

	// Create New List
	DSON_NodeList* NewList = DSON_CreateEmptyNodeList();

	// Allocate Element
	NewList->Entries = malloc(sizeof(DSON_Node*) * IN_NewSize);
	NewList->Num = IN_NewSize;

	if (IN_List->Num < IN_NewSize) {
		if (IN_List->Num > 0) {
			for (size_t X = 0; X < IN_List->Num; X++) { NewList->Entries[X] = IN_List->Entries[X]; }
		}
		if (IN_List->Num < IN_NewSize) {
			for (size_t X = IN_List->Num; X < IN_NewSize; X++) { NewList->Entries[X] = NULL; }
		}
	}
	else {
		for (size_t X = 0; X < IN_NewSize; X++) { NewList->Entries[X] = IN_List->Entries[X]; }
	}

	// Deallocate Old List - Dont Use DSON_FreeNodeList as that also deallocates Elements
	free(IN_List);

	return NewList;
}



// ----------------------------------------------------------------------
// DSON Node Functions

bool DSON_Node_isValue(DSON_Node* IN_Ptr) {
    if (IN_Ptr->Value == NULL) { return false; }
    else { return true; }
}
bool DSON_Node_isGroup(DSON_Node* IN_Ptr) {
    if (IN_Ptr->Childs->Num > 0) { return true; }
    else { return false; }
}
bool DSON_Node_isEmpty(DSON_Node* IN_Ptr) {
    if (IN_Ptr->Value == NULL && IN_Ptr->Childs->Num == 0) { return true; }
    else { return false; }
}
size_t DSON_Node_GetKeyIndex(DSON_Node* IN_Ptr, char* IN_Key) {
	if (IN_Ptr == NULL) { return DSON_UINT_MAX; }
	if (IN_Ptr->Childs->Num == 0) { return DSON_UINT_MAX; }

	for (size_t X = 0; X < IN_Ptr->Childs->Num; X++) {
		if (strcmp(IN_Ptr->Childs->Entries[X]->Name, IN_Key) == 0) { return X; }
	}

	return DSON_UINT_MAX;
}

DSON_Node* DSON_CreateEmptyNode() {
    DSON_Node* NewNode = malloc(sizeof(DSON_Node));

    NewNode->Name = NULL;
    NewNode->Value = NULL;
    NewNode->Parent = NULL;
    NewNode->Childs = DSON_CreateEmptyNodeList();

    NewNode->Level = 0;

    return NewNode;
}

void DSON_Node_Print(DSON_Node* IN_Node) {
	if (IN_Node == NULL) { return; }

	char* Indent = DSON_CreateUniformString(' ', (IN_Node->Level * DSON_STR_INDENT_SIZE));

	if (DSON_Node_isValue(IN_Node)) {
		printf("%s%s = [ %s ]\n", Indent, IN_Node->Name, IN_Node->Value);
		return;
	}
	else if (DSON_Node_isGroup(IN_Node)) {
		printf("%s%s = {\n", Indent, IN_Node->Name);
		for (size_t X = 0; X < IN_Node->Childs->Num; X++) {
			DSON_Node_Print(IN_Node->Childs->Entries[X]);
		}
		printf("%s}\n", Indent);
		return;
	}
	else if (DSON_Node_isEmpty(IN_Node)) {
		printf("%s%s = #EMPTY\n", Indent, IN_Node->Name);
		return;
	}
	else {
		// UNREACHABLE
	}
}
char* DSON_Node_ToString(DSON_Node* IN_Node) {
	if (IN_Node == NULL) { return NULL; }
	char* Indent = DSON_CreateUniformString(' ', (IN_Node->Level * DSON_STR_INDENT_SIZE));

	if (DSON_Node_isValue(IN_Node)) {
		return DSON_MakeString("%s%s = [ %s ]\n", Indent, IN_Node->Name, IN_Node->Value);
	}
	else if (DSON_Node_isGroup(IN_Node)) {
		char* Result = DSON_MakeString("%s%s = {\n", Indent, IN_Node->Name);
		for (size_t X = 0; X < IN_Node->Childs->Num; X++) {
			char* ChildString = DSON_Node_ToString(IN_Node->Childs->Entries[X]);
			char* TempSting = DSON_MakeString("%s%s", Result, ChildString);
			free(Result);
			free(ChildString);
			Result = TempSting;
		}
		char* CloseTag = DSON_MakeString("%s\n%s}\n", Result, Indent);
		free(Result);

		return CloseTag;
	}
	else if (DSON_Node_isEmpty(IN_Node)) {
		return DSON_MakeString("%s%s = #EMPTY\n", Indent, IN_Node->Name);
	}
	else {
		// UNREACHABLE
		return NULL;
	}
}
char* DSON_Node_ToCompactString(DSON_Node* IN_Node) {
	if (IN_Node == NULL) { return NULL; }

	if (DSON_Node_isValue(IN_Node)) {
		return DSON_MakeString("%s = [ %s ] ", IN_Node->Name, IN_Node->Value);
	}
	else if (DSON_Node_isGroup(IN_Node)) {
		char* Result = DSON_MakeString("%s = { ", IN_Node->Name);
		for (size_t X = 0; X < IN_Node->Childs->Num; X++) {
			char* ChildString = DSON_Node_ToCompactString(IN_Node->Childs->Entries[X]);
			char* TempSting = DSON_MakeString("%s%s", Result, ChildString);
			free(Result);
			free(ChildString);
			Result = TempSting;
		}
		char* CloseTag = DSON_MakeString("%s } ", Result);
		free(Result);

		return CloseTag;
	}
	else if (DSON_Node_isEmpty(IN_Node)) {
		return DSON_MakeString("%s = #EMPTY ", IN_Node->Name);
	}
	else {
		// UNREACHABLE
		return NULL;
	}
}
void DSON_Node_AddChild(DSON_Node* IN_Parent, DSON_Node* IN_Child) {
	DSON_LogDebug("Adding [%s] as a child to [%s]", IN_Child->Name, IN_Parent->Name);
	size_t Index = IN_Parent->Childs->Num;
	IN_Parent->Childs = DSON_ResizeNodeList(IN_Parent->Childs, IN_Parent->Childs->Num + 1);

	IN_Parent->Childs->Entries[Index] = IN_Child;
	IN_Child->Parent = IN_Parent;
	IN_Child->Level = IN_Parent->Level + 1;
}

bool DSON_Node_AddValueString(DSON_Node* IN_Node, char* IN_Key, char* IN_Value, bool IN_isAllowOverride) {
	if (DSON_Node_isValue(IN_Node)) {
		DSON_LogError("Unable to add Value to NULL");
		return false;
	}

	bool Result = false;
	DSON_StringList List = DSON_SplitString(IN_Key, '/');

	if (List.Num == 1) {
		size_t Index = DSON_Node_GetKeyIndex(IN_Node, IN_Key);
		if (Index < DSON_UINT_MAX) {
			if (IN_isAllowOverride == true) {
				if (DSON_Node_isGroup(IN_Node->Childs->Entries[Index]) == false) {
					char* Temp = IN_Node->Childs->Entries[Index]->Value;
					IN_Node->Childs->Entries[Index]->Value = malloc(sizeof(char) * (strlen(IN_Value) + 1));
					strcpy(IN_Node->Childs->Entries[Index]->Value, IN_Value);
					Result =  true;
					DSON_LogWarning(
						"Override value of [%s] from [%s] to [%s] in Node [%s]",
						IN_Key,
						Temp,
						IN_Value,
						IN_Node->Name
					);
					free(Temp);
					goto CleanUp;
				}
			}
			DSON_LogError(
				"Unable to add StringValue to [%s]. Key [%s] already exist",
				IN_Node->Name,
				IN_Key
			);
			Result = false;
			goto CleanUp;
		}

		DSON_Node* NewNode = DSON_CreateEmptyNode();
		DSON_Node_AddChild(IN_Node, NewNode);

		// +1 as length from strlen will not include the null termination char
		NewNode->Name = malloc(sizeof(char) * (strlen(IN_Key) + 1));
		strcpy(NewNode->Name, IN_Key);

		// +1 as length from strlen will not include the null termination char
		NewNode->Value = malloc(sizeof(char) * (strlen(IN_Value) + 1));
		strcpy(NewNode->Value, IN_Value);
		Result = true;
	}
	else {
		size_t Index = DSON_Node_GetKeyIndex(IN_Node, List.Entries[0]);
		if (Index < DSON_UINT_MAX) {
			if (DSON_Node_isValue(IN_Node->Childs->Entries[Index]) == true) {
				DSON_LogError(
					"Unable to add StringValue to [%s]. Key [%s] (from Key Chain [%s]) is not a group",
					IN_Node->Name,
					List.Entries[0],
					IN_Key
				);
				Result = false;
				goto CleanUp;
			}
			if (IN_isAllowOverride) {
				char* NewKey = DSON_MergeString(&List, '/', true);
				Result = DSON_Node_AddValueString(IN_Node->Childs->Entries[Index], NewKey, IN_Value, IN_isAllowOverride);
				free(NewKey);
				goto CleanUp;
			}
			DSON_LogError(
				"Unable to add StringValue to [%s]. Key [%s] (from Key Chain [%s]) already exist",
				IN_Node->Name,
				List.Entries[0],
				IN_Key
			);
			Result = false;
			goto CleanUp;
		}

		DSON_Node* NewNode = DSON_CreateEmptyNode();
		DSON_Node_AddChild(IN_Node, NewNode);
		// +1 as length from strlen will not include the null termination char
		NewNode->Name = malloc(sizeof(char) * (strlen(List.Entries[0]) + 1));
		strcpy(NewNode->Name, List.Entries[0]);

		char* NewKey = DSON_MergeString(&List, '/', true);
		Result = DSON_Node_AddValueString(NewNode, NewKey, IN_Value, IN_isAllowOverride);
		free(NewKey);
	}

	CleanUp:
	DSON_FreeStringList(&List);
	return Result;
}

bool DSON_Node_AddValueInt(DSON_Node* IN_Node, char* IN_Key, long long IN_Value, bool IN_isAllowOverride) {
	char* TempValue = DSON_IntToString(IN_Value);
	bool Result = false;
	Result = DSON_Node_AddValueString(IN_Node, IN_Key, TempValue, IN_isAllowOverride);
	free(TempValue);
	return Result;
}
bool DSON_Node_AddValueUInt(DSON_Node* IN_Node, char* IN_Key, unsigned long long IN_Value, bool IN_isAllowOverride) {
	char* TempValue = DSON_IntToString(IN_Value);
	bool Result = false;
	Result = DSON_Node_AddValueString(IN_Node, IN_Key, TempValue, IN_isAllowOverride);
	free(TempValue);
	return Result;
}
bool DSON_Node_AddValueFloat(DSON_Node* IN_Node, char* IN_Key, float IN_Value, bool IN_isAllowOverride) {
	char* TempValue = DSON_FloatToString(IN_Value);
	bool Result = false;
	Result = DSON_Node_AddValueString(IN_Node, IN_Key, TempValue, IN_isAllowOverride);
	free(TempValue);
	return Result;
}

char* DSON_Node_GetValueString(DSON_Node* IN_Node, char* IN_Key) {
	if (IN_Node == NULL) {
		DSON_LogError("Unable to Get value from a NULL node");
		return NULL;
	}
	if (IN_Key == 0) {
		DSON_LogError("Unable to Get value where Key is NULL");
		return NULL;
	}
	if (strlen(IN_Key) == 0) {
		DSON_LogError("Unable to Get value for an empty key");
		return NULL;
	}

	DSON_StringList List = DSON_SplitString(IN_Key, '/');

	size_t Index = DSON_Node_GetKeyIndex(IN_Node, List.Entries[0]);
	if (Index == DSON_UINT_MAX) {
		DSON_LogError(
			"Unable to get String Value. Key [%s] not found in node [%s]",
			List.Entries[0],
			IN_Node->Name
		);
		return NULL;
	}
	DSON_Node* Target = IN_Node->Childs->Entries[Index];

	if (List.Num == 1) {
		if (DSON_Node_isValue(Target)) {
			return DSON_CopyString(Target->Value);
		}
		if (DSON_Node_isGroup(Target)) {
			return DSON_CopyString(DSON_STR_GROUP);
		}
		if (DSON_Node_isEmpty(Target)) {
			return DSON_CopyString(DSON_STR_EMPTY);
		}
		// UNREACHABLE
		return DSON_CopyString(DSON_STR_NULL);
	}
	else {
		char* NewKey = DSON_MergeString(&List,'/',true);
		return DSON_Node_GetValueString(Target, NewKey);
	}

	// UNREACHABLE
	return NULL;
}
bool DSON_Node_GetValueInt(DSON_Node* IN_Node, char* IN_Key, long long* OUT_Value) {
	char* String = DSON_Node_GetValueString(IN_Node, IN_Key);
	if (String == NULL) { return false; }

	if (strcmp(String, DSON_STR_GROUP) == 0) {
		DSON_LogError(
			"Unable to get [%s] from Node [%s].[%s] cannot be converted to Int",
			IN_Key,
			IN_Node->Name,
			DSON_STR_GROUP
		);
		return false;
	}
	if (strcmp(String, DSON_STR_EMPTY) == 0) {
		DSON_LogError(
			"Unable to get [%s] from Node [%s].[%s] cannot be converted to Int",
			IN_Key,
			IN_Node->Name,
			DSON_STR_EMPTY
		);
		return false;
	}
	if (strcmp(String, DSON_STR_NULL) == 0) {
		DSON_LogError(
			"Unable to get [%s] from Node [%s].[%s] cannot be converted to Int",
			IN_Key,
			IN_Node->Name,
			DSON_STR_NULL
		);
		return false;
	}

	(*OUT_Value) = atoi(String);
	free(String);
	return true;
}
bool DSON_Node_GetValueUInt(DSON_Node* IN_Node, char* IN_Key, unsigned long long* OUT_Value) {
	char* String = DSON_Node_GetValueString(IN_Node, IN_Key);
	if (String == NULL) { return false; }

	if (strcmp(String, DSON_STR_GROUP) == 0) {
		DSON_LogError(
			"Unable to get [%s] from Node [%s].[%s] cannot be converted to Unsigned Int",
			IN_Key,
			IN_Node->Name,
			DSON_STR_GROUP
		);
		return false;
	}
	if (strcmp(String, DSON_STR_EMPTY) == 0) {
		DSON_LogError(
			"Unable to get [%s] from Node [%s].[%s] cannot be converted to Unsigned Int",
			IN_Key,
			IN_Node->Name,
			DSON_STR_EMPTY
		);
		return false;
	}
	if (strcmp(String, DSON_STR_NULL) == 0) {
		DSON_LogError(
			"Unable to get [%s] from Node [%s].[%s] cannot be converted to Unsigned Int",
			IN_Key,
			IN_Node->Name,
			DSON_STR_NULL
		);
		return false;
	}

	(*OUT_Value) = strtoull(String, NULL, 10);
	free(String);
	return true;
}
bool DSON_Node_GetValueFloat(DSON_Node* IN_Node, char* IN_Key, float* OUT_Value) {
	char* String = DSON_Node_GetValueString(IN_Node, IN_Key);
	if (String == NULL) { return false; }

	if (strcmp(String, DSON_STR_GROUP) == 0) {
		DSON_LogError(
			"Unable to get [%s] from Node [%s].[%s] cannot be converted to Float",
			IN_Key,
			IN_Node->Name,
			DSON_STR_GROUP
		);
		return false;
	}
	if (strcmp(String, DSON_STR_EMPTY) == 0) {
		DSON_LogError(
			"Unable to get [%s] from Node [%s].[%s] cannot be converted to Float",
			IN_Key,
			IN_Node->Name,
			DSON_STR_EMPTY
		);
		return false;
	}
	if (strcmp(String, DSON_STR_NULL) == 0) {
		DSON_LogError(
			"Unable to get [%s] from Node [%s].[%s] cannot be converted to Float",
			IN_Key,
			IN_Node->Name,
			DSON_STR_NULL
		);
		return false;
	}

	double Temp = strtod(String, NULL);
	(*OUT_Value) = Temp;
	free(String);
	return true;
}

bool DSON_Node_Remove(DSON_Node* IN_Node, char* IN_Key) {
	size_t Index = DSON_Node_GetKeyIndex(IN_Node, IN_Key);
	if (Index == DSON_UINT_MAX) {
		DSON_LogError(
			"Unable to remove Key [%s] from Node [%s]. Key not found",
			IN_Key,
			IN_Node->Name
		);
		return false;
	}

	DSON_Node* ToRemove = IN_Node->Childs->Entries[Index];

	for (size_t X = Index; X < (IN_Node->Childs->Num - 1); X++) {
		IN_Node->Childs->Entries[X] = IN_Node->Childs->Entries[X+1];
	}
	IN_Node->Childs = DSON_ResizeNodeList(IN_Node->Childs, IN_Node->Childs->Num - 1);

	DSON_FreeNodeList(ToRemove->Childs);
	free(ToRemove);

	DSON_LogInfo(
		"Key [%s] removed from Node [%s]",
		IN_Key,
		IN_Node->Name
	);

	return true;
}

// ----------------------------------------------------------------------
// DSON Token

DSON_Token_Type DSON_GetTokenType(char* IN_Token) {
	if (IN_Token == NULL) { return DSON_TOKEN_UNDEFINED; }
	size_t Len = strlen(IN_Token);

	if (Len == 0) { return DSON_TOKEN_UNDEFINED; }

	if (Len == 1) {
		if (IN_Token[0] == '=') { return DSON_TOKEN_EQUAL; }
		if (IN_Token[0] == '[') { return DSON_TOKEN_OPEN_B; }
		if (IN_Token[0] == ']') { return DSON_TOKEN_CLOSE_B; }
		if (IN_Token[0] == '{') { return DSON_TOKEN_OPEN_C; }
		if (IN_Token[0] == '}') { return DSON_TOKEN_CLOSE_C; }
		if (IN_Token[0] == '#') { return DSON_TOKEN_COMMENT; }
	}

	return DSON_TOKEN_LITERAL;
}

// ----------------------------------------------------------------------
// DSON Parser

DSON_Node* DSON_ParseLine(DSON_Node* IN_RootNode, DSON_Node* IN_CurrNode,int IN_LineNum, char* IN_Line) {
	// State Expection list:
	//	0 - Literal Name (Also handles '}')
	//	1 - '='
	//	2 - '[' or '{'
	//	3 - Literal Value
	//	4 - ']'
	//	5 - '#' Comment line, rest of the line is ignored
	unsigned char State = 0;

	if (IN_Line == NULL) { return IN_CurrNode; }	// Skipp Null Line
	if (IN_Line[0] == 0) { return IN_CurrNode; }	// Skip Empty Line
	if (IN_Line[0] == '\n') { return IN_CurrNode; }	// Skip Empty Line
	if (IN_Line[0] == '#') { return IN_CurrNode; }	// Skip Comment Line

	DSON_StringList Tokens = DSON_SplitString(IN_Line, ' ');
	for (size_t X = 0; X < Tokens.Num; X++) {
		DSON_Token_Type Type = DSON_GetTokenType(Tokens.Entries[X]);

		if (Type == DSON_TOKEN_COMMENT) {
			// ignore rest of the line == break out of for loop
			break;
		}

		if (State == 0) {
			if (Type == DSON_TOKEN_CLOSE_C && IN_CurrNode != IN_RootNode) {
				if (IN_CurrNode->Parent != NULL) {
					IN_CurrNode = IN_CurrNode->Parent;
					continue;
				}
			}
			if (Type != DSON_TOKEN_LITERAL) {
				DSON_LogError(
					"Unexpected '%s' at line [%i:%i]. Expected a literal name.",
					Tokens.Entries[X],
					IN_LineNum,
					Tokens.Offsets[X]
				);
				return NULL;
			}

			DSON_Node* NewNode = DSON_CreateEmptyNode();
			NewNode->Name = Tokens.Entries[X];
			DSON_Node_AddChild(IN_CurrNode, NewNode);
			IN_CurrNode = NewNode;
			State = 1;
		}
		else if (State == 1) {
			if (Type != DSON_TOKEN_EQUAL) {
				DSON_LogError(
					"Unexpected '%s' at line [%i:%i]. Expected a '=' sign.",
					Tokens.Entries[X],
					IN_LineNum,
					Tokens.Offsets[X]
				);
				return NULL;
			}

			State = 2;
		}
		else if (State == 2) {
			if (Type == DSON_TOKEN_OPEN_B) {
				State = 3;
				continue;
			}
			if (Type == DSON_TOKEN_OPEN_C) {
				// Dont create a new Node here,
				// The next expected token is a Literal name
				// Taht will create a new node for itself anyway
				State = 0;
				continue;
			}

			DSON_LogError(
				"Unexpected '%s' at line [%i:%i]. Expected a value or a group definition ('[' or '{').",
				Tokens.Entries[X],
				IN_LineNum,
				Tokens.Offsets[X]
			);
			return NULL;
		}
		else if (State == 3) {
			if (Type != DSON_TOKEN_LITERAL) {
				DSON_LogError(
					"Unexpected '%s' at line [%i:%i]. Expected a literal value.",
					Tokens.Entries[X],
					IN_LineNum,
					Tokens.Offsets[X]
				);
				return NULL;
			}
			IN_CurrNode->Value = Tokens.Entries[X];
			State = 4;
		}
		else if (State == 4) {
			if (Type != DSON_TOKEN_CLOSE_B) {
				DSON_LogError(
					"Unexpected '%s' at line [%i:%i]. Expected an end of value definition (']').",
					Tokens.Entries[X],
					IN_LineNum,
					Tokens.Offsets[X]
				);
				return NULL;
			}
			IN_CurrNode = IN_CurrNode->Parent;
			State = 0;
		}
	}


	switch (State) {
		case 0: return IN_CurrNode;
		case 1:
			DSON_LogError(
				"Unexpected end of line %i. Expected a literal name.",
				IN_LineNum
			);
			return NULL;
		case 2:
			DSON_LogError(
				"Unexpected end of line %i. Expecting value of group definition ('[' or '{').",
				IN_LineNum
			);
			return NULL;
		case 3:
			DSON_LogError(
				"Unexpected end of line %i. Expecting Literal Value.",
				IN_LineNum
			);
			return NULL;
		case 4:
			DSON_LogError(
				"Unexpected end of line %i. Expecting and end of value definition (']').",
				IN_LineNum
			);
			return NULL;
		default:
			DSON_LogError(
				"Unexpected parser state at line %i (Should be unreachable!).",
				IN_LineNum
			);
			return NULL;
	}
}

DSON_Node* DSON_ParseTextFile(char* IN_FileName) {
	FILE* FilePointer;
	char* CurrLine = NULL;
	size_t Len = 0;
	ssize_t Read;

	FilePointer = fopen(IN_FileName, "r");
	if (FilePointer == NULL) {
		DSON_LogError("Unable to open file [%s]", IN_FileName);
		return NULL;
	}


	int LineNum = 1;
	DSON_Node* Root = DSON_CreateEmptyNode();
	Root->Name = "Root";
	DSON_Node* CurrNode = Root;

	while ((Read = getline(&CurrLine, &Len, FilePointer)) != -1) {
		// Remove Line Break from end of line
		if (CurrLine[Read - 1] == '\n') { CurrLine[Read - 1] = 0; }
		CurrNode = DSON_ParseLine(Root, CurrNode, LineNum, CurrLine);
		if (CurrNode == NULL) {
			DSON_LogError(
				"Failed to parse [%s].",
				IN_FileName
			);
			return NULL;
		}
		LineNum++;
	}

	fclose(FilePointer);
	if (CurrLine) { free(CurrLine); }

	return Root;
}
DSON_Node* DSON_ParseString(char* IN_String) {
	DSON_StringList List = DSON_SplitString(IN_String, '\n');

	DSON_Node* Root = DSON_CreateEmptyNode();
	DSON_Node* CurrNode = Root;
	int LNum = 1;

	for (size_t X = 0; X < List.Num; X++) {
		CurrNode = DSON_ParseLine(Root, CurrNode, LNum, List.Entries[X]);
		if (CurrNode == NULL) {
			DSON_LogError("Failed to parse String");
			return NULL;
		}
		LNum++;
	}

	return Root;
}
bool DSON_SaveToTextFile(DSON_Node* IN_Node, char* IN_FileName) {
	char* String = DSON_Node_ToString(IN_Node);

	FILE* FilePointer;
	FilePointer = fopen(IN_FileName, "w");
	if (FilePointer == NULL) {
		DSON_LogError("Unable to save file [%s]", IN_FileName);
		return false;
	}

	fprintf(FilePointer,"%s", String);

	fclose(FilePointer);
	return true;
}
bool DSON_SaveToCompactTextFile(DSON_Node* IN_Node, char* IN_FileName) {
	char* String = DSON_Node_ToCompactString(IN_Node);

	FILE* FilePointer;
	FilePointer = fopen(IN_FileName, "w");
	if (FilePointer == NULL) {
		DSON_LogError("Unable to save file [%s]", IN_FileName);
		return false;
	}

	fprintf(FilePointer,"%s", String);

	fclose(FilePointer);
	return true;
}
bool DSON_SaveToBinaryFile(DSON_Node* IN_Node, char* IN_FileName) {
	char* String = DSON_Node_ToCompactString(IN_Node);

	FILE* FilePointer;
	FilePointer = fopen(IN_FileName, "wb");
	if (FilePointer == NULL) {
		DSON_LogError("Unable to save file [%s]", IN_FileName);
		return false;
	}

	size_t Len = strlen(String);
	fwrite(String, sizeof(char), Len, FilePointer);

	fclose(FilePointer);
	return true;
}

#ifdef cplusplus
	}
#endif

// End Implementation
// ------------------------------------------------------------------------------------------------

#endif
