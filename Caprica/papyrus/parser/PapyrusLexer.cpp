#include <papyrus/parser/PapyrusLexer.h>

#include <cctype>
#include <map>
#include <unordered_map>

#include <common/CapricaConfig.h>

#include <common/CaselessStringComparer.h>

namespace caprica { namespace papyrus { namespace parser {

static const std::unordered_map<TokenType, const std::string> prettyTokenTypeNameMap{
  { TokenType::Unknown, "Unknown" },
  { TokenType::EOL, "EOL" },
  { TokenType::END, "EOF" },
  { TokenType::Identifier, "Identifier" },
  { TokenType::DocComment, "Documentation Comment" },
  { TokenType::String, "String" },
  { TokenType::Integer, "Integer" },
  { TokenType::Float, "Float" },
  { TokenType::LParen, "(" },
  { TokenType::RParen, ")" },
  { TokenType::LSquare, "[" },
  { TokenType::RSquare, "]" },
  { TokenType::Dot, "." },
  { TokenType::Comma, "," },
  { TokenType::Equal, "=" },
  { TokenType::Exclaim, "!" },
  { TokenType::Plus, "+" },
  { TokenType::PlusEqual, "+=" },
  { TokenType::Minus, "-" },
  { TokenType::MinusEqual, "-=" },
  { TokenType::Mul, "*" },
  { TokenType::MulEqual, "*=" },
  { TokenType::Div, "/" },
  { TokenType::DivEqual, "/=" },
  { TokenType::Mod, "%" },
  { TokenType::ModEqual, "%=" },
  { TokenType::CmpEq, "==" },
  { TokenType::CmpNeq, "!=" },
  { TokenType::CmpLt, "<" },
  { TokenType::CmpLte, "<=" },
  { TokenType::CmpGt, ">" },
  { TokenType::CmpGte, ">=" },
  { TokenType::BooleanOr, "||" },
  { TokenType::BooleanAnd, "&&" },
  { TokenType::kAs, "As" },
  { TokenType::kAuto, "Auto" },
  { TokenType::kAutoReadOnly, "AutoReadOnly" },
  { TokenType::kBetaOnly, "BetaOnly" },
  { TokenType::kBool, "Bool" },
  { TokenType::kConst, "Const" },
  { TokenType::kCustomEvent, "CustomEvent" },
  { TokenType::kDebugOnly, "DebugOnly" },
  { TokenType::kElse, "Else" },
  { TokenType::kElseIf, "ElseIf" },
  { TokenType::kEndEvent, "EndEvent" },
  { TokenType::kEndFunction, "EndFunction" },
  { TokenType::kEndGroup, "EndGroup" },
  { TokenType::kEndIf, "EndIf" },
  { TokenType::kEndProperty, "EndProperty" },
  { TokenType::kEndState, "EndState" },
  { TokenType::kEndStruct, "EndStruct" },
  { TokenType::kEndWhile, "EndWhile" },
  { TokenType::kEvent, "Event" },
  { TokenType::kExtends, "Extends" },
  { TokenType::kFalse, "False" },
  { TokenType::kFloat, "Float" },
  { TokenType::kFunction, "Function" },
  { TokenType::kGlobal, "Global" },
  { TokenType::kGroup, "Group" },
  { TokenType::kIf, "If" },
  { TokenType::kImport, "Import" },
  { TokenType::kInt, "Int" },
  { TokenType::kIs, "Is" },
  { TokenType::kLength, "Length" },
  { TokenType::kNative, "Native" },
  { TokenType::kNew, "New" },
  { TokenType::kNone, "None" },
  { TokenType::kParent, "Parent" },
  { TokenType::kProperty, "Property" },
  { TokenType::kReturn, "Return" },
  { TokenType::kScriptName, "ScriptName" },
  { TokenType::kSelf, "Self" },
  { TokenType::kState, "State" },
  { TokenType::kString, "String" },
  { TokenType::kStruct, "Struct" },
  { TokenType::kTrue, "True" },
  { TokenType::kVar, "Var" },
  { TokenType::kWhile, "While" },

  { TokenType::kBreak, "Break" },
  { TokenType::kCase, "Case" },
  { TokenType::kContinue, "Continue" },
  { TokenType::kDefault, "Default" },
  { TokenType::kDo, "Do" },
  { TokenType::kEndFor, "EndFor" },
  { TokenType::kEndForEach, "EndForEach" },
  { TokenType::kEndSwitch, "EndSwitch" },
  { TokenType::kFor, "For" },
  { TokenType::kForEach, "ForEach" },
  { TokenType::kIn, "In" },
  { TokenType::kLoopWhile, "LoopWhile" },
  { TokenType::kStep, "Step" },
  { TokenType::kSwitch, "Switch" },
  { TokenType::kTo, "To" },
};

const std::string PapyrusLexer::Token::prettyTokenType(TokenType tp) {
  auto f = prettyTokenTypeNameMap.find(tp);
  if (f == prettyTokenTypeNameMap.end())
    CapricaError::logicalFatal("Unable to determine the pretty form of token type %i!", (int32_t)tp);
  return f->second;
}

void PapyrusLexer::setTok(TokenType tp, const CapricaFileLocation::Partial& loc, int consumeChars) {
  cur.type = tp;
  cur.location.updatePartial(loc);
  for (int i = 0; i < consumeChars; i++)
    getChar();
}

TokenType PapyrusLexer::peekTokenType(int distance) {
  auto oldCurTp = cur.type;
  auto oldCurPLoc = CapricaFileLocation::Partial(cur.location);
  auto oldCurI = cur.iValue;
  auto oldCurF = cur.fValue;
  auto oldCurS = cur.sValue;
  auto oldLoc = CapricaFileLocation::Partial(location);
  auto oldPos = strmI;
  auto oldStrm = strm;

  for (int i = 0; i <= distance; i++)
    consume();

  auto newTokTp = cur.type;
  cur.type = oldCurTp;
  cur.location.updatePartial(oldCurPLoc);
  cur.iValue = oldCurI;
  cur.fValue = oldCurF;
  cur.sValue = oldCurS;
  location.updatePartial(oldLoc);
  strm = oldStrm;
  strmI = oldPos;
  return newTokTp;
}

static const std::map<const char* const, TokenType, CaselessStringComparer> keywordMap {
  { "as", TokenType::kAs },
  { "auto", TokenType::kAuto },
  { "autoreadonly", TokenType::kAutoReadOnly },
  { "betaonly", TokenType::kBetaOnly },
  { "bool", TokenType::kBool },
  { "const", TokenType::kConst },
  { "customevent", TokenType::kCustomEvent },
  { "debugonly", TokenType::kDebugOnly },
  { "else", TokenType::kElse },
  { "elseif", TokenType::kElseIf },
  { "endevent", TokenType::kEndEvent },
  { "endfunction", TokenType::kEndFunction },
  { "endgroup", TokenType::kEndGroup },
  { "endif", TokenType::kEndIf },
  { "endproperty", TokenType::kEndProperty },
  { "endstate", TokenType::kEndState },
  { "endstruct", TokenType::kEndStruct },
  { "endwhile", TokenType::kEndWhile },
  { "event", TokenType::kEvent },
  { "extends", TokenType::kExtends },
  { "false", TokenType::kFalse },
  { "float", TokenType::kFloat },
  { "function", TokenType::kFunction },
  { "global", TokenType::kGlobal },
  { "group", TokenType::kGroup },
  { "if", TokenType::kIf },
  { "import", TokenType::kImport },
  { "int", TokenType::kInt },
  { "is", TokenType::kIs },
  { "length", TokenType::kLength },
  { "native", TokenType::kNative },
  { "new", TokenType::kNew },
  { "none", TokenType::kNone },
  { "parent", TokenType::kParent },
  { "property", TokenType::kProperty },
  { "return", TokenType::kReturn },
  { "scriptname", TokenType::kScriptName },
  { "self", TokenType::kSelf },
  { "state", TokenType::kState },
  { "string", TokenType::kString },
  { "struct", TokenType::kStruct },
  { "true", TokenType::kTrue },
  { "var", TokenType::kVar },
  { "while", TokenType::kWhile },
};

// Language extension keywords
static const std::map<const char* const, TokenType, CaselessStringComparer> languageExtensionsKeywordMap{
  { "break", TokenType::kBreak },
  { "case", TokenType::kCase },
  { "continue", TokenType::kContinue },
  { "default", TokenType::kDefault },
  { "do", TokenType::kDo },
  { "endfor", TokenType::kEndFor },
  { "endforeach", TokenType::kEndForEach },
  { "endswitch", TokenType::kEndSwitch },
  { "for", TokenType::kFor },
  { "foreach", TokenType::kForEach },
  { "in", TokenType::kIn },
  { "loopwhile", TokenType::kLoopWhile },
  { "step", TokenType::kStep },
  { "switch", TokenType::kSwitch },
  { "to", TokenType::kTo },
};

void PapyrusLexer::consume() {
StartOver:
  auto baseLoc = CapricaFileLocation::Partial(location);
  auto c = getChar();
  
  switch (c) {
    case -1:
      return setTok(TokenType::END, baseLoc);
    case '(':
      return setTok(TokenType::LParen, baseLoc);
    case ')':
      return setTok(TokenType::RParen, baseLoc);
    case '[':
      return setTok(TokenType::LSquare, baseLoc);
    case ']':
      return setTok(TokenType::RSquare, baseLoc);
    case '.':
      return setTok(TokenType::Dot, baseLoc);
    case ',':
      return setTok(TokenType::Comma, baseLoc);

    case '=':
      if (peekChar() == '=')
        return setTok(TokenType::CmpEq, baseLoc, 1);
      return setTok(TokenType::Equal, baseLoc);
    case '!':
      if (peekChar() == '=')
        return setTok(TokenType::CmpNeq, baseLoc, 1);
      return setTok(TokenType::Exclaim, baseLoc);
    case '+':
      if (peekChar() == '=')
        return setTok(TokenType::PlusEqual, baseLoc, 1);
      return setTok(TokenType::Plus, baseLoc);
    case '-':
      if (peekChar() == '=')
        return setTok(TokenType::MinusEqual, baseLoc, 1);
      if (isdigit(peekChar()))
        goto Number;
      return setTok(TokenType::Minus, baseLoc);
    case '*':
      if (peekChar() == '=')
        return setTok(TokenType::MulEqual, baseLoc, 1);
      return setTok(TokenType::Mul, baseLoc);
    case '/':
      if (peekChar() == '=')
        return setTok(TokenType::DivEqual, baseLoc, 1);
      return setTok(TokenType::Div, baseLoc);
    case '%':
      if (peekChar() == '=')
        return setTok(TokenType::ModEqual, baseLoc, 1);
      return setTok(TokenType::Mod, baseLoc);
    case '<':
      if (peekChar() == '=')
        return setTok(TokenType::CmpLte, baseLoc, 1);
      return setTok(TokenType::CmpLt, baseLoc);
    case '>':
      if (peekChar() == '=')
        return setTok(TokenType::CmpGte, baseLoc, 1);
      return setTok(TokenType::CmpGt, baseLoc);

    case '|':
      if (peekChar() != '|')
        CapricaError::fatal(baseLoc + location, "Bitwise OR is unsupported. Did you intend to use a logical or (\"||\") instead?");
      return setTok(TokenType::BooleanOr, baseLoc, 1);
    case '&':
      if (peekChar() != '&')
        CapricaError::fatal(baseLoc + location, "Bitwise AND is unsupported. Did you intend to use a logical and (\"&&\") instead?");
      return setTok(TokenType::BooleanAnd, baseLoc, 1);

    Number:
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
    {
      std::string str;
      str.reserve(64);
      str.append(1, (char)c);

      // It's hex.
      if (c == '0' && peekChar() == 'x') {
        str.append(1, (char)getChar());
        while (isxdigit(peekChar()))
          str.append(1, (char)getChar());
        
        auto i = std::stoul(str, nullptr, 16);
        setTok(TokenType::Integer, baseLoc);
        cur.iValue = (int32_t)i;
        return;
      }

      // Either normal int or float.
      while (isdigit(peekChar()))
        str.append(1, (char)getChar());

      // It's a float.
      if (peekChar() == '.') {
        str.append(1, (char)getChar());
        while (isdigit(peekChar()))
          str.append(1, (char)getChar());

        // Allow e+ notation.
        if (CapricaConfig::enableLanguageExtensions && peekChar() == 'e') {
          str.append(1, (char)getChar());
          if (getChar() != '+')
            CapricaError::fatal(location, "Unexpected character 'e'!");
          str.append(1, '+');

          while (isdigit(peekChar()))
            str.append(1, (char)getChar());
        }

        auto f = std::stof(str);
        setTok(TokenType::Float, baseLoc);
        cur.fValue = f;
        return;
      }

      if (str.size() < 8 || (str.size() == 8 && str[0] <= '4')) {
        // It is probably an integer, but maybe not.
        try {
          auto i = std::stoul(str);
          setTok(TokenType::Integer, baseLoc);
          cur.iValue = (int32_t)i;
          return;
        } catch (std::out_of_range oor) { }
      }
      // It's very definitely a float, and a very large one at that.
      auto f = std::stof(str);
      setTok(TokenType::Float, baseLoc);
      cur.fValue = f;
      return;
    }

    case ':':
    case '_':
    case 'a':
    case 'b':
    case 'c':
    case 'd':
    case 'e':
    case 'f':
    case 'g':
    case 'h':
    case 'i':
    case 'j':
    case 'k':
    case 'l':
    case 'm':
    case 'n':
    case 'o':
    case 'p':
    case 'q':
    case 'r':
    case 's':
    case 't':
    case 'u':
    case 'v':
    case 'w':
    case 'x':
    case 'y':
    case 'z':
    case 'A':
    case 'B':
    case 'C':
    case 'D':
    case 'E':
    case 'F':
    case 'G':
    case 'H':
    case 'I':
    case 'J':
    case 'K':
    case 'L':
    case 'M':
    case 'N':
    case 'O':
    case 'P':
    case 'Q':
    case 'R':
    case 'S':
    case 'T':
    case 'U':
    case 'V':
    case 'W':
    case 'X':
    case 'Y':
    case 'Z':
    {
      std::string str;
      str.reserve(64);
      
      if (c == ':') {
        if (!CapricaConfig::allowCompilerIdentifiers || peekChar() != ':')
          CapricaError::fatal(baseLoc + location, "Unexpected character '%c'!", (char)c);
        getChar();
        str.append("::");
      } else {
        str.append(1, (char)c);
      }

      while (isalnum(peekChar()) || peekChar() == '_' || peekChar() == ':')
        str.append(1, (char)getChar());

      if (CapricaConfig::allowDecompiledStructNameRefs && peekChar() == '#') {
        str.append(1, (char)getChar());

        while (isalnum(peekChar()) || peekChar() == '_')
          str.append(1, (char)getChar());
      }

      auto f = keywordMap.find(str.c_str());
      if (f != keywordMap.end())
        return setTok(f->second, baseLoc);

      if (CapricaConfig::enableLanguageExtensions) {
        auto f2 = languageExtensionsKeywordMap.find(str.c_str());
        if (f2 != languageExtensionsKeywordMap.end())
          return setTok(f2->second, baseLoc);
      }

      setTok(TokenType::Identifier, baseLoc);
      str.shrink_to_fit();
      cur.sValue = str;
      return;
    }

    case '"':
    {
      std::string str;
      str.reserve(64);

      while (peekChar() != '"' && peekChar() != '\r' && peekChar() != '\n' && peekChar() != -1) {
        if (peekChar() == '\\') {
          getChar();
          auto escapeChar = getChar();
          switch (escapeChar) {
            case 'n':
              str.append(1, '\n');
              break;
            case 't':
              str.append(1, '\t');
              break;
            case '\\':
              str.append(1, '\\');
              break;
            case '"':
              str.append(1, '"');
              break;
            case -1:
              CapricaError::fatal(location, "Unexpected EOF before the end of the string.");
            default:
              CapricaError::fatal(location, "Unrecognized escape sequence: '\\%c'", (char)escapeChar);
          }
        } else {
          str.append(1, (char)getChar());
        }
      }

      if (peekChar() != '"')
        CapricaError::fatal(location, "Unclosed string!");
      getChar();

      setTok(TokenType::String, baseLoc);
      str.shrink_to_fit();
      cur.sValue = str;
      return;
    }

    case ';':
    {
      if (peekChar() == '/') {
        // Multiline comment.
        getChar();

        while (peekChar() != -1) {
          if (peekChar() == '\r' || peekChar() == '\n') {
            auto c2 = getChar();
            if (c2 == '\r' && peekChar() == '\n')
              getChar();
            location.nextLine();
          }

          if (getChar() == '/' && peekChar() == ';') {
            getChar();
            goto StartOver;
          }
        }

        CapricaError::fatal(location, "Unexpected EOF before the end of a multiline comment!");
      }

      // Single line comment.
      while (peekChar() != '\r' && peekChar() != '\n' && peekChar() != -1)
        getChar();
      goto StartOver;
    }

    case '{':
    {
      std::string str;
      str.reserve(64);

      // Trim all leading whitespace.
      while (isspace(peekChar()))
        getChar();

      while (peekChar() != '}' && peekChar() != -1) {
        // For sanity reasons, we only put out unix newlines in the
        // doc comment string.
        auto c2 = getChar();
        if (c2 == '\r' && peekChar() == '\n') {
          getChar();
          str.append(1, '\n');
          location.nextLine();
        } else {
          if (c2 == '\n')
            location.nextLine();
          // Whether this is a Unix newline, or a normal character,
          // we don't care, they both get written as-is.
          str.append(1, (char)c2);
        }
      }

      if (peekChar() == -1)
        CapricaError::fatal(location, "Unexpected EOF before the end of a documentation comment!");
      getChar();

      setTok(TokenType::DocComment, baseLoc);
      str.shrink_to_fit();
      cur.sValue = str;
      // Trim trailing whitespace.
      if (cur.sValue.length())
        cur.sValue = cur.sValue.substr(0, cur.sValue.find_last_not_of(" \t\n\v\f\r") + 1);
      return;
    }

    case '\\':
    {
      consume();
      if (cur.type != TokenType::EOL)
        CapricaError::fatal(baseLoc + location, "Unexpected '\\'! Division is done with a forward slash '/'.");
      goto StartOver;
    }

    case '\r':
    case '\n':
    {
      if (c == '\r' && peekChar() == '\n')
        getChar();
      location.nextLine();
      return setTok(TokenType::EOL, baseLoc);
    }

    case ' ':
    case '\t':
    {
      while (peekChar() == ' ' || peekChar() == '\t')
        getChar();
      goto StartOver;
    }

    default:
      CapricaError::fatal(baseLoc + location, "Unexpected character '%c'!", (char)c);
  }
}

}}}
