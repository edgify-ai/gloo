#pragma once

#include "gloo/rendezvous/store.h"
#include <unordered_map>
#include <iostream>
#include <fstream>

namespace gloo {
namespace rendezvous {

class EdgifyStore : public Store {
 public:
  explicit EdgifyStore(std::unordered_map<std::string, std::vector<std::vector<char>>>  participants, int selfRank);
  virtual ~EdgifyStore() {}

  virtual void set(const std::string& key, const std::vector<char>& data)
      override;

  virtual std::vector<char> get(const std::string& key) override;

    bool check(const std::vector<std::string>& keys);

    bool connectionOK(const std::string key);

    virtual void wait(const std::vector<std::string>& keys) override {
    wait(keys, Store::kDefaultTimeout);
  }

  virtual void wait(
      const std::vector<std::string>& keys,
      const std::chrono::milliseconds& timeout) override;

protected:
    std::unordered_map<std::string, std::vector<std::vector<char>>>  _participants;
    int _selfRank;
    std::ofstream glooLog;

};

} // namespace rendezvous
} // namespace gloo
