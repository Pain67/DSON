// --------------------------------------------------------------------------------------------
// Defines

#define true 1
#define false 0
#define DSON_STRING_BUFFER_SIZE 1024
#define DSON_UINT_MAX 4294967295
#define DSON_FLOAT_PREC 100000
#define DSON_STR_INDENT_SIZE 2

#define DSON_STR_GROUP "{#GROUP}"
#define DSON_STR_EMPTY "{#EMPTY}"
#define DSON_STR_NULL "{#NULL}"

#define DSON_LOG_LEVEL_DEBUG	0
#define DSON_LOG_LEVEL_INFO		1
#define DSON_LOG_LEVEL_WARNING	2
#define DSON_LOG_LEVEL_ERROR	3

#define DSON_TOKEN_UNDEFINED	0
#define DSON_TOKEN_LITERAL		1
#define DSON_TOKEN_EQUAL		2
#define DSON_TOKEN_OPEN_B		3
#define DSON_TOKEN_OPEN_C		4
#define DSON_TOKEN_CLOSE_B		5
#define DSON_TOKEN_CLOSE_C		6

// --------------------------------------------------------------------------------------------
// Extern C Stuff

extern "c" I64 atoi(U8* IN_String);
extern "c" U64 strtoull(U8* str, U8** str_end, I32 base);

// --------------------------------------------------------------------------------------------
// TypeDefs

class DSON_StringList {
	U8** Entries;
	U32* Offsets;
	U32 Num;
};

class DSON_NodeList {
	U0** Entries;
	U32 Num;
};

class DSON_Node {
	U8* Name;
	U8* Value;

	U0* Parent;
	DSON_NodeList* Childs;

	U32 Level;
};

// --------------------------------------------------------------------------------------------
// Globals

static I64 DSON_LogLevel = DSON_LOG_LEVEL_INFO;

// --------------------------------------------------------------------------------------------
// Util Functions

U8* DSON_CreateUniformString(U8 IN_Char, U32 IN_Num) {
	// +1 for the terminateing 0
	U8* Result = MAlloc(sizeof(U8) * (IN_Num + 1));

	for (U32 X = 0; X < IN_Num; X++) { Result[X] = IN_Char; }
	Result[IN_Num] = 0;

	return Result;
}

U0 DSON_FreeStringList(DSON_StringList* IN_List) {
	if (IN_List->Num == 0) { return; }

	for (U32 X = 0; X < IN_List->Num; X++) {
		if (IN_List->Entries[X] != NULL) { Free(IN_List->Entries[X]); }
	}
	Free(IN_List->Offsets);
	Free(IN_List->Entries);
}

U0 DSON_FreeNodeList(DSON_NodeList* IN_List) {
	if (IN_List->Num == 0) { return; }

	for (U32 X = 0; X < IN_List->Num; X++) {
	    Free(IN_List->Entries[X]);
	}

	Free(IN_List->Entries);
}

DSON_StringList* DSON_SplitString(U8* IN_String, U8 IN_Delimiter) {
	DSON_StringList* Result = MAlloc(sizeof(DSON_StringList));
	Result->Num = 0;
	Result->Entries = NULL;
	U64 Len = StrLen(IN_String);
	U32 CharNum = 0;
	U8* Buffer = NULL;
	U64 BufferIndex = 0;
	U64 ResultIndex = 0;
	// Starts with true, so if first char is a delimiter it will just be ignored
	U8 isLastDelimiter = true;

	// Count the number of Tokens
	for (U32 X = 0; X < Len; X++) {
		if (IN_String[X] == IN_Delimiter) {
			if (isLastDelimiter == false) { Result->Num++; }
			isLastDelimiter = true;
		}
		else { isLastDelimiter = false; }
	}

	//  +1 as we need one more token than number of delimiters
	Result->Num ++;

	// Allocate Space for Result Array
	Result->Entries = MAlloc(sizeof(U8*) * Result->Num);
	Result->Offsets = MAlloc(sizeof(U32) * Result->Num);

	// Allocate Space for Buffer
	Buffer = MAlloc(sizeof(U8) * DSON_STRING_BUFFER_SIZE);
	MemSet(Buffer, 0, DSON_STRING_BUFFER_SIZE);

    // Building Result Array
    for (U32 X = 0; X < Len; X++) {
		if (IN_String[X] == IN_Delimiter) {
			if (BufferIndex > 0) {
				Result->Entries[ResultIndex] = MAlloc(sizeof(U8) * (BufferIndex + 1));
				StrCpy(Result->Entries[ResultIndex], Buffer);
				Result->Offsets[ResultIndex] = CharNum;
				ResultIndex++;
				BufferIndex = 0;
				MemSet(Buffer, 0, DSON_STRING_BUFFER_SIZE);
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
        Result->Entries[ResultIndex] = MAlloc(sizeof(U8) * BufferIndex);
        StrCpy(Result->Entries[ResultIndex], Buffer);
		Result->Offsets[ResultIndex] = CharNum;
        ResultIndex++;
    }

    Free(Buffer);

    // All Done :D
    return Result;
}

U8* DSON_MergeString(DSON_StringList* IN_List, U8 IN_Delimiter, U8 IN_isSkipFirst) {
	if (IN_List == NULL) { return NULL; }
	if (IN_List->Num == 0) { return NULL; }

	U8* Buffer = 0;
	U8* Result = 0;
	U32 BufferIndex = 0;

	Buffer = MAlloc(sizeof(U8) * DSON_STRING_BUFFER_SIZE);
	MemSet(Buffer,0,DSON_STRING_BUFFER_SIZE);

	U32 From = 0;
	if (IN_isSkipFirst == true) { From = 1; }

	for (U32 X = From; X < IN_List->Num; X++) {
		if (X > From) {
			Buffer[BufferIndex] = IN_Delimiter;
			BufferIndex++;
		}
		U32 TokenLen = StrLen(IN_List->Entries[X]);
		for (U32 Y = 0; Y < TokenLen; Y++) {
			Buffer[BufferIndex + Y] = IN_List->Entries[X][Y];
		}
		BufferIndex += TokenLen;
	}

	Result = MAlloc(sizeof(U8) * BufferIndex);
	StrCpy(Result, Buffer);
	Free(Buffer);

	return Result;
}

U8* DSON_CopyString(U8* IN_String) {
	if (IN_String == NULL) { return NULL; }
	U32 Len = StrLen(IN_String);

	if (Len == 0) {
		U8* Result = MAlloc(sizeof(U8));
		Result[0] = 0;
		return Result;
	}
	else {
		U8* Result = MAlloc(sizeof(U8) * (Len + 1));
		StrCpy(Result, IN_String);
		return Result;
	}
}
U8* DSON_MakeString(U8* IN_Format, ...) {
	U8* TempStr = StrPrintJoin(NULL, IN_Format, argc, argv);
	return TempStr;
}
U8* DSON_IntToString(I32 IN_Value) {
	return StrPrint(NULL, "%d", IN_Value);
}
U8* DSON_UIntToString(U64 IN_Value) {
	return StrPrint(NULL, "%u", IN_Value);
}

// --------------------------------------------------------------------------------------------
// Log Funtions

U0 DSON_LogDebug(U8* IN_Format, ...) {
	if (DSON_LogLevel > DSON_LOG_LEVEL_DEBUG) { return; }

	U8* Message = StrPrintJoin(NULL, IN_Format, argc, argv);
	"[DEBUG] => %s\n", Message;
	Free(Message);
}
U0 DSON_LogInfo(U8* IN_Format, ...) {
	if (DSON_LogLevel > DSON_LOG_LEVEL_INFO) { return; }

	U8* Message = StrPrintJoin(NULL, IN_Format, argc, argv);
	"[INFO] => %s\n", Message;
	Free(Message);
}
U0 DSON_LogWarning(U8* IN_Format, ...) {
	if (DSON_LogLevel > DSON_LOG_LEVEL_WARNING) { return; }

	U8* Message = StrPrintJoin(NULL, IN_Format, argc, argv);
	"[WARNING] => %s\n", Message;
	Free(Message);
}
U0 DSON_LogError(U8* IN_Format, ...) {
	U8* Message = StrPrintJoin(NULL, IN_Format, argc, argv);
	"[ERROR] => %s\n", Message;
	Free(Message);
}

// --------------------------------------------------------------------------------------------
// DSON List Function

DSON_StringList* DSON_CreateEmptyStringList() {
    DSON_StringList* NewList = MAlloc(sizeof(DSON_StringList));

    NewList->Entries = NULL;
    NewList->Num = 0;

    return NewList;
}

DSON_NodeList* DSON_CreateEmptyNodeList() {
    DSON_NodeList* NewList = MAlloc(sizeof(DSON_NodeList));

    NewList->Entries = NULL;
    NewList->Num = 0;

    return NewList;
}

DSON_NodeList* DSON_ResizeNodeList(DSON_NodeList* IN_List, U32 IN_NewSize) {
	if (IN_NewSize == IN_List->Num) { return IN_List; }

	// Create New List
	DSON_NodeList* NewList = DSON_CreateEmptyNodeList();

	// Allocate Element
	NewList->Entries = MAlloc(sizeof(DSON_Node*) * IN_NewSize);
	NewList->Num = IN_NewSize;

	if (IN_List->Num < IN_NewSize) {
		if (IN_List->Num > 0) {
			for (U32 X = 0; X < IN_List->Num; X++) { NewList->Entries[X] = IN_List->Entries[X]; }
		}
		if (IN_List->Num < IN_NewSize) {
			for (U32 X = IN_List->Num; X < IN_NewSize; X++) { NewList->Entries[X] = NULL; }
		}
	}
	else {
		for (U32 X = 0; X < IN_NewSize; X++) { NewList->Entries[X] = IN_List->Entries[X]; }
	}

	// Deallocate Old List - Dont Use DSON_FreeNodeList as that also deallocates Elements
	Free(IN_List);

	return NewList;
}

// --------------------------------------------------------------------------------------------
// DSON Node Functions

U8 DSON_Node_isValue(DSON_Node* IN_Ptr) {
    if (IN_Ptr->Value == NULL) { return false; }
    else { return true; }
}
U8 DSON_Node_isGroup(DSON_Node* IN_Ptr) {
    if (IN_Ptr->Childs->Num > 0) { return true; }
    else { return false; }
}
U8 DSON_Node_isEmpty(DSON_Node* IN_Ptr) {
    if (IN_Ptr->Value == NULL && IN_Ptr->Childs->Num == 0) { return true; }
    else { return false; }
}
U32 DSON_Node_GetKeyIndex(DSON_Node* IN_Ptr, U8* IN_Key) {
	if (IN_Ptr == NULL) { return DSON_UINT_MAX; }
	if (IN_Ptr->Childs->Num == 0) { return DSON_UINT_MAX; }

	for (U32 X = 0; X < IN_Ptr->Childs->Num; X++) {
		DSON_Node* Temp = IN_Ptr->Childs->Entries[X](DSON_Node*);
		if (StrCmp(Temp->Name, IN_Key) == 0) { return X; }
	}

	return DSON_UINT_MAX;
}


DSON_Node* DSON_CreateEmptyNode() {
    DSON_Node* NewNode = MAlloc(sizeof(DSON_Node));

    NewNode->Name = NULL;
    NewNode->Value = NULL;
    NewNode->Parent = NULL;
    NewNode->Childs = DSON_CreateEmptyNodeList();

    NewNode->Level = 0;

    return NewNode;
}
U0 DSON_Node_Print(DSON_Node* IN_Node) {
	if (IN_Node == NULL) { return; }

	U8* Indent = DSON_CreateUniformString(' ', (IN_Node->Level * DSON_STR_INDENT_SIZE));

	if (DSON_Node_isValue(IN_Node)) {
		"%s%s = [ %s ]\n", Indent, IN_Node->Name, IN_Node->Value;
		return;
	}
	else if (DSON_Node_isGroup(IN_Node)) {
		"%s%s = {\n", Indent, IN_Node->Name;
		for (U32 X = 0; X < IN_Node->Childs->Num; X++) {
			DSON_Node_Print(IN_Node->Childs->Entries[X]);
		}
		"%s}\n", Indent;
		return;
	}
	else if (DSON_Node_isEmpty(IN_Node)) {
		"%s%s = #EMPTY\n", Indent, IN_Node->Name;
		return;
	}
	else {
		"Achivement Unlock: How did we get here? - For reaching an unreachable code :D";
		// UNREACHABLE
	}
}
U8* DSON_Node_ToString(DSON_Node* IN_Node) {
	if (IN_Node == NULL) { return NULL; }
	U8* Indent = DSON_CreateUniformString(' ', (IN_Node->Level * DSON_STR_INDENT_SIZE));

	if (DSON_Node_isValue(IN_Node)) {
		return DSON_MakeString("%s%s = [ %s ]\n", Indent, IN_Node->Name, IN_Node->Value);
	}
	else if (DSON_Node_isGroup(IN_Node)) {
		U8* Result = DSON_MakeString("%s%s = {\n", Indent, IN_Node->Name);
		for (U32 X = 0; X < IN_Node->Childs->Num; X++) {
			U8* ChildString = DSON_Node_ToString(IN_Node->Childs->Entries[X](DSON_Node*));
			U8* TempSting = DSON_MakeString("%s%s", Result, ChildString);
			Free(Result);
			Free(ChildString);
			Result = TempSting;
		}
		U8* CloseTag = DSON_MakeString("%s\n%s}\n", Result, Indent);
		Free(Result);

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
U8* DSON_Node_ToCompactString(DSON_Node* IN_Node) {
	if (IN_Node == NULL) { return NULL; }

	if (DSON_Node_isValue(IN_Node)) {
		return DSON_MakeString("%s = [ %s ] ", IN_Node->Name, IN_Node->Value);
	}
	else if (DSON_Node_isGroup(IN_Node)) {
		U8* Result = DSON_MakeString("%s = { ", IN_Node->Name);
		for (U32 X = 0; X < IN_Node->Childs->Num; X++) {
			U8* ChildString = DSON_Node_ToCompactString(IN_Node->Childs->Entries[X]);
			U8* TempSting = DSON_MakeString("%s%s", Result, ChildString);
			Free(Result);
			Free(ChildString);
			Result = TempSting;
		}
		U8* CloseTag = DSON_MakeString("%s } ", Result);
		Free(Result);

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
U0 DSON_Node_AddChild(DSON_Node* IN_Parent, DSON_Node* IN_Child) {
	DSON_LogDebug("Adding [%s] as a child to [%s]", IN_Child->Name, IN_Parent->Name);
	U32 Index = IN_Parent->Childs->Num;
	IN_Parent->Childs = DSON_ResizeNodeList(IN_Parent->Childs, IN_Parent->Childs->Num + 1);

	IN_Parent->Childs->Entries[Index] = IN_Child;
	IN_Child->Parent = IN_Parent;
	IN_Child->Level = IN_Parent->Level + 1;
}
U8 DSON_Node_AddValueString(DSON_Node* IN_Node, U8* IN_Key, U8* IN_Value, U8 IN_isAllowOverride) {
	if (DSON_Node_isValue(IN_Node)) {
		DSON_LogError("Unable to add Value to NULL");
		return false;
	}

	U8 Result = false;
	DSON_StringList* StringList = DSON_SplitString(IN_Key, '/');

	if (StringList->Num == 1) {
		U32 Index = DSON_Node_GetKeyIndex(IN_Node, IN_Key);
		if (Index < DSON_UINT_MAX) {
			if (IN_isAllowOverride == true) {
				if (DSON_Node_isGroup(IN_Node->Childs->Entries[Index]) == false) {
					DSON_Node* N = IN_Node->Childs->Entries[Index];
					U8* Temp = N->Value;
					N->Value = MAlloc(sizeof(U8) * (StrLen(IN_Value) + 1));
					StrCpy(N->Value, IN_Value);
					Result =  true;
					DSON_LogWarning(
						"Override value of [%s] from [%s] to [%s] in Node [%s]",
						IN_Key,
						Temp,
						IN_Value,
						IN_Node->Name
					);
					Free(Temp);
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
		NewNode->Name = MAlloc(sizeof(U8) * (StrLen(IN_Key) + 1));
		StrCpy(NewNode->Name, IN_Key);

		// +1 as length from strlen will not include the null termination char
		NewNode->Value = MAlloc(sizeof(U8) * (StrLen(IN_Value) + 1));
		StrCpy(NewNode->Value, IN_Value);
		Result = true;
	}
	else {
		U32 Index = DSON_Node_GetKeyIndex(IN_Node, StringList->Entries[0]);
		if (Index < DSON_UINT_MAX) {
			if (DSON_Node_isValue(IN_Node->Childs->Entries[Index]) == true) {
				DSON_LogError(
					"Unable to add StringValue to [%s]. Key [%s] (from Key Chain [%s]) is not a group",
					IN_Node->Name,
					StringList->Entries[0],
					IN_Key
				);
				Result = false;
				goto CleanUp;
			}
			if (IN_isAllowOverride) {
				U8* NewKey = DSON_MergeString(StringList, '/', true);
				Result = DSON_Node_AddValueString(IN_Node->Childs->Entries[Index], NewKey, IN_Value, IN_isAllowOverride);
				Free(NewKey);
				goto CleanUp;
			}
			DSON_LogError(
				"Unable to add StringValue to [%s]. Key [%s] (from Key Chain [%s]) already exist",
				IN_Node->Name,
				StringList->Entries[0],
				IN_Key
			);
			Result = false;
			goto CleanUp;
		}

		DSON_Node* NewNode = DSON_CreateEmptyNode();
		DSON_Node_AddChild(IN_Node, NewNode);
		// +1 as length from strlen will not include the null termination char
		NewNode->Name = MAlloc(sizeof(U8) * (strlen(StringList->Entries[0]) + 1));
		StrCpy(NewNode->Name, StringList->Entries[0]);

		U8* NewKey = DSON_MergeString(StringList, '/', true);
		Result = DSON_Node_AddValueString(NewNode, NewKey, IN_Value, IN_isAllowOverride);
		Free(NewKey);
	}

	CleanUp:
	DSON_FreeStringList(StringList);
	return Result;
}
U8 DSON_Node_AddValueInt(DSON_Node* IN_Node, U8* IN_Key, I64 IN_Value, U8 IN_isAllowOverride) {
	U8* TempValue = DSON_IntToString(IN_Value);
	U8 Result = false;
	Result = DSON_Node_AddValueString(IN_Node, IN_Key, TempValue, IN_isAllowOverride);
	Free(TempValue);
	return Result;
}
U8 DSON_Node_AddValueUInt(DSON_Node* IN_Node, U8* IN_Key, U64 IN_Value, U8 IN_isAllowOverride) {
	U8* TempValue = DSON_UIntToString(IN_Value);
	U8 Result = false;
	Result = DSON_Node_AddValueString(IN_Node, IN_Key, TempValue, IN_isAllowOverride);
	Free(TempValue);
	return Result;
}
U8* DSON_Node_GetValueString(DSON_Node* IN_Node, U8* IN_Key) {
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

	DSON_StringList* StringList = DSON_SplitString(IN_Key, '/');

	U32 Index = DSON_Node_GetKeyIndex(IN_Node, StringList->Entries[0]);
	if (Index == DSON_UINT_MAX) {
		DSON_LogError(
			"Unable to get String Value. Key [%s] not found in node [%s]",
			StringList->Entries[0],
			IN_Node->Name
		);
		DSON_FreeStringList(StringList);
		return NULL;
	}
	DSON_Node* Target = IN_Node->Childs->Entries[Index];

	if (StringList->Num == 1) {
		DSON_FreeStringList(StringList);

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
		U8* NewKey = DSON_MergeString(StringList,'/',true);
		DSON_FreeStringList(StringList);
		return DSON_Node_GetValueString(Target, NewKey);
	}

	// UNREACHABLE
	DSON_FreeStringList(StringList);
	return NULL;
}
U8 DSON_Node_GetValueInt(DSON_Node* IN_Node, U8* IN_Key, I64* OUT_Value) {
	U8* String = DSON_Node_GetValueString(IN_Node, IN_Key);
	if (String == NULL) { return false; }

	if (StrCmp(String, DSON_STR_GROUP) == 0) {
		DSON_LogError(
			"Unable to get [%s] from Node [%s].[%s] cannot be converted to Int",
			IN_Key,
			IN_Node->Name,
			DSON_STR_GROUP
		);
		Free(String);
		return false;
	}
	if (StrCmp(String, DSON_STR_EMPTY) == 0) {
		DSON_LogError(
			"Unable to get [%s] from Node [%s].[%s] cannot be converted to Int",
			IN_Key,
			IN_Node->Name,
			DSON_STR_EMPTY
		);
		Free(String);
		return false;
	}
	if (StrCmp(String, DSON_STR_NULL) == 0) {
		DSON_LogError(
			"Unable to get [%s] from Node [%s].[%s] cannot be converted to Int",
			IN_Key,
			IN_Node->Name,
			DSON_STR_NULL
		);
		Free(String);
		return false;
	}

	(*OUT_Value) = atoi(String);
	Free(String);
	return true;
}
U8 DSON_Node_GetValueUInt(DSON_Node* IN_Node, U8* IN_Key, U64* OUT_Value) {
	U8* String = DSON_Node_GetValueString(IN_Node, IN_Key);
	if (String == NULL) { return false; }

	if (StrCmp(String, DSON_STR_GROUP) == 0) {
		DSON_LogError(
			"Unable to get [%s] from Node [%s].[%s] cannot be converted to Unsigned Int",
			IN_Key,
			IN_Node->Name,
			DSON_STR_GROUP
		);
		Free(String);
		return false;
	}
	if (StrCmp(String, DSON_STR_EMPTY) == 0) {
		DSON_LogError(
			"Unable to get [%s] from Node [%s].[%s] cannot be converted to Unsigned Int",
			IN_Key,
			IN_Node->Name,
			DSON_STR_EMPTY
		);
		Free(String);
		return false;
	}
	if (StrCmp(String, DSON_STR_NULL) == 0) {
		DSON_LogError(
			"Unable to get [%s] from Node [%s].[%s] cannot be converted to Unsigned Int",
			IN_Key,
			IN_Node->Name,
			DSON_STR_NULL
		);
		Free(String);
		return false;
	}

	(*OUT_Value) = strtoull(String, NULL, 10);
	Free(String);
	return true;
}
U8 DSON_Node_Remove(DSON_Node* IN_Node, U8* IN_Key) {
	U32 Index = DSON_Node_GetKeyIndex(IN_Node, IN_Key);
	if (Index == DSON_UINT_MAX) {
		DSON_LogError(
			"Unable to remove Key [%s] from Node [%s]. Key not found",
			IN_Key,
			IN_Node->Name
		);
		return false;
	}

	DSON_Node* ToRemove = IN_Node->Childs->Entries[Index];

	for (U32 X = Index; X < (IN_Node->Childs->Num - 1); X++) {
		IN_Node->Childs->Entries[X] = IN_Node->Childs->Entries[X+1];
	}
	IN_Node->Childs = DSON_ResizeNodeList(IN_Node->Childs, IN_Node->Childs->Num - 1);

	DSON_FreeNodeList(ToRemove->Childs);
	Free(ToRemove);

	DSON_LogInfo(
		"Key [%s] removed from Node [%s]",
		IN_Key,
		IN_Node->Name
	);

	return true;
}
// --------------------------------------------------------------------------------------------
// DSON Token

U32 DSON_GetTokenType(U8* IN_Token) {
	if (IN_Token == NULL) { return DSON_TOKEN_UNDEFINED; }
	U32 Len = strlen(IN_Token);

	if (Len == 0) { return DSON_TOKEN_UNDEFINED; }

	if (Len == 1) {
		if (IN_Token[0] == '=') { return DSON_TOKEN_EQUAL; }
		if (IN_Token[0] == '[') { return DSON_TOKEN_OPEN_B; }
		if (IN_Token[0] == ']') { return DSON_TOKEN_CLOSE_B; }
		if (IN_Token[0] == '{') { return DSON_TOKEN_OPEN_C; }
		if (IN_Token[0] == '}') { return DSON_TOKEN_CLOSE_C; }
	}

	return DSON_TOKEN_LITERAL;
}

// --------------------------------------------------------------------------------------------
// DSON Parser

DSON_Node* DSON_ParseLine(DSON_Node* IN_RootNode, DSON_Node* IN_CurrNode,U32 IN_LineNum, U8* IN_Line) {
	// State Expection list:
	//	0 - Literal Name (Also handles '}')
	//	1 - '='
	//	2 - '[' or '{'
	//	3 - Literal Value
	//	4 - ']'
	U8 State = 0;

	if (IN_Line == NULL) { return IN_CurrNode; }	// Skipp Null Line
	if (IN_Line[0] == 0) { return IN_CurrNode; }	// Skip Empty Line
	if (IN_Line[0] == '\n') { return IN_CurrNode; }	// Skip Empty Line
	if (IN_Line[0] == '#') { return IN_CurrNode; }	// Skip Comment Line

	DSON_StringList* Tokens = DSON_SplitString(IN_Line, ' ');
	for (U32 X = 0; X < Tokens->Num; X++) {
		U32 Type = DSON_GetTokenType(Tokens->Entries[X]);

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
					Tokens->Entries[X],
					IN_LineNum,
					Tokens->Offsets[X]
				);
				return NULL;
			}

			DSON_Node* NewNode = DSON_CreateEmptyNode();
			NewNode->Name = Tokens->Entries[X];
			DSON_Node_AddChild(IN_CurrNode, NewNode);
			IN_CurrNode = NewNode;
			State = 1;
		}
		else if (State == 1) {
			if (Type != DSON_TOKEN_EQUAL) {
				DSON_LogError(
					"Unexpected '%s' at line [%i:%i]. Expected a '=' sign.",
					Tokens->Entries[X],
					IN_LineNum,
					Tokens->Offsets[X]
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
				Tokens->Entries[X],
				IN_LineNum,
				Tokens->Offsets[X]
			);
			return NULL;
		}
		else if (State == 3) {
			if (Type != DSON_TOKEN_LITERAL) {
				DSON_LogError(
					"Unexpected '%s' at line [%i:%i]. Expected a literal value.",
					Tokens->Entries[X],
					IN_LineNum,
					Tokens->Offsets[X]
				);
				return NULL;
			}
			IN_CurrNode->Value = Tokens->Entries[X];
			State = 4;
		}
		else if (State == 4) {
			if (Type != DSON_TOKEN_CLOSE_B) {
				DSON_LogError(
					"Unexpected '%s' at line [%i:%i]. Expected an end of value definition (']').",
					Tokens->Entries[X],
					IN_LineNum,
					Tokens->Offsets[X]
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

DSON_Node* DSON_ParseTextFile(U8* IN_FileName) {
	DSON_LogDebug("Parsing Source File [%s]", IN_FileName);
	U8* Content = FileRead(IN_FileName, NULL);
	if (Content == NULL) {
		DSON_LogError("Unable to open file [%s]", IN_FileName);
		return NULL;
	}

	DSON_StringList* SList = DSON_SplitString(Content, '\n');
	if (SList->Num == 0) {
		return NULL;
	}

	DSON_Node* Root = DSON_CreateEmptyNode();
	Root->Name = "Root";
	DSON_Node* CurrNode = Root;

	for (U64 X = 0; X < SList->Num; X++) {
		U8* CurrLine = SList->Entries[X];
		CurrNode = DSON_ParseLine(Root, CurrNode, X + 1, CurrLine);
		if (CurrNode == NULL) {
			DSON_LogError(
				"Failed to parse [%s].",
				IN_FileName
			);
			return NULL;
		}
	}
	DSON_FreeStringList(SList);
	DSON_LogDebug("File Parsed Successfully");
	return Root;
}
DSON_Node* DSON_ParseString(U8* IN_String) {
	DSON_StringList* SList = DSON_SplitString(IN_String, '\n');

	DSON_Node* Root = DSON_CreateEmptyNode();
	DSON_Node* CurrNode = Root;

	for (U32 X = 0; X < SList->Num; X++) {
		CurrNode = DSON_ParseLine(Root, CurrNode, X + 1, SList->Entries[X]);
		if (CurrNode == NULL) {
			DSON_LogError("Failed to parse String");
			return NULL;
		}
	}

	return Root;
}

// --------------------------------------------------------------------------------------------








U0 Main() {
	U8 *message = "hello world";
	"%s\n",message;
	DSON_Node* Root = DSON_ParseTextFile("/mnt/nvme0n1p4/Development/GIT/DSON/Cpp/Test.dson");
}
Main;
