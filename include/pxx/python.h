#ifndef __PXX_PYTHON_H__
#define __PXX_PYTHON_H__

#include <clang-c/Index.h>
#include <iostream>
#include <string>
#include <vector>
#include <filesystem>
#include <inja/inja.hpp>

namespace pxx::python {

using std::filesystem::path;

path template_directory = path(__FILE__).parent_path().append("templates/");

class Module {
public:
  Module(std::string name, std::vector<std::string> includes)
      : name_(name), includes_(includes), environment_(template_directory) {
    environment_.set_trim_blocks(true);
  }

  std::string render(ast::TranslationUnit tu) {
    json data{};
    to_json(data, tu);
    data["module_name"] = name_;
    data["includes"] = includes_;
    auto temp = environment_.parse_template("module.template");
    auto result = environment_.render(temp, data);
    return result;
  }

private:
  std::vector<std::string> includes_;
  std::string name_;
  inja::Environment environment_;
};

} // namespace pxx::python

#endif
