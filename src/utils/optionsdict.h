#pragma once

#include <map>
#include <string>
#include <unordered_map>
#include <vector>
#include "utils/exception.h"

namespace pzero {

template <typename T>
class TypeDict {
 protected:
  std::unordered_map<std::string, T> dict_;
};

class OptionsDict : TypeDict<bool>,
                    TypeDict<int>,
                    TypeDict<std::string>,
                    TypeDict<float> {
 public:

  OptionsDict(const OptionsDict* parent = nullptr) : parent_(parent) {}

  // e.g. dict.Get<int>("threads")
  // Returns value of given type. Throws exception if not found.
  template <typename T>
  T Get(const std::string& key) const;

  // Checks whether the given key exists for given type.
  template <typename T>
  bool Exists(const std::string& key) const;

  // Returns value of given type. Returns default if not found.
  template <typename T>
  T GetOrDefault(const std::string& key, const T& default_val) const;

  // Sets value for a given type.
  template <typename T>
  void Set(const std::string& key, const T& value);

  // Get reference to assign value to.
  template <typename T>
  T& GetRef(const std::string& key);

  // Returns true when the value is not set anywhere maybe except the root
  // dictionary;
  template <typename T>
  bool IsDefault(const std::string& key) const;

  // Returns subdictionary. Throws exception if doesn't exist.
  const OptionsDict& GetSubdict(const std::string& name) const;

  // Returns subdictionary. Throws exception if doesn't exist.
  OptionsDict* GetMutableSubdict(const std::string& name);

  // Creates subdictionary. Throws exception if already exists.
  OptionsDict* AddSubdict(const std::string& name);

  // Returns list of subdictionaries.
  std::vector<std::string> ListSubdicts() const;

  // Creates options dict from string. Example of a string:
  // option1=1, option_two = "string val", subdict(option3=3.14)
  //
  // the sub dictionary is containing a parent pointer refering 
  // back to this object. You need to ensure, that this object
  // is still in scope, when the parent pointer is used
  void AddSubdictFromString(const std::string& str);

  bool HasSubdict(const std::string& name) const;

 private:
  const OptionsDict* parent_ = nullptr;
  std::map<std::string, OptionsDict> subdicts_;
};

template <typename T>
T OptionsDict::Get(const std::string& key) const {
  const auto& dict = TypeDict<T>::dict_;
  auto iter = dict.find(key);
  if (iter != dict.end()) return iter->second;
  if (parent_) return parent_->Get<T>(key);
  throw Exception("Key [" + key + "] was not set in options.");
}

template <typename T>
bool OptionsDict::Exists(const std::string& key) const {
  const auto& dict = TypeDict<T>::dict_;
  auto iter = dict.find(key);
  if (iter != dict.end()) return true;
  if (!parent_) return false;
  return parent_->Exists<T>(key);
}

template <typename T>
T OptionsDict::GetOrDefault(const std::string& key,
                            const T& default_val) const {
  const auto& dict = TypeDict<T>::dict_;
  auto iter = dict.find(key);
  if (iter != dict.end()) return iter->second;
  if (parent_) return parent_->GetOrDefault<T>(key, default_val);
  return default_val;
}

template <typename T>
void OptionsDict::Set(const std::string& key, const T& value) {
  TypeDict<T>::dict_[key] = value;
}

template <typename T>
T& OptionsDict::GetRef(const std::string& key) {
  return TypeDict<T>::dict_[key];
}

template <typename T>
bool OptionsDict::IsDefault(const std::string& key) const {
  if (!parent_) return true;
  const auto& dict = TypeDict<T>::dict_;
  if (dict.find(key) != dict.end()) return false;
  return parent_->IsDefault<T>(key);
}

}  // namespace pzero
