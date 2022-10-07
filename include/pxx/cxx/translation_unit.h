/** \file pxx/cxx/translation_unit.
 *
 * Defines the TranslationUnit class, which provides a high-level interface to manipulate
 * parsed C++ code.
 */
#include <string>
#include <pxx/writer.h>
#include <pxx/cxx/parser.h>


#ifndef __PXX_CXX_TRANSLATION_UNIT_H__
#define __PXX_CXX_TRANSLATION_UNIT_H__

namespace pxx {
namespace cxx {

/** A C++ translation unit.

TranslationUnit provides the high-level interface for the pxx representation
of a parsed C++ file.

*/
class TranslationUnit{
public:
    TranslationUnit(
        std::filesystem::path filename,
        std::vector<std::string> additional_args
        ) : parser_(filename, additional_args) {
        auto ptrs = parser_.parse();
        ast_ = std::unique_ptr<ASTNode>(std::get<0>(ptrs));
        scope_ = std::unique_ptr<Scope>(std::get<1>(ptrs));
    }

    TranslationUnit(
        std::string filename,
        std::vector<std::string> additional_args
        ) : TranslationUnit(std::filesystem::path(filename), additional_args) {}

    std::string dump_ast() {
        std::stringstream output{};
        ast_->print_tree(output);
        return output.str();
    }

    std::string print_bindings(Settings settings) {
        std::stringstream output{};
        auto writer = Writer{output};
        writer.write(scope_.get(), ast_.get(), settings);
        return output.str();
    }


private:

    Parser parser_;
    std::unique_ptr<ASTNode> ast_;
    std::unique_ptr<Scope> scope_;

};

}
}
#endif
