#include <stdio.h>

#include "../../Sources/C/DSON.h"

int main(int argc, char** argv) {
    printf("\n");

    if (argc == 1) {
        printf("Missing required argument.\nPath to file to parse expected to be the 1st argument\n\n");
        return -1;
    }
    char* Path = argv[1];
    printf("Parsing text file [%s]...\n", Path);

    DSON_Node* Root = DSON_ParseTextFile(Path);

    if (Root == NULL) {
        printf("Failed to parse source file. See details above\n\n");
        return -1;
    }
    else {
        printf("Source file parsed successfully. Loaded [%d] nodes.\n\n", DSON_Node_Count(Root));
    }

    printf("\nPrinting Node Tree:\n-----------------------------------------\n\n");
    DSON_Node_Print(Root);

    printf("\n\nPrinting Node Tree (Compact):\n-----------------------------------------\n\n");
    DSON_Node_PrintCompact(Root);

    printf("\n\nPrinting Node Tree (Binary):\n-----------------------------------------\n\n");
    DSON_Node_PrintBinary(Root);

    printf("\nAdd new String Element (NewKey):\n-----------------------------------------\n\n");
    DSON_Node_AddValueString(Root, "NewKey", "New Value", false);
    DSON_Node_Print(Root);

    printf("\nAdd new String Element (Chained keys):\n-----------------------------------------\n\n");
    DSON_Node_AddValueString(Root, "NewChain/NewChain/NewChain/NewChain", "New Chain Value", false);
    DSON_Node_Print(Root);

    printf("\nAttempt to add already existing key with no override allowed (NewKey):\n-----------------------------------------\n\n");
    DSON_Node_AddValueString(Root, "NewKey", "New Value", false);

    printf("\nAttempt to add already existing key with override allowed (NewKey):\n-----------------------------------------\n\n");
    DSON_Node_AddValueString(Root, "NewKey", "Updated Value", true);

    printf("\nAdd new Signed Integer Element (NewInt):\n-----------------------------------------\n\n");
    DSON_Node_AddValueInt(Root, "NewInt", -42, false);
    DSON_Node_Print(Root);

    printf("\nAdd new Unsigned Integer Element (NewUInt):\n-----------------------------------------\n\n");
    DSON_Node_AddValueInt(Root, "NewUInt", 4294967295, false);
    DSON_Node_Print(Root);

    printf("\nAdd new floating point Element (NewFloat):\n-----------------------------------------\n\n");
    DSON_Node_AddValueFloat(Root, "NewFloat", 12.324F, false);
    DSON_Node_Print(Root);

    printf("\nRead String Value:\n-----------------------------------------\n\n");
    char* Str = DSON_Node_GetValueString(Root, "name");
    if (Str != NULL) {
        printf("Value for key [name] = %s\n", Str);
        free(Str);
    }
    else { printf("Failed to read value for key [name]\n"); }
    char* StrChain = DSON_Node_GetValueString(Root, "bio/hobbies/0");
    if (StrChain != NULL) {
        printf("Value for key [bio/hobbies/0] = %s\n", StrChain);
        free(StrChain);
    }
    else { printf("Failed to read value for key [bio/hobbies/0\n"); }


    printf("\nRead Signed Integer Value:\n-----------------------------------------\n\n");
    long long TempInt = 0;
    if (DSON_Node_GetValueInt(Root, "NewInt", &TempInt)) {
        printf("Value for key [NewInt] = %lld\n\n", TempInt);
    }
    else { printf("Failed to read value for key [NewFloat]\n\n"); }


    printf("\nRead Unsigned Integer Value:\n-----------------------------------------\n\n");
    unsigned long long TempUInt = 0;
    if (DSON_Node_GetValueUInt(Root, "NewUInt", &TempUInt)) {
        printf("Value for key [NewUInt] = %lld\n\n", TempUInt);
    }
    else { printf("Failed to read value for key [NewFloat]\n\n"); }


    printf("\nRead Unsigned Integer Value:\n-----------------------------------------\n\n");
    float TempFloat = 0.0F;
    if (DSON_Node_GetValueFloat(Root, "NewFloat", &TempFloat)) {
        printf("Value for key [NewFloat] = %.4f\n\n", TempFloat);
    }
    else { printf("Failed to read value for key [NewFloat]\n\n"); }

    printf("\nRead Value for key that does not exist:\n-----------------------------------------\n\n");
    char* NoKey = DSON_Node_GetValueString(Root, "InvalidKey");
    if (NoKey == NULL) {
        printf("Failed to read value for key [InvalidKey]\n\n");
    }
    char* NoChainKey = DSON_Node_GetValueString(Root, "contacts/social/InvalidKey");
    if (NoChainKey == NULL) {
        printf("Failed to read value for key [contacts/social/InvalidKey]\n\n");
    }

    printf("\nSave Node Tree\n-----------------------------------------\n\n");
    if (DSON_SaveToTextFile(Root, "Text.dson")) {
        printf("Save Node Tree to text file [Text.dson]\n");
    }
    else { printf("Failed to Save Node Tree to binary file [Text.dson]\n"); }
    if (DSON_SaveToCompactTextFile(Root,"CompactText.dson")) {
        printf("Save Node Tree to compact text file [CompactText.dson]\n");
    }
    else { printf("Failed to Save Node Tree to binary file [Binary.dson]\n"); }
    if (DSON_SaveToBinaryFile(Root, "Binary.dson")) {
        printf("Save Node Tree to binary file [Binary.dson]\n");
    }
    else { printf("Failed to Save Node Tree to binary file [Binary.dson]\n"); }

    printf("\n\nFree All allocated Node(s)...\nApplication now terminates\n\n");
    DSON_FreeNode(Root);

    return 0;
}
