/** \file pxx/generator.h
 *
 * This file defines the Generator class which provides the top-level interface
 * to the functionality of pxx.
 *
 */
#ifndef __PXX_GENERATOR_H__
#define __PXX_GENERATOR_H__

#include <string>

#include <clang-c/Index.h>

namespace pxx {

/** Generator for Python interfaces.
 *
 * This class is responsible of generating Python bindings
 * from a C++ translation unit.
 */
class Generator {

  /** Invoke clang.
   *
   * This function run clang on the provided input and sets the
   * index_ and unit_ memeber variables of the class.
   */
  void invoke_clang() {
    std::vector<const char *> command_line_args = {"-x", "c++", "-std=c++11",
                                                   "-fparse-all-comments"};
    for (auto &s : extra_arguments_) {
      command_line_args.push_back(s.c_str());
    }
    index_ = clang_createIndex(0, 0);
    unit_ = clang_parseTranslationUnit(
        index_, filename_.c_str(), command_line_args.data(),
        command_line_args.size(), nullptr, 0, CXTranslationUnit_None);
    for (size_t i = 0; i < clang_getNumDiagnostics(unit_); ++i) {
      std::cout << "Warning encountered during parsing of translation unit:"
                << std::endl;
      std::cout << clang_getDiagnostic(unit_, i) << std::endl;
    }
    if (unit_ == nullptr) {
      throw std::runtime_error("Failed to parse the translation unit.");
    }
  }

public:
  Generator(std::string filename,
            std::vector<std::string> extra_arguments)
      : filename_(filename),
        extra_arguments_(extra_arguments)
  {
      invoke_clang();
  }


  ~Generator() {
      clang_disposeTranslationUnit(unit_);
      clang_disposeIndex(index_);
  }

private:

  std::string filename_;
  std::vector<std::string> extra_arguments_;

  CXIndex index_;
  CXTranslationUnit unit_;
};

} // namespace pxx
#endif
