/** \file pxx/cxx/language_object.h
 *
 * Defines the LanguageObject class which is the base class for all nodes
 * of the C++ AST.
 */
#ifndef __PXX_CXX_LANGUAGE_OBJECT_H__
#define __PXX_CXX_LANGUAGE_OBJECT_H__

#include <memory>

#include <inja/inja.hpp>
#include <clang-c/Index.h>

#include <pxx/utils.h>
#include <pxx/comment_parser.h>
#include <pxx/cxx/scope.h>

namespace pxx {
namespace cxx {

using pxx::comment_parser::CommentParser;
using pxx::comment_parser::ExportSettings;
using json = nlohmann::json;

/** Base class for all C++ language constructs.
 *
 * Each C++ language object has two types of name. Its normal name,
 * or spelling, which is the objects name as it appears in the code,
 * and the qualified name, which is the name that identifies the the
 * object at root scope.
 *
 * Each language object also has a parent scope, which it needs to know
 * in order to determine its qualified name.
 *
 * Additionally, each language object has export settings that define
 * their behavior during interface creation.
 */
class LanguageObject {
 public:
  /** Create new language object.
     *
     * @param cursor The Clang AST cursor representing the object.
     * @param parent_scope The parent scope of the language object.
     */
  LanguageObject(CXCursor cursor, LanguageObject *parent = nullptr)
      : name_(to_string(clang_getCursorSpelling(cursor))),
        parent_(parent),
        parent_scope_(parent ? parent->get_scope() : nullptr) {
    if (parent) {
      settings_ = parent->get_export_settings();
    }

    export_name_ = name_;

    if (clang_isDeclaration(clang_getCursorKind(cursor))) {
      auto s = to_string(clang_Cursor_getRawCommentText(cursor));
      CommentParser cp(s, settings_);
      settings_ = cp.settings;
    }
  }

  virtual ~LanguageObject() = default;

  /** Clone language object
   *
   * Creates new LanguageObject object on the heap and returns it
   * as smart pointer.
   *
   * @param The new parent of the language object or nullptr for AST root.
   * @return std::shared_ptr pointing to clone.
   */
  std::shared_ptr<LanguageObject> clone(LanguageObject *p) {
    auto lo_new = std::make_shared<LanguageObject>(*this);
    lo_new->set_parent(p);
    return lo_new;
  }
  /** Spelling of language object.
     * @return The name of the object as it appears in the code.
     */
  std::string get_name() const { return name_; }
  /** Set spelling of language object.
   * @param The new name of the object.
   */
  void set_name(std::string name) { name_ = name; }

  /// Get parent pointer.
  LanguageObject *get_parent() { return parent_; }
  const LanguageObject *get_parent() const { return parent_; }
  /** Set parent pointer of object.
   * @param lo Pointer to new parent.
   */
  virtual void set_parent(LanguageObject *lo) {
    parent_ = lo;
    parent_scope_ = lo->get_scope();
  }
  /** Qualified name of language object.
     *
     * The qualified name is the name of the object that identifies it at root scope.
     *
     * @return The qualified name with all dependent parts are resolved.
     */
  std::string get_qualified_name() const {
    std::string prefix = "";
    if (parent_scope_) {
      prefix = parent_scope_->get_prefix();
    }
    return prefix + name_;
  }
  /** Get export name.
   * The export name is the name that the object will have on the Python side.
   * It can modified set using a pxx comment.
   * @return The export name
   */
  std::string get_export_name() const { return export_name_; }
  /** Set the export name of the object.
   * @param The new export name.
   */
  void set_export_name(std::string name) { export_name_ = name; }

  /** Get the object's scope.
   *
   * Note that this is the object's scope and not the scope at which the object
   * is defined, which is the parent's scope.
   *
   * @return The object's scope.
   */
  virtual Scope *get_scope() { return parent_scope_; }
  virtual const Scope *get_scope() const { return parent_scope_; }

  /// Get the object's export settings.
  /** Get the object's export settings.
   *
   * The export setting define how the object is exposed through
   * the Python interface.
   *
   * @return The object's scope.
   */
  ExportSettings get_export_settings() const { return settings_; }
  void set_export_settings(ExportSettings settings) { settings_ = settings; }

 protected:
  std::string name_;
  std::string export_name_;
  LanguageObject *parent_;
  Scope *parent_scope_;
  ExportSettings settings_;
};

/** Generic clone method.
 *
 * Generic method to clone AST nodes. Cloning AST nodes is required to
 * create an independent cope of parts of the AST, which is required
 * for example for template instances. Note that cloning the AST also
 * requires updating the node parent pointer, which this method
 * takes care of.
 *
 * @param lo The LanguageObject instance to clone.
 * @param Pointer to cloned parent node
 * @return Smart pointer to the cloned object.
 */
template <typename T>
std::shared_ptr<T> clone(const T &lo, LanguageObject *p) {
  std::shared_ptr<T> a = std::make_shared<T>(lo);
  if (p) {
    a->set_parent(p);
  }
  return a;
}

/** JSON serialization of LanguageObject
 *
 * Interface function defining the interface for the json library.
 * Stores name, qualified and export name of the language object.
 *
 * @param j JSON handle serving as destination.
 * @param lo The language object to serialize.
 */
void to_json(json &j, LanguageObject lo) {
  j["name"] = lo.get_name();
  j["qualified_name"] = lo.get_qualified_name();
  j["export_name"] = lo.get_export_name();
}

}
}

#endif
