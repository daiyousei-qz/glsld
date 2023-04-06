#pragma once
// clang-format off
#include <optional>
#include <vector>
#include "SyntaxToken.h"
#include "SourceView.h"

namespace glsld::detail {
auto Tokenize(SourceScanner& srcView, std::vector<char>& buffer) -> TokenKlass {
TokenKlass acceptedKlass;
size_t acceptedSize;
SourceScanner acceptedCheckpoint;
char ch;
LexState_0:
ch = srcView.ConsumeChar();
buffer.push_back(ch);

// Initialize error token as a fallback
acceptedCheckpoint = srcView.Clone();
acceptedKlass = TokenKlass::Error;
acceptedSize = buffer.size();

switch(ch) {
    case '!': goto LexState_11;
    case '#': goto LexState_5;
    case '%': goto LexState_17;
    case '&': goto LexState_12;
    case '(': goto LexState_18;
    case ')': goto LexState_19;
    case '*': goto LexState_15;
    case '+': goto LexState_8;
    case ',': goto LexState_24;
    case '-': goto LexState_9;
    case '.': goto LexState_4;
    case '/': goto LexState_16;
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
    case ':': goto LexState_25;
    case ';': goto LexState_26;
    case '<': goto LexState_6;
    case '=': goto LexState_10;
    case '>': goto LexState_7;
    case '?': goto LexState_28;
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
    case '[': goto LexState_20;
    case ']': goto LexState_21;
    case '^': goto LexState_14;
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
    case '{': goto LexState_22;
    case '|': goto LexState_13;
    case '}': goto LexState_23;
    case '~': goto LexState_27;
    default: goto FinishToken;
}

LexState_1:
// Accepting as Identifier
acceptedCheckpoint = srcView.Clone();
acceptedKlass = static_cast<TokenKlass>(9);
acceptedSize = buffer.size();

ch = srcView.ConsumeChar();
buffer.push_back(ch);

switch(ch) {
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
acceptedCheckpoint = srcView.Clone();
acceptedKlass = static_cast<TokenKlass>(7);
acceptedSize = buffer.size();

ch = srcView.ConsumeChar();
buffer.push_back(ch);

switch(ch) {
    case '.': goto LexState_33;
    case '0': goto LexState_29;
    case '1': goto LexState_29;
    case '2': goto LexState_29;
    case '3': goto LexState_29;
    case '4': goto LexState_29;
    case '5': goto LexState_29;
    case '6': goto LexState_29;
    case '7': goto LexState_29;
    case '8': goto LexState_32;
    case '9': goto LexState_32;
    case 'E': goto LexState_34;
    case 'U': goto LexState_30;
    case 'X': goto LexState_31;
    case 'e': goto LexState_34;
    case 'u': goto LexState_30;
    case 'x': goto LexState_31;
    default: goto FinishToken;
}

LexState_3:
// Accepting as IntegerConstant
acceptedCheckpoint = srcView.Clone();
acceptedKlass = static_cast<TokenKlass>(7);
acceptedSize = buffer.size();

ch = srcView.ConsumeChar();
buffer.push_back(ch);

switch(ch) {
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
    case 'E': goto LexState_34;
    case 'U': goto LexState_30;
    case 'e': goto LexState_34;
    case 'u': goto LexState_30;
    default: goto FinishToken;
}

LexState_4:
// Accepting as Dot
acceptedCheckpoint = srcView.Clone();
acceptedKlass = static_cast<TokenKlass>(240);
acceptedSize = buffer.size();

ch = srcView.ConsumeChar();
buffer.push_back(ch);

switch(ch) {
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
// Accepting as #
acceptedCheckpoint = srcView.Clone();
acceptedKlass = static_cast<TokenKlass>(3);
acceptedSize = buffer.size();

ch = srcView.ConsumeChar();
buffer.push_back(ch);

switch(ch) {
    case '#': goto LexState_35;
    default: goto FinishToken;
}

LexState_6:
// Accepting as LAngle
acceptedCheckpoint = srcView.Clone();
acceptedKlass = static_cast<TokenKlass>(252);
acceptedSize = buffer.size();

ch = srcView.ConsumeChar();
buffer.push_back(ch);

switch(ch) {
    case '<': goto LexState_36;
    case '=': goto LexState_37;
    default: goto FinishToken;
}

LexState_7:
// Accepting as RAngle
acceptedCheckpoint = srcView.Clone();
acceptedKlass = static_cast<TokenKlass>(253);
acceptedSize = buffer.size();

ch = srcView.ConsumeChar();
buffer.push_back(ch);

switch(ch) {
    case '=': goto LexState_39;
    case '>': goto LexState_38;
    default: goto FinishToken;
}

LexState_8:
// Accepting as Plus
acceptedCheckpoint = srcView.Clone();
acceptedKlass = static_cast<TokenKlass>(248);
acceptedSize = buffer.size();

ch = srcView.ConsumeChar();
buffer.push_back(ch);

switch(ch) {
    case '+': goto LexState_40;
    case '=': goto LexState_41;
    default: goto FinishToken;
}

LexState_9:
// Accepting as Dash
acceptedCheckpoint = srcView.Clone();
acceptedKlass = static_cast<TokenKlass>(246);
acceptedSize = buffer.size();

ch = srcView.ConsumeChar();
buffer.push_back(ch);

switch(ch) {
    case '-': goto LexState_43;
    case '=': goto LexState_42;
    default: goto FinishToken;
}

LexState_10:
// Accepting as Assign
acceptedCheckpoint = srcView.Clone();
acceptedKlass = static_cast<TokenKlass>(243);
acceptedSize = buffer.size();

ch = srcView.ConsumeChar();
buffer.push_back(ch);

switch(ch) {
    case '=': goto LexState_44;
    default: goto FinishToken;
}

LexState_11:
// Accepting as Bang
acceptedCheckpoint = srcView.Clone();
acceptedKlass = static_cast<TokenKlass>(245);
acceptedSize = buffer.size();

ch = srcView.ConsumeChar();
buffer.push_back(ch);

switch(ch) {
    case '=': goto LexState_45;
    default: goto FinishToken;
}

LexState_12:
// Accepting as Ampersand
acceptedCheckpoint = srcView.Clone();
acceptedKlass = static_cast<TokenKlass>(256);
acceptedSize = buffer.size();

ch = srcView.ConsumeChar();
buffer.push_back(ch);

switch(ch) {
    case '&': goto LexState_47;
    case '=': goto LexState_46;
    default: goto FinishToken;
}

LexState_13:
// Accepting as VerticalBar
acceptedCheckpoint = srcView.Clone();
acceptedKlass = static_cast<TokenKlass>(254);
acceptedSize = buffer.size();

ch = srcView.ConsumeChar();
buffer.push_back(ch);

switch(ch) {
    case '=': goto LexState_49;
    case '|': goto LexState_48;
    default: goto FinishToken;
}

LexState_14:
// Accepting as Caret
acceptedCheckpoint = srcView.Clone();
acceptedKlass = static_cast<TokenKlass>(255);
acceptedSize = buffer.size();

ch = srcView.ConsumeChar();
buffer.push_back(ch);

switch(ch) {
    case '=': goto LexState_51;
    case '^': goto LexState_50;
    default: goto FinishToken;
}

LexState_15:
// Accepting as Star
acceptedCheckpoint = srcView.Clone();
acceptedKlass = static_cast<TokenKlass>(249);
acceptedSize = buffer.size();

ch = srcView.ConsumeChar();
buffer.push_back(ch);

switch(ch) {
    case '=': goto LexState_52;
    default: goto FinishToken;
}

LexState_16:
// Accepting as Slash
acceptedCheckpoint = srcView.Clone();
acceptedKlass = static_cast<TokenKlass>(250);
acceptedSize = buffer.size();

ch = srcView.ConsumeChar();
buffer.push_back(ch);

switch(ch) {
    case '=': goto LexState_53;
    default: goto FinishToken;
}

LexState_17:
// Accepting as Percent
acceptedCheckpoint = srcView.Clone();
acceptedKlass = static_cast<TokenKlass>(251);
acceptedSize = buffer.size();

ch = srcView.ConsumeChar();
buffer.push_back(ch);

switch(ch) {
    case '=': goto LexState_54;
    default: goto FinishToken;
}

LexState_18:
// Accepting as LParen
acceptedCheckpoint = srcView.Clone();
acceptedKlass = static_cast<TokenKlass>(234);
acceptedSize = buffer.size();

ch = srcView.ConsumeChar();
buffer.push_back(ch);

goto FinishToken;

LexState_19:
// Accepting as RParen
acceptedCheckpoint = srcView.Clone();
acceptedKlass = static_cast<TokenKlass>(235);
acceptedSize = buffer.size();

ch = srcView.ConsumeChar();
buffer.push_back(ch);

goto FinishToken;

LexState_20:
// Accepting as LBracket
acceptedCheckpoint = srcView.Clone();
acceptedKlass = static_cast<TokenKlass>(236);
acceptedSize = buffer.size();

ch = srcView.ConsumeChar();
buffer.push_back(ch);

goto FinishToken;

LexState_21:
// Accepting as RBracket
acceptedCheckpoint = srcView.Clone();
acceptedKlass = static_cast<TokenKlass>(237);
acceptedSize = buffer.size();

ch = srcView.ConsumeChar();
buffer.push_back(ch);

goto FinishToken;

LexState_22:
// Accepting as LBrace
acceptedCheckpoint = srcView.Clone();
acceptedKlass = static_cast<TokenKlass>(238);
acceptedSize = buffer.size();

ch = srcView.ConsumeChar();
buffer.push_back(ch);

goto FinishToken;

LexState_23:
// Accepting as RBrace
acceptedCheckpoint = srcView.Clone();
acceptedKlass = static_cast<TokenKlass>(239);
acceptedSize = buffer.size();

ch = srcView.ConsumeChar();
buffer.push_back(ch);

goto FinishToken;

LexState_24:
// Accepting as Comma
acceptedCheckpoint = srcView.Clone();
acceptedKlass = static_cast<TokenKlass>(241);
acceptedSize = buffer.size();

ch = srcView.ConsumeChar();
buffer.push_back(ch);

goto FinishToken;

LexState_25:
// Accepting as Colon
acceptedCheckpoint = srcView.Clone();
acceptedKlass = static_cast<TokenKlass>(242);
acceptedSize = buffer.size();

ch = srcView.ConsumeChar();
buffer.push_back(ch);

goto FinishToken;

LexState_26:
// Accepting as Semicolon
acceptedCheckpoint = srcView.Clone();
acceptedKlass = static_cast<TokenKlass>(244);
acceptedSize = buffer.size();

ch = srcView.ConsumeChar();
buffer.push_back(ch);

goto FinishToken;

LexState_27:
// Accepting as Tilde
acceptedCheckpoint = srcView.Clone();
acceptedKlass = static_cast<TokenKlass>(247);
acceptedSize = buffer.size();

ch = srcView.ConsumeChar();
buffer.push_back(ch);

goto FinishToken;

LexState_28:
// Accepting as Question
acceptedCheckpoint = srcView.Clone();
acceptedKlass = static_cast<TokenKlass>(257);
acceptedSize = buffer.size();

ch = srcView.ConsumeChar();
buffer.push_back(ch);

goto FinishToken;

LexState_29:
// Accepting as IntegerConstant
acceptedCheckpoint = srcView.Clone();
acceptedKlass = static_cast<TokenKlass>(7);
acceptedSize = buffer.size();

ch = srcView.ConsumeChar();
buffer.push_back(ch);

switch(ch) {
    case '.': goto LexState_33;
    case '0': goto LexState_29;
    case '1': goto LexState_29;
    case '2': goto LexState_29;
    case '3': goto LexState_29;
    case '4': goto LexState_29;
    case '5': goto LexState_29;
    case '6': goto LexState_29;
    case '7': goto LexState_29;
    case '8': goto LexState_32;
    case '9': goto LexState_32;
    case 'E': goto LexState_34;
    case 'U': goto LexState_30;
    case 'e': goto LexState_34;
    case 'u': goto LexState_30;
    default: goto FinishToken;
}

LexState_30:
// Accepting as IntegerConstant
acceptedCheckpoint = srcView.Clone();
acceptedKlass = static_cast<TokenKlass>(7);
acceptedSize = buffer.size();

ch = srcView.ConsumeChar();
buffer.push_back(ch);

goto FinishToken;

LexState_31:
// Accepting as IntegerConstant
acceptedCheckpoint = srcView.Clone();
acceptedKlass = static_cast<TokenKlass>(7);
acceptedSize = buffer.size();

ch = srcView.ConsumeChar();
buffer.push_back(ch);

switch(ch) {
    case '0': goto LexState_31;
    case '1': goto LexState_31;
    case '2': goto LexState_31;
    case '3': goto LexState_31;
    case '4': goto LexState_31;
    case '5': goto LexState_31;
    case '6': goto LexState_31;
    case '7': goto LexState_31;
    case '8': goto LexState_31;
    case '9': goto LexState_31;
    case 'A': goto LexState_31;
    case 'B': goto LexState_31;
    case 'C': goto LexState_31;
    case 'D': goto LexState_31;
    case 'E': goto LexState_31;
    case 'F': goto LexState_31;
    case 'U': goto LexState_30;
    case 'a': goto LexState_31;
    case 'b': goto LexState_31;
    case 'c': goto LexState_31;
    case 'd': goto LexState_31;
    case 'e': goto LexState_31;
    case 'f': goto LexState_31;
    case 'u': goto LexState_30;
    default: goto FinishToken;
}

LexState_32:
ch = srcView.ConsumeChar();
buffer.push_back(ch);

switch(ch) {
    case '.': goto LexState_33;
    case '0': goto LexState_32;
    case '1': goto LexState_32;
    case '2': goto LexState_32;
    case '3': goto LexState_32;
    case '4': goto LexState_32;
    case '5': goto LexState_32;
    case '6': goto LexState_32;
    case '7': goto LexState_32;
    case '8': goto LexState_32;
    case '9': goto LexState_32;
    case 'E': goto LexState_34;
    case 'e': goto LexState_34;
    default: goto FinishToken;
}

LexState_33:
// Accepting as FloatConstant
acceptedCheckpoint = srcView.Clone();
acceptedKlass = static_cast<TokenKlass>(8);
acceptedSize = buffer.size();

ch = srcView.ConsumeChar();
buffer.push_back(ch);

switch(ch) {
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
    case 'E': goto LexState_34;
    case 'F': goto LexState_55;
    case 'L': goto LexState_57;
    case 'e': goto LexState_34;
    case 'f': goto LexState_55;
    case 'l': goto LexState_56;
    default: goto FinishToken;
}

LexState_34:
ch = srcView.ConsumeChar();
buffer.push_back(ch);

switch(ch) {
    case '+': goto LexState_59;
    case '-': goto LexState_59;
    case '0': goto LexState_58;
    case '1': goto LexState_58;
    case '2': goto LexState_58;
    case '3': goto LexState_58;
    case '4': goto LexState_58;
    case '5': goto LexState_58;
    case '6': goto LexState_58;
    case '7': goto LexState_58;
    case '8': goto LexState_58;
    case '9': goto LexState_58;
    default: goto FinishToken;
}

LexState_35:
// Accepting as ##
acceptedCheckpoint = srcView.Clone();
acceptedKlass = static_cast<TokenKlass>(4);
acceptedSize = buffer.size();

ch = srcView.ConsumeChar();
buffer.push_back(ch);

goto FinishToken;

LexState_36:
// Accepting as LShift
acceptedCheckpoint = srcView.Clone();
acceptedKlass = static_cast<TokenKlass>(213);
acceptedSize = buffer.size();

ch = srcView.ConsumeChar();
buffer.push_back(ch);

switch(ch) {
    case '=': goto LexState_60;
    default: goto FinishToken;
}

LexState_37:
// Accepting as LessEq
acceptedCheckpoint = srcView.Clone();
acceptedKlass = static_cast<TokenKlass>(217);
acceptedSize = buffer.size();

ch = srcView.ConsumeChar();
buffer.push_back(ch);

goto FinishToken;

LexState_38:
// Accepting as RShift
acceptedCheckpoint = srcView.Clone();
acceptedKlass = static_cast<TokenKlass>(214);
acceptedSize = buffer.size();

ch = srcView.ConsumeChar();
buffer.push_back(ch);

switch(ch) {
    case '=': goto LexState_61;
    default: goto FinishToken;
}

LexState_39:
// Accepting as GreaterEq
acceptedCheckpoint = srcView.Clone();
acceptedKlass = static_cast<TokenKlass>(218);
acceptedSize = buffer.size();

ch = srcView.ConsumeChar();
buffer.push_back(ch);

goto FinishToken;

LexState_40:
// Accepting as Increment
acceptedCheckpoint = srcView.Clone();
acceptedKlass = static_cast<TokenKlass>(215);
acceptedSize = buffer.size();

ch = srcView.ConsumeChar();
buffer.push_back(ch);

goto FinishToken;

LexState_41:
// Accepting as AddAssign
acceptedCheckpoint = srcView.Clone();
acceptedKlass = static_cast<TokenKlass>(226);
acceptedSize = buffer.size();

ch = srcView.ConsumeChar();
buffer.push_back(ch);

goto FinishToken;

LexState_42:
// Accepting as SubAssign
acceptedCheckpoint = srcView.Clone();
acceptedKlass = static_cast<TokenKlass>(233);
acceptedSize = buffer.size();

ch = srcView.ConsumeChar();
buffer.push_back(ch);

goto FinishToken;

LexState_43:
// Accepting as Decrement
acceptedCheckpoint = srcView.Clone();
acceptedKlass = static_cast<TokenKlass>(216);
acceptedSize = buffer.size();

ch = srcView.ConsumeChar();
buffer.push_back(ch);

goto FinishToken;

LexState_44:
// Accepting as Equal
acceptedCheckpoint = srcView.Clone();
acceptedKlass = static_cast<TokenKlass>(219);
acceptedSize = buffer.size();

ch = srcView.ConsumeChar();
buffer.push_back(ch);

goto FinishToken;

LexState_45:
// Accepting as NotEqual
acceptedCheckpoint = srcView.Clone();
acceptedKlass = static_cast<TokenKlass>(220);
acceptedSize = buffer.size();

ch = srcView.ConsumeChar();
buffer.push_back(ch);

goto FinishToken;

LexState_46:
// Accepting as AndAssign
acceptedCheckpoint = srcView.Clone();
acceptedKlass = static_cast<TokenKlass>(230);
acceptedSize = buffer.size();

ch = srcView.ConsumeChar();
buffer.push_back(ch);

goto FinishToken;

LexState_47:
// Accepting as And
acceptedCheckpoint = srcView.Clone();
acceptedKlass = static_cast<TokenKlass>(221);
acceptedSize = buffer.size();

ch = srcView.ConsumeChar();
buffer.push_back(ch);

goto FinishToken;

LexState_48:
// Accepting as Or
acceptedCheckpoint = srcView.Clone();
acceptedKlass = static_cast<TokenKlass>(222);
acceptedSize = buffer.size();

ch = srcView.ConsumeChar();
buffer.push_back(ch);

goto FinishToken;

LexState_49:
// Accepting as OrAssign
acceptedCheckpoint = srcView.Clone();
acceptedKlass = static_cast<TokenKlass>(232);
acceptedSize = buffer.size();

ch = srcView.ConsumeChar();
buffer.push_back(ch);

goto FinishToken;

LexState_50:
// Accepting as Xor
acceptedCheckpoint = srcView.Clone();
acceptedKlass = static_cast<TokenKlass>(223);
acceptedSize = buffer.size();

ch = srcView.ConsumeChar();
buffer.push_back(ch);

goto FinishToken;

LexState_51:
// Accepting as XorAssign
acceptedCheckpoint = srcView.Clone();
acceptedKlass = static_cast<TokenKlass>(231);
acceptedSize = buffer.size();

ch = srcView.ConsumeChar();
buffer.push_back(ch);

goto FinishToken;

LexState_52:
// Accepting as MulAssign
acceptedCheckpoint = srcView.Clone();
acceptedKlass = static_cast<TokenKlass>(224);
acceptedSize = buffer.size();

ch = srcView.ConsumeChar();
buffer.push_back(ch);

goto FinishToken;

LexState_53:
// Accepting as DivAssign
acceptedCheckpoint = srcView.Clone();
acceptedKlass = static_cast<TokenKlass>(225);
acceptedSize = buffer.size();

ch = srcView.ConsumeChar();
buffer.push_back(ch);

goto FinishToken;

LexState_54:
// Accepting as ModAssign
acceptedCheckpoint = srcView.Clone();
acceptedKlass = static_cast<TokenKlass>(227);
acceptedSize = buffer.size();

ch = srcView.ConsumeChar();
buffer.push_back(ch);

goto FinishToken;

LexState_55:
// Accepting as FloatConstant
acceptedCheckpoint = srcView.Clone();
acceptedKlass = static_cast<TokenKlass>(8);
acceptedSize = buffer.size();

ch = srcView.ConsumeChar();
buffer.push_back(ch);

goto FinishToken;

LexState_56:
ch = srcView.ConsumeChar();
buffer.push_back(ch);

switch(ch) {
    case 'f': goto LexState_55;
    default: goto FinishToken;
}

LexState_57:
ch = srcView.ConsumeChar();
buffer.push_back(ch);

switch(ch) {
    case 'F': goto LexState_55;
    default: goto FinishToken;
}

LexState_58:
// Accepting as FloatConstant
acceptedCheckpoint = srcView.Clone();
acceptedKlass = static_cast<TokenKlass>(8);
acceptedSize = buffer.size();

ch = srcView.ConsumeChar();
buffer.push_back(ch);

switch(ch) {
    case '0': goto LexState_58;
    case '1': goto LexState_58;
    case '2': goto LexState_58;
    case '3': goto LexState_58;
    case '4': goto LexState_58;
    case '5': goto LexState_58;
    case '6': goto LexState_58;
    case '7': goto LexState_58;
    case '8': goto LexState_58;
    case '9': goto LexState_58;
    case 'F': goto LexState_55;
    case 'L': goto LexState_57;
    case 'f': goto LexState_55;
    case 'l': goto LexState_56;
    default: goto FinishToken;
}

LexState_59:
ch = srcView.ConsumeChar();
buffer.push_back(ch);

switch(ch) {
    case '0': goto LexState_58;
    case '1': goto LexState_58;
    case '2': goto LexState_58;
    case '3': goto LexState_58;
    case '4': goto LexState_58;
    case '5': goto LexState_58;
    case '6': goto LexState_58;
    case '7': goto LexState_58;
    case '8': goto LexState_58;
    case '9': goto LexState_58;
    default: goto FinishToken;
}

LexState_60:
// Accepting as LShiftAssign
acceptedCheckpoint = srcView.Clone();
acceptedKlass = static_cast<TokenKlass>(228);
acceptedSize = buffer.size();

ch = srcView.ConsumeChar();
buffer.push_back(ch);

goto FinishToken;

LexState_61:
// Accepting as RShiftAssign
acceptedCheckpoint = srcView.Clone();
acceptedKlass = static_cast<TokenKlass>(229);
acceptedSize = buffer.size();

ch = srcView.ConsumeChar();
buffer.push_back(ch);

goto FinishToken;

FinishToken:
srcView.Restore(acceptedCheckpoint);
buffer.resize(acceptedSize);
return acceptedKlass;
}
}
// clang-format on
