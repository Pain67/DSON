#include "DSON.h"

// ------------------------------------------------------------------------------------------------
// Globals

DSON_LogFunction DSON_LogCallback = NULL;
int DSON_MinLogLevel = 0;

// ----------------------------------------------------------------------
// Util Functions


char* DSON_CreateUniformString(char IN_Char, size_t IN_Num) {
	// +1 for the terminateing 0
	char* Result = (char*)malloc(sizeof(char) * (IN_Num + 1));

	for (size_t X = 0; X < IN_Num; X++) { Result[X] = IN_Char; }
	Result[IN_Num] = 0;

	return Result;
}

void DSON_FreeStringList(DSON_StringList* IN_List) {
	if (IN_List->Num == 0) { return; }

	for (size_t X = 0; X < IN_List->Num; X++) {
		free(IN_List->Entries[X]);
		IN_List->Entries[X] = NULL;
	}
	free (IN_List->Offsets);
	IN_List->Offsets = NULL;
	free(IN_List->Entries);
	IN_List->Entries = NULL;
}

void DSON_FreeNodeList(DSON_NodeList* IN_List) {
	if (IN_List->Num == 0) { return; }

	for (size_t X = 0; X < IN_List->Num; X++) {
		DSON_FreeNode(IN_List->Entries[X]);
		//free();
	}

	free(IN_List->Entries);
	IN_List->Entries = NULL;
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
	Result.Entries = (char**)malloc(sizeof(char*) * Result.Num);
	Result.Offsets = (int*)malloc(sizeof(int) * Result.Num);

	// Allocate Space for Buffer
	Buffer = (char*)malloc(sizeof(char) * DSON_STRING_BUFFER_SIZE);
	memset(Buffer, 0, DSON_STRING_BUFFER_SIZE);

	// Building Result Array
	for (size_t X = 0; X < Len; X++) {
		if (IN_String[X] == IN_Delimiter) {
			if (BufferIndex > 0) {
				// BufferIndex + 1 to make space for terminating 0
				Result.Entries[ResultIndex] = (char*)malloc(sizeof(char) * (BufferIndex + 1));
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
		// BufferIndex + 1 to make space for terminating 0
		Result.Entries[ResultIndex] = (char*)malloc(sizeof(char) * (BufferIndex + 1));
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

	Buffer = (char*)malloc(sizeof(char) * DSON_STRING_BUFFER_SIZE);
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

	Result = (char*)malloc(sizeof(char) * BufferIndex);
	strcpy(Result, Buffer);
	free(Buffer);

	return Result;
}
char* DSON_CopyString(char* IN_String) {
	if (IN_String == NULL) { return NULL; }
	size_t Len = strlen(IN_String);

	if (Len == 0) {
		char* Result = (char*)malloc(sizeof(char));
		Result[0] = 0;
		return Result;
	}
	else {
		char* Result = (char*)malloc(sizeof(char) * (Len + 1));
		strcpy(Result, IN_String);
		return Result;
	}
}
char* DSON_MakeString(char* IN_Format, ...) {
	char* TempBuffer = (char*)malloc(sizeof(char) * DSON_STRING_BUFFER_SIZE);
	memset(TempBuffer, 0, DSON_STRING_BUFFER_SIZE);

	va_list Args;
	va_start(Args, IN_Format);
	size_t Size = vsprintf(TempBuffer, IN_Format, Args);
	va_end(Args);

	// For the terminateing 0;
	Size++;

	char* Result = (char*)malloc(sizeof(char) * Size);
	memset(Result, 0, Size);

	strcpy(Result, TempBuffer);
	free(TempBuffer);

	return Result;
}

char* DSON_ReplaceString(char* IN_From, char* IN_To, char* IN_String) {
	// https://stackoverflow.com/questions/779875/what-function-is-to-replace-a-substring-from-a-string-in-c
	// Posted by: jmucchiello
	// Edited by: 1156752

	// The optimizer should eliminate most of the local variables
	char *result; // the return string
	char *ins;    // the next insert point
	char *tmp;    // varies
	int len_rep;  // length of IN_From (the string to remove)
	int len_with; // length of IN_To (the string to replace IN_From IN_To)
	int len_front; // distance between IN_From and end of last IN_From
	int count;    // number of replacements

	// sanity checks and initialization
	if (!IN_String || !IN_From) return NULL;
	len_rep = strlen(IN_From);

	if (len_rep == 0) return NULL; // empty IN_From causes infinite loop during count
	if (!IN_To) IN_To = "";
	len_with = strlen(IN_To);

	// count the number of replacements needed
	ins = IN_String;
	for (count = 0; (tmp = strstr(ins, IN_From)); ++count) {
		ins = tmp + len_rep;
	}

	tmp = result = (char*)malloc(strlen(IN_String) + (len_with - len_rep) * count + 1);

	if (!result)
		return NULL;

	// first time through the loop, all the variable are set correctly
	// from here on,
	//    tmp points to the end of the result string
	//    ins points to the next occurrence of IN_From in IN_String
	//    IN_String points to the remainder of IN_String after "end of IN_From"
	while (count--) {
		ins = strstr(IN_String, IN_From);
		len_front = ins - IN_String;
		tmp = strncpy(tmp, IN_String, len_front) + len_front;
		tmp = strcpy(tmp, IN_To) + len_with;
		IN_String += len_front + len_rep; // move to next "end of IN_From"
	}
	strcpy(tmp, IN_String);
	return result;
}
char* DSON_RemoveSpace(char* IN_String) { return DSON_ReplaceString(" ","&#32;", IN_String); }
char* DSON_AddSpace(char* IN_String) { return DSON_ReplaceString("&#32;", " ", IN_String); }


char* DSON_IntToString(long long IN_Value) {
	char* Buffer = (char*)malloc(sizeof(char) * DSON_STRING_BUFFER_SIZE);
	char* Result = NULL;

	size_t CharNum = sprintf(Buffer,"%lld", IN_Value);

	// +1 as charnum returned from sprintf not include the terminating 0
	Result = (char*)malloc(sizeof(char) * (CharNum + 1));

	strcpy(Result, Buffer);
	free(Buffer);

	return Result;
}
char* DSON_UIntToString(unsigned long long IN_Value) {
	char* Buffer = (char*)malloc(sizeof(char) * DSON_STRING_BUFFER_SIZE);
	char* Result = NULL;

	size_t CharNum = sprintf(Buffer,"%llu", IN_Value);

	// +1 as charnum returned from sprintf not include the terminating 0
	Result = (char*)malloc(sizeof(char) * (CharNum + 1));

	strcpy(Result, Buffer);
	free(Buffer);

	return Result;
}
char* DSON_FloatToString(float IN_Value) {
	char* Buffer = (char*)malloc(sizeof(char) * DSON_STRING_BUFFER_SIZE);
	char* Result = NULL;

	size_t CharNum = sprintf(Buffer,"%f", IN_Value);

	// +1 as charnum returned from sprintf not include the terminating 0
	Result = (char*)malloc(sizeof(char) * (CharNum + 1));

	strcpy(Result, Buffer);
	free(Buffer);

	return Result;
}


void DSON_Log(int IN_LogLevel, char* IN_Format, va_list IN_Args) {
	if (IN_LogLevel < DSON_MinLogLevel) { return; }

	char* Buffer = (char*)malloc(sizeof(char) * 1024);
	vsnprintf(Buffer, 1024, IN_Format, IN_Args);

	if (DSON_LogCallback != NULL) {
		DSON_LogCallback(IN_LogLevel, Buffer);
		free(Buffer);
		return;
	}

	char* LogType = (char*)malloc(sizeof(char) * 10);
	switch(IN_LogLevel) {
		case 0: strcpy(LogType, "DEBUG"); break;
		case 1: strcpy(LogType, "INFO"); break;
		case 2: strcpy(LogType, "WARNING"); break;
		case 3: strcpy(LogType, "ERROR"); break;
		default: strcpy(LogType, "UNDEFINED"); break;
	}
	printf("[%s] => %s\n", LogType, Buffer);
	free(Buffer);
	free(LogType);
}

void DSON_LogDebug(char* IN_Format, ...) {
	if (DSON_MinLogLevel > DSON_LOG_LEVEL_DEBUG) { return; }
	va_list args;
	va_start(args, IN_Format);
	DSON_Log(DSON_LOG_LEVEL_DEBUG, IN_Format, args);
	va_end(args);
}
void DSON_LogInfo(char* IN_Format, ...) {
	if (DSON_MinLogLevel > DSON_LOG_LEVEL_INFO) { return; }
	va_list args;
	va_start(args, IN_Format);
	DSON_Log(DSON_LOG_LEVEL_INFO, IN_Format, args);
	va_end(args);
}
void DSON_LogWarning(char* IN_Format, ...) {
	if (DSON_MinLogLevel > DSON_LOG_LEVEL_WARNING) { return; }
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
	DSON_StringList* NewList = (DSON_StringList*)malloc(sizeof(DSON_StringList));

	NewList->Entries = NULL;
	NewList->Num = 0;

	return NewList;
}

DSON_NodeList* DSON_CreateEmptyNodeList() {
	DSON_NodeList* NewList = (DSON_NodeList*)malloc(sizeof(DSON_NodeList));

	NewList->Entries = NULL;
	NewList->Num = 0;

	return NewList;
}

DSON_NodeList* DSON_ResizeNodeList(DSON_NodeList* IN_List, size_t IN_NewSize) {
	if (IN_NewSize == IN_List->Num) { return IN_List; }

	// Create New List
	DSON_NodeList* NewList = DSON_CreateEmptyNodeList();

	// Allocate Element
	NewList->Entries = (DSON_Node**)malloc(sizeof(DSON_Node*) * IN_NewSize);
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
	free(IN_List->Entries);
	IN_List->Entries = NULL;
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

	DSON_StringList List = DSON_SplitString(IN_Key, '/');

	if (List.Num == 1) {
		DSON_FreeStringList(&List);

		for (size_t X = 0; X < IN_Ptr->Childs->Num; X++) {
			if (strcmp(IN_Ptr->Childs->Entries[X]->Name, IN_Key) == 0) { return X; }
		}

		return DSON_UINT_MAX;
	}
	else {
		size_t Index = DSON_Node_GetKeyIndex(IN_Ptr, List.Entries[0]);
		if (Index == DSON_UINT_MAX) {
			DSON_FreeStringList(&List);
			return DSON_UINT_MAX;
		}

		char* NewKey = DSON_MergeString(&List,'/',true);
		size_t Result = DSON_Node_GetKeyIndex(IN_Ptr->Childs->Entries[Index], NewKey);

		free(NewKey);
		DSON_FreeStringList(&List);
		return Result;
	}
}

DSON_Node* DSON_CreateEmptyNode() {
	DSON_Node* NewNode = (DSON_Node*)malloc(sizeof(DSON_Node));

	NewNode->Name = NULL;
	NewNode->Value = NULL;
	NewNode->Parent = NULL;
	NewNode->Childs = DSON_CreateEmptyNodeList();

	NewNode->Level = 0;

	return NewNode;
}
void DSON_FreeNode(DSON_Node* IN_Node) {
	if (IN_Node == NULL) { return; }

	if (IN_Node->Name != NULL) { free(IN_Node->Name); IN_Node->Name = NULL; }
	if (IN_Node->Value != NULL) { free(IN_Node->Value); IN_Node->Value = NULL; }
	DSON_FreeNodeList(IN_Node->Childs);
	free(IN_Node->Childs);
	IN_Node->Childs = NULL;
	free(IN_Node);
	IN_Node = NULL;
}

void DSON_Node_Print(DSON_Node* IN_Node) {
	if (IN_Node == NULL) { return; }

	char* Indent = DSON_CreateUniformString(' ', (IN_Node->Level * DSON_STR_INDENT_SIZE));

	if (DSON_Node_isValue(IN_Node)) {
		char* Temp = DSON_AddSpace(IN_Node->Value);
		printf("%s%s = [ %s ]\n", Indent, IN_Node->Name, Temp);
		free(Temp);
	}
	else if (DSON_Node_isGroup(IN_Node)) {
		printf("%s%s = {\n", Indent, IN_Node->Name);
		for (size_t X = 0; X < IN_Node->Childs->Num; X++) {
			DSON_Node_Print(IN_Node->Childs->Entries[X]);
		}
		printf("%s}\n", Indent);
	}
	else if (DSON_Node_isEmpty(IN_Node)) {
		printf("%s%s = [ ]\n", Indent, IN_Node->Name);
	}
	else {
		// UNREACHABLE
	}

	free(Indent);
	return;
}
void DSON_Node_PrintCompact(DSON_Node* IN_Node) {
	if (IN_Node == NULL) { return; }
	if (DSON_Node_isValue(IN_Node)) {
		char* Temp = DSON_AddSpace(IN_Node->Value);
		printf(" %s = [ %s ]", IN_Node->Name, Temp);
		free(Temp);
	}
	else if (DSON_Node_isGroup(IN_Node)) {
		printf(" %s = { ", IN_Node->Name);
		for (size_t X = 0; X < IN_Node->Childs->Num; X++) {
			DSON_Node_PrintCompact(IN_Node->Childs->Entries[X]);
		}
		printf(" }");
	}
	else if (DSON_Node_isEmpty(IN_Node)) {
		printf(" %s = [ ]", IN_Node->Name);
	}
	else {
		// UNREACHABLE
	}
	if (IN_Node->Level == 0) { printf("\n"); }
	return;
}
void DSON_Node_PrintBinary(DSON_Node* IN_Node) {
	char* Temp = DSON_Node_ToBinaryString(IN_Node);
	printf("%s\n",Temp);
	free(Temp);
}
char* DSON_Node_ToString(DSON_Node* IN_Node) {
	if (IN_Node == NULL) { return NULL; }
	char* Indent = DSON_CreateUniformString(' ', (IN_Node->Level * DSON_STR_INDENT_SIZE));

	if (DSON_Node_isValue(IN_Node)) {
		char* Result = DSON_MakeString("%s%s = [ %s ]\n", Indent, IN_Node->Name, IN_Node->Value);
		free(Indent);
		return Result;
	}
	else if (DSON_Node_isGroup(IN_Node)) {
		// Get the String From the childs
		char* ChildsStr = (char*)malloc(sizeof(char));
		ChildsStr[0] = 0;
		for (size_t X = 0; X < IN_Node->Childs->Num; X++) {
			char* Curr = DSON_Node_ToString(IN_Node->Childs->Entries[X]);
			char* TempSting = DSON_MakeString("%s%s", ChildsStr, Curr);
			free(ChildsStr);
			free(Curr);
			ChildsStr = TempSting;
		}
		// Add Open and close tag if needed
		if (IN_Node->Name != NULL && strcmp(IN_Node->Name,"Root") != 0) {
			char* Open = DSON_MakeString("%s%s = {\n", Indent, IN_Node->Name);
			char* TempString = DSON_MakeString("%s%s%s}\n", Open, ChildsStr,Indent);
			free(Indent);
			free(ChildsStr);
			free(Open);
			return TempString;
		}
		else {
			free(Indent);
			return ChildsStr;
		}
	}
	else if (DSON_Node_isEmpty(IN_Node)) {
		char* Result = DSON_MakeString("%s%s = [ ]\n", Indent, IN_Node->Name);
		free(Indent);
		return Result;
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
		// Get the String From the childs
		char* ChildsStr = (char*)malloc(sizeof(char));
		ChildsStr[0] = 0;
		for (size_t X = 0; X < IN_Node->Childs->Num; X++) {
			char* Curr = DSON_Node_ToCompactString(IN_Node->Childs->Entries[X]);
			char* TempSting = DSON_MakeString("%s%s", ChildsStr, Curr);
			free(ChildsStr);
			free(Curr);
			ChildsStr = TempSting;
		}
		// Add Open and close tag if needed
		if (IN_Node->Name != NULL && strcmp(IN_Node->Name,"Root") != 0) {
			char* Open = DSON_MakeString("%s = { ", IN_Node->Name);
			char* TempString = DSON_MakeString("%s%s } ", Open, ChildsStr);
			free(ChildsStr);
			free(Open);
			return TempString;
		}
		else { return ChildsStr; }
	}
	else if (DSON_Node_isEmpty(IN_Node)) {
		return DSON_MakeString("%s = [ ] ", IN_Node->Name);
	}
	else {
		// UNREACHABLE
		return NULL;
	}
}
char* DSON_Node_ToBinaryString(DSON_Node* IN_Node) {
	const char Lookup[16] = { '0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F' };
	char* Temp = DSON_Node_ToCompactString(IN_Node);
	size_t Len = strlen(Temp);
	size_t BuffPos = 0;
	size_t BuffRemain = Len * 3;
	char* FinalBuffer = (char*)malloc(sizeof(char) * BuffRemain);

	char* Buffer_A = (char*)malloc(sizeof(char) * 2);
	char* Buffer_B = (char*)malloc(sizeof(char) * 2);
	Buffer_A[0] = 0;
	Buffer_A[1] = 0;
	Buffer_B[0] = 0;
	Buffer_B[1] = 0;

	for (size_t X = 0; X < Len; X++) {
		unsigned char L = Temp[X] & 0b00001111;
		unsigned char H = Temp[X] >> 4;
		Buffer_A[0] = Lookup[H];
		Buffer_B[0] = Lookup[L];
		//printf("%s%s ", Buffer_A, Buffer_B);
		snprintf(&FinalBuffer[BuffPos], BuffRemain,"%s%s", Buffer_A, Buffer_B);
		BuffPos += 2;
		BuffRemain -= 2;
	}

	free(Buffer_A);
	free(Buffer_B);
	free(Temp);
	return FinalBuffer;
}
int DSON_Node_Count(DSON_Node* IN_Node) {
	int Result = 0;

	if (IN_Node != NULL) {
		if (IN_Node->Childs != NULL) {
			int Temp = IN_Node->Childs->Num;
			Result += Temp;
			if (Temp > 0) {
				for (int X = 0; X < Temp; X++) {
					Result += DSON_Node_Count(IN_Node->Childs->Entries[X]);
				}
			}
		}
	}

	return Result;
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

	// Remove space from IN_Value (if any)
	IN_Value = DSON_RemoveSpace(IN_Value);
	DSON_Node* TargetNode = nullptr;
	
	if (List.Num == 1) {
		size_t Index = DSON_Node_GetKeyIndex(IN_Node, IN_Key);
		if (Index < DSON_UINT_MAX) {
			if (IN_isAllowOverride == true) {
				if (DSON_Node_isGroup(IN_Node->Childs->Entries[Index]) == false) {
					char* Temp = IN_Node->Childs->Entries[Index]->Value;
					IN_Node->Childs->Entries[Index]->Value = (char*)malloc(sizeof(char) * (strlen(IN_Value) + 1));
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

		// +1 as length from strlen will not include the null termination char
		NewNode->Name = (char*)malloc(sizeof(char) * (strlen(IN_Key) + 1));
		strcpy(NewNode->Name, IN_Key);
		
		DSON_Node_AddChild(IN_Node, NewNode);

		// +1 as length from strlen will not include the null termination char
		NewNode->Value = (char*)malloc(sizeof(char) * (strlen(IN_Value) + 1));
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
			TargetNode = IN_Node->Childs->Entries[Index];
			//if (IN_isAllowOverride) {
			//	char* NewKey = DSON_MergeString(&List, '/', true);
			//	Result = DSON_Node_AddValueString(IN_Node->Childs->Entries[Index], NewKey, IN_Value, IN_isAllowOverride);
			//	free(NewKey);
			//	goto CleanUp;
			//}
			//DSON_LogError(
			//	"Unable to add StringValue to [%s]. Key [%s] (from Key Chain [%s]) already exist",
			//				IN_Node->Name,
			//	List.Entries[0],
			//	IN_Key
			//);
			//Result = false;
			//goto CleanUp;
		}
		else {
			TargetNode = DSON_CreateEmptyNode();
			// +1 as length from strlen will not include the null termination char
			TargetNode->Name = (char*)malloc(sizeof(char) * (strlen(List.Entries[0]) + 1));
			strcpy(TargetNode->Name, List.Entries[0]);
			
			DSON_Node_AddChild(IN_Node, TargetNode);
		}

		char* NewKey = DSON_MergeString(&List, '/', true);
		Result = DSON_Node_AddValueString(TargetNode, NewKey, IN_Value, IN_isAllowOverride);
		free(NewKey);
	}

	CleanUp:
	DSON_FreeStringList(&List);
	free(IN_Value);
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
		DSON_FreeStringList(&List);
		return NULL;
	}
	DSON_Node* Target = IN_Node->Childs->Entries[Index];

	if (List.Num == 1) {
		DSON_FreeStringList(&List);
		if (DSON_Node_isValue(Target)) {
			return DSON_AddSpace(Target->Value);
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
		char* A = DSON_Node_GetValueString(Target, NewKey);
		free(NewKey);
		DSON_FreeStringList(&List);
		return A;
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
DSON_Node* DSON_GetSubNode(DSON_Node* IN_Node, char* IN_Key) {
	if (IN_Node == NULL) {
		DSON_LogError("Unable to Get SubNode fromNULL");
		return NULL;
	}

	DSON_StringList List = DSON_SplitString(IN_Key, '/');

	if (List.Num == 1) {
		size_t Index = DSON_Node_GetKeyIndex(IN_Node, IN_Key);
		if (Index == DSON_UINT_MAX) {
			DSON_LogError(
				"Unable to get SubNode. Key [%s] not found in Node [%s]",
				IN_Key,
				IN_Node->Name
			);
			DSON_FreeStringList(&List);
			return NULL;
		}
		DSON_FreeStringList(&List);
		return IN_Node->Childs->Entries[Index];
	}
	else {
		size_t Index = DSON_Node_GetKeyIndex(IN_Node, List.Entries[0]);
		if (Index == DSON_UINT_MAX) {
			DSON_LogError(
				"Unable to get SubNode. Key [%s] not found in Node [%s]",
				 IN_Key,
				 IN_Node->Name
			);
			DSON_FreeStringList(&List);
			return NULL;
		}
		char* NewKey = DSON_MergeString(&List, '/', true);
		DSON_Node* Result = DSON_GetSubNode(IN_Node->Childs->Entries[Index], NewKey);
		free(NewKey);
		DSON_FreeStringList(&List);
		return Result;
	}
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
	//	3 - Literal Value also handles empty groups with ']'
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
			NewNode->Name = DSON_CopyString(Tokens.Entries[X]);
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
				if (Type != DSON_TOKEN_CLOSE_B) {
					DSON_LogError(
						"Unexpected '%s' at line [%i:%i]. Expected a literal value.",
				   Tokens.Entries[X],
				   IN_LineNum,
				   Tokens.Offsets[X]
					);
					return NULL;
				}
				else {
					IN_CurrNode = IN_CurrNode->Parent;
					State = 0;
					continue;
				}
			}
			IN_CurrNode->Value = DSON_CopyString(Tokens.Entries[X]);
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

	DSON_FreeStringList(&Tokens);

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
	Root->Name = (char*)malloc(sizeof(char) * 5);
	memset(Root->Name, 0, 5);
	strcpy(Root->Name, "Root");
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
	free(String);
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
	free(String);
	return true;
}
bool DSON_SaveToBinaryFile(DSON_Node* IN_Node, char* IN_FileName) {
	char* String = DSON_Node_ToBinaryString(IN_Node);

	FILE* FilePointer;
	FilePointer = fopen(IN_FileName, "wb");
	if (FilePointer == NULL) {
		DSON_LogError("Unable to save file [%s]", IN_FileName);
		if (String != NULL) { free(String); }
		return false;
	}

	size_t Len = strlen(String);
	fwrite(String, sizeof(char), Len, FilePointer);

	fclose(FilePointer);
	if (String != NULL) { free(String); }
	return true;
}
