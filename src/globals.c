#include "../includes/globals.h"
#include "../includes/object.h"
#include "../includes/value.h"
#include "../includes/msapi.h"
#include <time.h>
#include <stdlib.h>
#include <string.h>

void injectGlobals(VM* vm) {
    ObjString* str_print = allocateString(vm, "print", 5);
    ObjString* str_str = allocateString(vm, "str", 3);
    ObjString* str_num = allocateString(vm, "num", 3);
    ObjString* str_clock = allocateString(vm, "clock", 5);
    ObjString* str_type = allocateString(vm, "type", 4);
    ObjString* str_input = allocateString(vm, "input", 5);
    ObjString* str_char = allocateString(vm, "char", 4);

    ObjNativeFunction* native_print = allocateNativeFunction(vm, str_print, &msglobal_print);
    ObjNativeFunction* native_clock = allocateNativeFunction(vm, str_clock, &msglobal_clock);
    ObjNativeFunction* native_str = allocateNativeFunction(vm, str_str, &msglobal_str);
    ObjNativeFunction* native_num = allocateNativeFunction(vm, str_num, &msglobal_num);
    ObjNativeFunction* native_type = allocateNativeFunction(vm, str_type, &msglobal_type);
    ObjNativeFunction* native_input = allocateNativeFunction(vm, str_input, &msglobal_input);
    ObjNativeFunction* native_char = allocateNativeFunction(vm, str_char, &msglobal_char);

    insertTable(&vm->globals->table, str_clock, OBJ(native_clock));
    insertTable(&vm->globals->table, str_str, OBJ(native_str));
    insertTable(&vm->globals->table, str_num, OBJ(native_num));
    insertTable(&vm->globals->table, str_type, OBJ(native_type));
    insertTable(&vm->globals->table, str_print, OBJ(native_print));
    insertTable(&vm->globals->table, str_input, OBJ(native_input));
    insertTable(&vm->globals->table, str_char, OBJ(native_char));
}


bool msglobal_print(VM* vm, int argCount, bool shouldReturn) {
    
    for (int i = argCount - 1; i >= 0; i--) {
        if (i != argCount - 1) printf("    ");
        printValue(msapi_peek(vm, i));
    }
    msapi_popn(vm, argCount + 1);       // pop the function itself too
    
    if (shouldReturn) {
        msapi_push(vm, NIL());
    }

    printf("\n");
    return true;
}

bool msglobal_clock(VM* vm, int argCount, bool shouldReturn) {
    msapi_popn(vm, argCount + 1) ;
    
    if (shouldReturn) {
        msapi_push(vm, NATIVE_TO_NUMBER((double)clock() / CLOCKS_PER_SEC));
    }

    return true;
}

bool msglobal_str(VM* vm, int argCount, bool shouldReturn) {
    if (argCount == 0) {
        msapi_runtimeError(vm, "Expected an argument in the 'str() global");
        return false;
    }

    char buffer[1000];
    Value thing = msapi_getArg(vm, 1, argCount);
    msapi_popn(vm, argCount + 1);

    if (!shouldReturn) return true; 
    switch (thing.type) {
        case VAL_NUMBER: {
            snprintf(buffer, 1000, "%g", AS_NUMBER(thing));
            int length = 0;

            for (int i = 0; i < 1000; i++) {
                if (buffer[i] == -1 || buffer[i] == '\0') break;
                length++;
            }
            msapi_push(vm, OBJ(allocateString(vm, buffer, length)));
            break;
        }
        case VAL_BOOL: {
            if (AS_BOOL(thing)) {
                msapi_push(vm, OBJ(allocateString(vm, "true", 4)));
            } else {
                msapi_push(vm, OBJ(allocateString(vm, "false", 5)));
            }
            break;
        }
        case VAL_NIL: {
            msapi_push(vm, OBJ(allocateString(vm, "nil", 3)));
            break;
        }
        case VAL_OBJ: {
            switch (AS_OBJ(thing)->type) {
                case OBJ_STRING:
                    msapi_push(vm, thing);
                    break;
                case OBJ_ARRAY: 
                    msapi_push(vm, OBJ(allocateString(vm, "array", 5)));
                    break;
                case OBJ_CLOSURE:
                    msapi_push(vm, OBJ(AS_CLOSURE(thing)->function->name));
                    break;
                case OBJ_NATIVE_FUNCTION:
                    msapi_push(vm, OBJ(AS_NATIVE_FUNCTION(thing)->name));
                    break;
                case OBJ_INSTANCE:
                    msapi_push(vm, OBJ(AS_INSTANCE(thing)->klass->name));
                    break;
                case OBJ_CLASS:
                    msapi_push(vm, OBJ(AS_CLASS(thing)->name));
                    break;
                case OBJ_METHOD:
                    msapi_push(vm, OBJ(AS_METHOD(thing)->closure->function->name));
                    break;
                case OBJ_TABLE:
                    msapi_push(vm, OBJ(allocateString(vm, "table", 5)));
                    break;
                case OBJ_NATIVE_METHOD:
                    msapi_push(vm, OBJ(AS_NATIVE_METHOD(thing)->name));
                    break;
                case OBJ_DLL_CONTAINER:
                    msapi_push(vm, OBJ(AS_DLL_CONTAINER(thing)->fileName));
                    break;
                case OBJ_SSOCKET:
                case OBJ_SOCKET:
                    msapi_push(vm, OBJ(allocateString(vm, "socket", 6)));
                    break;
                case OBJ_COROUTINE:
                    msapi_push(vm, OBJ(AS_COROUTINE(thing)->closure->function->name));
                    break;
                default: msapi_push(vm, NIL()); break;
            }
            break;
        }

    }

    return true;

}

bool msglobal_char(VM* vm, int argCount, bool shouldReturn) {
    if (argCount == 0) {
        msapi_runtimeError(vm, "Expected an argument in the 'char()' function");
        return false;
    }

    Value val = msapi_getArg(vm, 1, argCount);

    if (!CHECK_NUMBER(val)) {
        msapi_runtimeError(vm, "Expected an ascii numeric code");
        return false;
    }

    double number = AS_NUMBER(val);

    if ((number >= 0 && number <= 255) || (number >= -128 && number <= 128)) {
        char chars[2];
        chars[0] = (char)number;
        chars[1] = '\0';
        ObjString* character = allocateString(vm, chars, 1);
        msapi_popn(vm, argCount + 1);
        msapi_push(vm, OBJ(character));
    } else {
        msapi_runtimeError(vm, "Number out of ASCII Range");
        return false;
    }

    return true;
}

bool msglobal_num(VM* vm, int argCount, bool shouldReturn) {
    if (argCount == 0) {
        msapi_runtimeError(vm, "Expected an argument in the 'num()' function");
        return false;
    }
    Value val = msapi_getArg(vm, 1, argCount);
    msapi_popn(vm, argCount + 1);
 
    if (!shouldReturn) return true;

    if (val.type != VAL_OBJ && AS_OBJ(val)->type != OBJ_STRING) {
        msapi_push(vm, NIL());
        return true; 
    }


    char* endptr;
    double num = strtod(AS_STRING(val)->allocated, &endptr);
    
   /* strtod returns 0.0 if its not convertible, but we can allow 0 strings by
    * checking the next character of the string 
    * 
    * If the conversion was successful, the next char will always be the null terminator 
    * otherwise its a different character and we can note that 
    */

    if (*endptr != '\0') {
        msapi_push(vm, NIL());
    } else {
        msapi_push(vm, NATIVE_TO_NUMBER(num));
    }

    return true;
}

bool msglobal_input(VM* vm, int argCount, bool shouldReturn) {
    if (argCount >= 1) {
        Value value = msapi_getArg(vm, 1, argCount);
        printValue(value); 
    }
    char str[1000];
    
    if (!fgets(str, sizeof(str), stdin)) {
        printf("\n");
        msapi_runtimeError(vm, "An error occured while getting input");
        return false;
    }
    
    int len = strlen(str);
    str[len - 1] = '\0';        // replace the newline character with null terminator 

    ObjString* string = allocateString(vm, str, len - 1);
    msapi_popn(vm, argCount + 1);
    
    if (shouldReturn) {
        msapi_push(vm, OBJ(string));
    }
    return true;
}

bool msglobal_type(VM *vm, int argCount, bool shouldReturn) {
    if (argCount == 0) {
        msapi_runtimeError(vm, "Expected an argument in the 'type()' global");
        return false;
    }

    Value val = msapi_getArg(vm, 1, argCount);
    msapi_popn(vm, argCount + 1);
    
    if (!shouldReturn) return true;

    switch (val.type) {
        case VAL_NIL: 
            msapi_push(vm, OBJ(allocateString(vm, "nil", 3)));
            break;
        case VAL_BOOL: {
            msapi_push(vm, OBJ(allocateString(vm, "boolean", 7)));
            break;
        }
        case VAL_NUMBER: {
            msapi_push(vm, OBJ(allocateString(vm, "number", 6)));
            break;
        }
        case VAL_OBJ: {
            switch (AS_OBJ(val)->type) {
                case OBJ_STRING: {
                    msapi_push(vm, OBJ(allocateString(vm, "string", 6)));
                    break;
                }
                case OBJ_ARRAY: {
                    msapi_push(vm, OBJ(allocateString(vm, "array", 5)));
                    break;
                }
                case OBJ_CLASS: {
                    msapi_push(vm, OBJ(allocateString(vm, "class", 5)));
                    break;
                }
                case OBJ_INSTANCE: {
                    msapi_push(vm, OBJ(allocateString(vm, "instance", 8)));
                    break;
                }
                case OBJ_TABLE: {
                    msapi_push(vm, OBJ(allocateString(vm, "table", 5)));
                    break;
                }
                case OBJ_NATIVE_FUNCTION:
                case OBJ_CLOSURE:
                case OBJ_METHOD:
                case OBJ_NATIVE_METHOD:
                case OBJ_COROUTINE:
                case OBJ_DLL_CONTAINER: {
                    msapi_push(vm, OBJ(allocateString(vm, "function", 8)));
                    break;
                }
                case OBJ_SSOCKET:
                case OBJ_SOCKET: {
                    msapi_push(vm, OBJ(allocateString(vm, "socket", 6)));
                    break;
                }
                default: msapi_push(vm, NIL()); break;
            }
        }
    }

    return true;
}
