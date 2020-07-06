#ifndef __PXX_COMMENT_PARSER_H__
#define __PXX_COMMENT_PARSER_H__

#include <peglib/peglib.h>

#include <sstream>
#include <string>
#include <vector>

namespace pxx::ast {

void initialize_parser();

using InstanceString = std::pair<std::string, std::vector<std::string>>;

////////////////////////////////////////////////////////////////////////////////
// Export settings
////////////////////////////////////////////////////////////////////////////////

struct ExportSettings {
  bool exp = false;
  std::vector<InstanceString> instance_strings = {};

  ExportSettings& operator+=(const ExportSettings& other) {
    exp |= other.exp;
    instance_strings.insert(instance_strings.end(),
                            other.instance_strings.begin(),
                            other.instance_strings.end());
    return *this;
  }
};

std::ostream& operator<<(std::ostream& stream, ExportSettings settings) {
  stream << "Export settings :: " << std::endl;
  ;
  stream << "  export = " << settings.exp << std::endl;
  ;
  stream << "  instance_strings = ";
  for (auto& is : settings.instance_strings) {
    stream << "    " << std::get<0>(is) << " : ";
    auto v = std::get<1>(is);
    for (auto& s : v) {
      stream << s << ", ";
    }
  }
  stream << std::endl << std::endl;
  return stream;
}

////////////////////////////////////////////////////////////////////////////////
// Comment parser
////////////////////////////////////////////////////////////////////////////////

struct CommentParser {
  CommentParser(std::string comment,
                ExportSettings default_settings = ExportSettings())
      : settings(default_settings) {
    if (!CommentParser::initialized) {
      initialize_parser();
      CommentParser::initialized = true;
    }
    parse_comment(comment);
  }

  void parse_comment(std::string comment) {
    std::istringstream s(comment);
    std::string l;
    // Read lines in comment.
    while (std::getline(s, l)) {

        std::istringstream ls(l);

        std::string item;

        if ((ls >> item) && (item != "//")) {
            continue;
        }

        // Check is this is a pxx comment.
        if ((ls >> item) && (item != "pxx")) {
          continue;
        }
        if ((ls >> item) && (item != "::")) {
          continue;
        }

        parser.enable_packrat_parsing();
        std::vector<ExportSettings> ret;
        parser.parse(l.c_str(), ret);
        for (auto& s : ret) {
          settings += s;
        }
      }
  }

  ExportSettings settings;
  std::map<int, std::string> warnings;

  friend void initialize_parser();

 private:
  static bool initialized;
  static peg::parser parser;
};

bool CommentParser::initialized = false;
peg::parser CommentParser::parser = peg::parser(R"(
    pxx <- '//' 'pxx' '::' expression (',' expression)*
    expression <- export / instance
    export <- 'export' ('(' ')')?
    string <- '\"'[a-zA-Z0-9_,;<>?: ]*'\"'
    string_list <- '[' string (',' string)* ']'
    instance <- 'instance('(string ',')? string_list ')'
    %whitespace <- [ \t]*
    )");


void initialize_parser() {
  CommentParser::parser["pxx"] = [](const peg::SemanticValues& sv) {
    return sv.transform<ExportSettings>();
  };

  CommentParser::parser["expression"] = [](const peg::SemanticValues& sv) {
    return sv[0];
  };

  CommentParser::parser["export"] = [](const peg::SemanticValues& /*sv*/) {
    return ExportSettings{true};
  };

  CommentParser::parser["instance"] = [](const peg::SemanticValues& sv) {
    std::string name = "";
    if (sv.size() > 1) {
      name = peg::any_cast<std::string>(sv[0]);
    }
    auto instance_strings = std::make_pair(
        name, peg::any_cast<std::vector<std::string>>(sv[sv.size() - 1]));
    return ExportSettings{false, {instance_strings}};
  };

  CommentParser::parser["string_list"] = [](const peg::SemanticValues& sv) {
    std::vector<std::string> list{};
    for (size_t i = 0; i < sv.size(); ++i) {
      list.push_back(peg::any_cast<std::string>(sv[i]));
    }
    return list;
  };

  CommentParser::parser["string"] = [](const peg::SemanticValues& sv) {
    std::string s_raw = sv.token();
    std::string s(s_raw.size() - 2, ' ');
    s_raw.copy(s.data(), s.size(), 1);
    return s;
  };
}
}  // namespace pxx::ast

#endif
