#include <string_view>

// clang-format off
struct TokenizeResult {
    int acceptedKlass;
    int numAcceptedChar;
};
auto Tokenize(std::string_view remainingSource) -> TokenizeResult {
    int index = 0;
    int numAcceptedChar = 0;
    int acceptedKlass = -1;
LexState_0:
if (index == remainingSource.size()) {
    goto FinishToken;
}
switch(remainingSource[index++]) {
    case '!': goto LexState_10;
    case '%': goto LexState_16;
    case '&': goto LexState_11;
    case '(': goto LexState_17;
    case ')': goto LexState_18;
    case '*': goto LexState_14;
    case '+': goto LexState_7;
    case ',': goto LexState_23;
    case '-': goto LexState_8;
    case '.': goto LexState_4;
    case '/': goto LexState_15;
    case '0': goto LexState_2;
    case '1': goto LexState_3;
    case '2': goto LexState_3;
    case '3': goto LexState_3;
    case '4': goto LexState_3;
    case '5': goto LexState_3;
    case '6': goto LexState_3;
    case '7': goto LexState_3;
    case '8': goto LexState_3;
    case '9': goto LexState_3;
    case ':': goto LexState_24;
    case ';': goto LexState_25;
    case '<': goto LexState_5;
    case '=': goto LexState_9;
    case '>': goto LexState_6;
    case '?': goto LexState_27;
    case 'A': goto LexState_1;
    case 'B': goto LexState_1;
    case 'C': goto LexState_1;
    case 'D': goto LexState_1;
    case 'E': goto LexState_1;
    case 'F': goto LexState_1;
    case 'G': goto LexState_1;
    case 'H': goto LexState_1;
    case 'I': goto LexState_1;
    case 'J': goto LexState_1;
    case 'K': goto LexState_1;
    case 'L': goto LexState_1;
    case 'M': goto LexState_1;
    case 'N': goto LexState_1;
    case 'O': goto LexState_1;
    case 'P': goto LexState_1;
    case 'Q': goto LexState_1;
    case 'R': goto LexState_1;
    case 'S': goto LexState_1;
    case 'T': goto LexState_1;
    case 'U': goto LexState_1;
    case 'V': goto LexState_1;
    case 'W': goto LexState_1;
    case 'X': goto LexState_1;
    case 'Y': goto LexState_1;
    case 'Z': goto LexState_1;
    case '[': goto LexState_19;
    case ']': goto LexState_20;
    case '^': goto LexState_13;
    case '_': goto LexState_1;
    case 'a': goto LexState_1;
    case 'b': goto LexState_1;
    case 'c': goto LexState_1;
    case 'd': goto LexState_1;
    case 'e': goto LexState_1;
    case 'f': goto LexState_1;
    case 'g': goto LexState_1;
    case 'h': goto LexState_1;
    case 'i': goto LexState_1;
    case 'j': goto LexState_1;
    case 'k': goto LexState_1;
    case 'l': goto LexState_1;
    case 'm': goto LexState_1;
    case 'n': goto LexState_1;
    case 'o': goto LexState_1;
    case 'p': goto LexState_1;
    case 'q': goto LexState_1;
    case 'r': goto LexState_1;
    case 's': goto LexState_1;
    case 't': goto LexState_1;
    case 'u': goto LexState_1;
    case 'v': goto LexState_1;
    case 'w': goto LexState_1;
    case 'x': goto LexState_1;
    case 'y': goto LexState_1;
    case 'z': goto LexState_1;
    case '{': goto LexState_21;
    case '|': goto LexState_12;
    case '}': goto LexState_22;
    case '~': goto LexState_26;
    default: goto FinishToken;
}

LexState_1:
// Accepting as Identifier
numAcceptedChar = index;
acceptedKlass = 5;
if (index == remainingSource.size()) {
    goto FinishToken;
}
switch(remainingSource[index++]) {
    case '0': goto LexState_1;
    case '1': goto LexState_1;
    case '2': goto LexState_1;
    case '3': goto LexState_1;
    case '4': goto LexState_1;
    case '5': goto LexState_1;
    case '6': goto LexState_1;
    case '7': goto LexState_1;
    case '8': goto LexState_1;
    case '9': goto LexState_1;
    case 'A': goto LexState_1;
    case 'B': goto LexState_1;
    case 'C': goto LexState_1;
    case 'D': goto LexState_1;
    case 'E': goto LexState_1;
    case 'F': goto LexState_1;
    case 'G': goto LexState_1;
    case 'H': goto LexState_1;
    case 'I': goto LexState_1;
    case 'J': goto LexState_1;
    case 'K': goto LexState_1;
    case 'L': goto LexState_1;
    case 'M': goto LexState_1;
    case 'N': goto LexState_1;
    case 'O': goto LexState_1;
    case 'P': goto LexState_1;
    case 'Q': goto LexState_1;
    case 'R': goto LexState_1;
    case 'S': goto LexState_1;
    case 'T': goto LexState_1;
    case 'U': goto LexState_1;
    case 'V': goto LexState_1;
    case 'W': goto LexState_1;
    case 'X': goto LexState_1;
    case 'Y': goto LexState_1;
    case 'Z': goto LexState_1;
    case '_': goto LexState_1;
    case 'a': goto LexState_1;
    case 'b': goto LexState_1;
    case 'c': goto LexState_1;
    case 'd': goto LexState_1;
    case 'e': goto LexState_1;
    case 'f': goto LexState_1;
    case 'g': goto LexState_1;
    case 'h': goto LexState_1;
    case 'i': goto LexState_1;
    case 'j': goto LexState_1;
    case 'k': goto LexState_1;
    case 'l': goto LexState_1;
    case 'm': goto LexState_1;
    case 'n': goto LexState_1;
    case 'o': goto LexState_1;
    case 'p': goto LexState_1;
    case 'q': goto LexState_1;
    case 'r': goto LexState_1;
    case 's': goto LexState_1;
    case 't': goto LexState_1;
    case 'u': goto LexState_1;
    case 'v': goto LexState_1;
    case 'w': goto LexState_1;
    case 'x': goto LexState_1;
    case 'y': goto LexState_1;
    case 'z': goto LexState_1;
    default: goto FinishToken;
}

LexState_2:
// Accepting as IntegerConstant
numAcceptedChar = index;
acceptedKlass = 3;
if (index == remainingSource.size()) {
    goto FinishToken;
}
switch(remainingSource[index++]) {
    case '.': goto LexState_33;
    case '0': goto LexState_28;
    case '1': goto LexState_28;
    case '2': goto LexState_28;
    case '3': goto LexState_28;
    case '4': goto LexState_28;
    case '5': goto LexState_28;
    case '6': goto LexState_28;
    case '7': goto LexState_28;
    case '8': goto LexState_30;
    case '9': goto LexState_30;
    case 'E': goto LexState_32;
    case 'U': goto LexState_31;
    case 'X': goto LexState_29;
    case 'e': goto LexState_32;
    case 'u': goto LexState_31;
    case 'x': goto LexState_29;
    default: goto FinishToken;
}

LexState_3:
// Accepting as IntegerConstant
numAcceptedChar = index;
acceptedKlass = 3;
if (index == remainingSource.size()) {
    goto FinishToken;
}
switch(remainingSource[index++]) {
    case '.': goto LexState_33;
    case '0': goto LexState_3;
    case '1': goto LexState_3;
    case '2': goto LexState_3;
    case '3': goto LexState_3;
    case '4': goto LexState_3;
    case '5': goto LexState_3;
    case '6': goto LexState_3;
    case '7': goto LexState_3;
    case '8': goto LexState_3;
    case '9': goto LexState_3;
    case 'E': goto LexState_32;
    case 'U': goto LexState_31;
    case 'e': goto LexState_32;
    case 'u': goto LexState_31;
    default: goto FinishToken;
}

LexState_4:
// Accepting as Dot
numAcceptedChar = index;
acceptedKlass = 236;
if (index == remainingSource.size()) {
    goto FinishToken;
}
switch(remainingSource[index++]) {
    case '0': goto LexState_33;
    case '1': goto LexState_33;
    case '2': goto LexState_33;
    case '3': goto LexState_33;
    case '4': goto LexState_33;
    case '5': goto LexState_33;
    case '6': goto LexState_33;
    case '7': goto LexState_33;
    case '8': goto LexState_33;
    case '9': goto LexState_33;
    default: goto FinishToken;
}

LexState_5:
// Accepting as LAngle
numAcceptedChar = index;
acceptedKlass = 248;
if (index == remainingSource.size()) {
    goto FinishToken;
}
switch(remainingSource[index++]) {
    case '<': goto LexState_34;
    case '=': goto LexState_35;
    default: goto FinishToken;
}

LexState_6:
// Accepting as RAngle
numAcceptedChar = index;
acceptedKlass = 249;
if (index == remainingSource.size()) {
    goto FinishToken;
}
switch(remainingSource[index++]) {
    case '=': goto LexState_37;
    case '>': goto LexState_36;
    default: goto FinishToken;
}

LexState_7:
// Accepting as Plus
numAcceptedChar = index;
acceptedKlass = 244;
if (index == remainingSource.size()) {
    goto FinishToken;
}
switch(remainingSource[index++]) {
    case '+': goto LexState_38;
    case '=': goto LexState_39;
    default: goto FinishToken;
}

LexState_8:
// Accepting as Dash
numAcceptedChar = index;
acceptedKlass = 242;
if (index == remainingSource.size()) {
    goto FinishToken;
}
switch(remainingSource[index++]) {
    case '-': goto LexState_41;
    case '=': goto LexState_40;
    default: goto FinishToken;
}

LexState_9:
// Accepting as Assign
numAcceptedChar = index;
acceptedKlass = 239;
if (index == remainingSource.size()) {
    goto FinishToken;
}
switch(remainingSource[index++]) {
    case '=': goto LexState_42;
    default: goto FinishToken;
}

LexState_10:
// Accepting as Bang
numAcceptedChar = index;
acceptedKlass = 241;
if (index == remainingSource.size()) {
    goto FinishToken;
}
switch(remainingSource[index++]) {
    case '=': goto LexState_43;
    default: goto FinishToken;
}

LexState_11:
// Accepting as Ampersand
numAcceptedChar = index;
acceptedKlass = 252;
if (index == remainingSource.size()) {
    goto FinishToken;
}
switch(remainingSource[index++]) {
    case '&': goto LexState_45;
    case '=': goto LexState_44;
    default: goto FinishToken;
}

LexState_12:
// Accepting as VerticalBar
numAcceptedChar = index;
acceptedKlass = 250;
if (index == remainingSource.size()) {
    goto FinishToken;
}
switch(remainingSource[index++]) {
    case '=': goto LexState_47;
    case '|': goto LexState_46;
    default: goto FinishToken;
}

LexState_13:
// Accepting as Caret
numAcceptedChar = index;
acceptedKlass = 251;
if (index == remainingSource.size()) {
    goto FinishToken;
}
switch(remainingSource[index++]) {
    case '=': goto LexState_48;
    case '^': goto LexState_49;
    default: goto FinishToken;
}

LexState_14:
// Accepting as Star
numAcceptedChar = index;
acceptedKlass = 245;
if (index == remainingSource.size()) {
    goto FinishToken;
}
switch(remainingSource[index++]) {
    case '=': goto LexState_50;
    default: goto FinishToken;
}

LexState_15:
// Accepting as Slash
numAcceptedChar = index;
acceptedKlass = 246;
if (index == remainingSource.size()) {
    goto FinishToken;
}
switch(remainingSource[index++]) {
    case '=': goto LexState_51;
    default: goto FinishToken;
}

LexState_16:
// Accepting as Percent
numAcceptedChar = index;
acceptedKlass = 247;
if (index == remainingSource.size()) {
    goto FinishToken;
}
switch(remainingSource[index++]) {
    case '=': goto LexState_52;
    default: goto FinishToken;
}

LexState_17:
// Accepting as LParen
numAcceptedChar = index;
acceptedKlass = 230;
goto FinishToken;

LexState_18:
// Accepting as RParen
numAcceptedChar = index;
acceptedKlass = 231;
goto FinishToken;

LexState_19:
// Accepting as LBracket
numAcceptedChar = index;
acceptedKlass = 232;
goto FinishToken;

LexState_20:
// Accepting as RBracket
numAcceptedChar = index;
acceptedKlass = 233;
goto FinishToken;

LexState_21:
// Accepting as LBrace
numAcceptedChar = index;
acceptedKlass = 234;
goto FinishToken;

LexState_22:
// Accepting as RBrace
numAcceptedChar = index;
acceptedKlass = 235;
goto FinishToken;

LexState_23:
// Accepting as Comma
numAcceptedChar = index;
acceptedKlass = 237;
goto FinishToken;

LexState_24:
// Accepting as Colon
numAcceptedChar = index;
acceptedKlass = 238;
goto FinishToken;

LexState_25:
// Accepting as Semicolon
numAcceptedChar = index;
acceptedKlass = 240;
goto FinishToken;

LexState_26:
// Accepting as Tilde
numAcceptedChar = index;
acceptedKlass = 243;
goto FinishToken;

LexState_27:
// Accepting as Question
numAcceptedChar = index;
acceptedKlass = 253;
goto FinishToken;

LexState_28:
// Accepting as IntegerConstant
numAcceptedChar = index;
acceptedKlass = 3;
if (index == remainingSource.size()) {
    goto FinishToken;
}
switch(remainingSource[index++]) {
    case '.': goto LexState_33;
    case '0': goto LexState_28;
    case '1': goto LexState_28;
    case '2': goto LexState_28;
    case '3': goto LexState_28;
    case '4': goto LexState_28;
    case '5': goto LexState_28;
    case '6': goto LexState_28;
    case '7': goto LexState_28;
    case '8': goto LexState_30;
    case '9': goto LexState_30;
    case 'E': goto LexState_32;
    case 'U': goto LexState_31;
    case 'e': goto LexState_32;
    case 'u': goto LexState_31;
    default: goto FinishToken;
}

LexState_29:
// Accepting as IntegerConstant
numAcceptedChar = index;
acceptedKlass = 3;
if (index == remainingSource.size()) {
    goto FinishToken;
}
switch(remainingSource[index++]) {
    case '0': goto LexState_29;
    case '1': goto LexState_29;
    case '2': goto LexState_29;
    case '3': goto LexState_29;
    case '4': goto LexState_29;
    case '5': goto LexState_29;
    case '6': goto LexState_29;
    case '7': goto LexState_29;
    case '8': goto LexState_29;
    case '9': goto LexState_29;
    case 'A': goto LexState_29;
    case 'B': goto LexState_29;
    case 'C': goto LexState_29;
    case 'D': goto LexState_29;
    case 'E': goto LexState_29;
    case 'F': goto LexState_29;
    case 'U': goto LexState_31;
    case 'a': goto LexState_29;
    case 'b': goto LexState_29;
    case 'c': goto LexState_29;
    case 'd': goto LexState_29;
    case 'e': goto LexState_29;
    case 'f': goto LexState_29;
    case 'u': goto LexState_31;
    default: goto FinishToken;
}

LexState_30:
if (index == remainingSource.size()) {
    goto FinishToken;
}
switch(remainingSource[index++]) {
    case '.': goto LexState_33;
    case '0': goto LexState_30;
    case '1': goto LexState_30;
    case '2': goto LexState_30;
    case '3': goto LexState_30;
    case '4': goto LexState_30;
    case '5': goto LexState_30;
    case '6': goto LexState_30;
    case '7': goto LexState_30;
    case '8': goto LexState_30;
    case '9': goto LexState_30;
    case 'E': goto LexState_32;
    case 'e': goto LexState_32;
    default: goto FinishToken;
}

LexState_31:
// Accepting as IntegerConstant
numAcceptedChar = index;
acceptedKlass = 3;
goto FinishToken;

LexState_32:
if (index == remainingSource.size()) {
    goto FinishToken;
}
switch(remainingSource[index++]) {
    case '+': goto LexState_54;
    case '-': goto LexState_54;
    case '0': goto LexState_53;
    case '1': goto LexState_53;
    case '2': goto LexState_53;
    case '3': goto LexState_53;
    case '4': goto LexState_53;
    case '5': goto LexState_53;
    case '6': goto LexState_53;
    case '7': goto LexState_53;
    case '8': goto LexState_53;
    case '9': goto LexState_53;
    default: goto FinishToken;
}

LexState_33:
// Accepting as FloatConstant
numAcceptedChar = index;
acceptedKlass = 4;
if (index == remainingSource.size()) {
    goto FinishToken;
}
switch(remainingSource[index++]) {
    case '0': goto LexState_33;
    case '1': goto LexState_33;
    case '2': goto LexState_33;
    case '3': goto LexState_33;
    case '4': goto LexState_33;
    case '5': goto LexState_33;
    case '6': goto LexState_33;
    case '7': goto LexState_33;
    case '8': goto LexState_33;
    case '9': goto LexState_33;
    case 'E': goto LexState_32;
    case 'F': goto LexState_55;
    case 'L': goto LexState_57;
    case 'e': goto LexState_32;
    case 'f': goto LexState_55;
    case 'l': goto LexState_56;
    default: goto FinishToken;
}

LexState_34:
// Accepting as LShift
numAcceptedChar = index;
acceptedKlass = 209;
if (index == remainingSource.size()) {
    goto FinishToken;
}
switch(remainingSource[index++]) {
    case '=': goto LexState_58;
    default: goto FinishToken;
}

LexState_35:
// Accepting as LessEq
numAcceptedChar = index;
acceptedKlass = 213;
goto FinishToken;

LexState_36:
// Accepting as RShift
numAcceptedChar = index;
acceptedKlass = 210;
if (index == remainingSource.size()) {
    goto FinishToken;
}
switch(remainingSource[index++]) {
    case '=': goto LexState_59;
    default: goto FinishToken;
}

LexState_37:
// Accepting as GreaterEq
numAcceptedChar = index;
acceptedKlass = 214;
goto FinishToken;

LexState_38:
// Accepting as Increment
numAcceptedChar = index;
acceptedKlass = 211;
goto FinishToken;

LexState_39:
// Accepting as AddAssign
numAcceptedChar = index;
acceptedKlass = 222;
goto FinishToken;

LexState_40:
// Accepting as SubAssign
numAcceptedChar = index;
acceptedKlass = 229;
goto FinishToken;

LexState_41:
// Accepting as Decrement
numAcceptedChar = index;
acceptedKlass = 212;
goto FinishToken;

LexState_42:
// Accepting as Equal
numAcceptedChar = index;
acceptedKlass = 215;
goto FinishToken;

LexState_43:
// Accepting as NotEqual
numAcceptedChar = index;
acceptedKlass = 216;
goto FinishToken;

LexState_44:
// Accepting as AndAssign
numAcceptedChar = index;
acceptedKlass = 226;
goto FinishToken;

LexState_45:
// Accepting as And
numAcceptedChar = index;
acceptedKlass = 217;
goto FinishToken;

LexState_46:
// Accepting as Or
numAcceptedChar = index;
acceptedKlass = 218;
goto FinishToken;

LexState_47:
// Accepting as OrAssign
numAcceptedChar = index;
acceptedKlass = 228;
goto FinishToken;

LexState_48:
// Accepting as XorAssign
numAcceptedChar = index;
acceptedKlass = 227;
goto FinishToken;

LexState_49:
// Accepting as Xor
numAcceptedChar = index;
acceptedKlass = 219;
goto FinishToken;

LexState_50:
// Accepting as MulAssign
numAcceptedChar = index;
acceptedKlass = 220;
goto FinishToken;

LexState_51:
// Accepting as DivAssign
numAcceptedChar = index;
acceptedKlass = 221;
goto FinishToken;

LexState_52:
// Accepting as ModAssign
numAcceptedChar = index;
acceptedKlass = 223;
goto FinishToken;

LexState_53:
// Accepting as FloatConstant
numAcceptedChar = index;
acceptedKlass = 4;
if (index == remainingSource.size()) {
    goto FinishToken;
}
switch(remainingSource[index++]) {
    case '0': goto LexState_53;
    case '1': goto LexState_53;
    case '2': goto LexState_53;
    case '3': goto LexState_53;
    case '4': goto LexState_53;
    case '5': goto LexState_53;
    case '6': goto LexState_53;
    case '7': goto LexState_53;
    case '8': goto LexState_53;
    case '9': goto LexState_53;
    case 'F': goto LexState_55;
    case 'L': goto LexState_57;
    case 'f': goto LexState_55;
    case 'l': goto LexState_56;
    default: goto FinishToken;
}

LexState_54:
if (index == remainingSource.size()) {
    goto FinishToken;
}
switch(remainingSource[index++]) {
    case '0': goto LexState_53;
    case '1': goto LexState_53;
    case '2': goto LexState_53;
    case '3': goto LexState_53;
    case '4': goto LexState_53;
    case '5': goto LexState_53;
    case '6': goto LexState_53;
    case '7': goto LexState_53;
    case '8': goto LexState_53;
    case '9': goto LexState_53;
    default: goto FinishToken;
}

LexState_55:
// Accepting as FloatConstant
numAcceptedChar = index;
acceptedKlass = 4;
goto FinishToken;

LexState_56:
if (index == remainingSource.size()) {
    goto FinishToken;
}
switch(remainingSource[index++]) {
    case 'f': goto LexState_55;
    default: goto FinishToken;
}

LexState_57:
if (index == remainingSource.size()) {
    goto FinishToken;
}
switch(remainingSource[index++]) {
    case 'F': goto LexState_55;
    default: goto FinishToken;
}

LexState_58:
// Accepting as LShiftAssign
numAcceptedChar = index;
acceptedKlass = 224;
goto FinishToken;

LexState_59:
// Accepting as RShiftAssign
numAcceptedChar = index;
acceptedKlass = 225;
goto FinishToken;

FinishToken:
return {acceptedKlass, numAcceptedChar};
}
// clang-format on
