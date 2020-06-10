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
      : environment_(template_directory), includes_(includes), name_(name) {
    environment_.set_trim_blocks(true);
    environment_.set_lstrip_blocks(true);
  }

  std::string render(ast::TranslationUnit tu) {
    json data{};
    to_json(data, tu);
    data["module_name"] = name_;
    data["includes"] = includes_;

    if (tu.has_standard_headers()) {
        data["standard_headers"] = true;
    } else {
        data["standard_headers"] = false;
    }


    auto temp = environment_.parse_template("module.template");
    environment_.set_trim_blocks(true);
    environment_.set_lstrip_blocks(true);
    auto result = environment_.render(temp, data);
    return result;
  }

private:
  inja::Environment environment_;
  std::vector<std::string> includes_;
  std::string name_;
};

} // namespace pxx::python

#endif
