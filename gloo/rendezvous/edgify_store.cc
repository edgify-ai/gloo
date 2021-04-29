#include <gloo/rendezvous/edgify_store.h>

#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <array>
#include <fstream>
#include <functional>
#include <iostream>
#include <thread>
#include <map>
#include <arpa/inet.h>

#include "gloo/common/error.h"
#include "gloo/common/logging.h"

namespace gloo {
namespace rendezvous {

EdgifyStore::EdgifyStore(std::unordered_map<std::string, std::vector<std::vector<char>>>  participants, int selfRank) {
    _participants = participants;
    _selfRank = selfRank;
}

void EdgifyStore::set(const std::string& key, const std::vector<char>& data) {
    //unnecessary. we get all our information in the constructor.
}

std::vector<char> EdgifyStore::get(const std::string& key) {

    // concatenate the data to a single vector<char>
    std::vector<char> data;
    for (const auto& p : _participants[key]){
        data.insert( data.end(), p.begin(), p.end() );
    }
    return data;
}


bool EdgifyStore::connectionOK(const std::string key) {
    // find the appropriate port to connect to.
    auto prefix = key.substr(0, key.find("/"));
    auto peerRank = std::stoi(key.substr(key.find("/")+1, key.size()));
    int adjRank = peerRank > _selfRank ? peerRank -1 : peerRank;

    std::vector<char> peer = _participants[key][adjRank];

    //convert to sockaddr_storage
    struct sockaddr_storage ss;
    memcpy(&ss, peer.data(), sizeof(ss));

    //connect and disconnect to verify peer is available
    //tcp only (SOCK_STREAM)
    int fd = socket(ss.ss_family, SOCK_STREAM, 0);
    int res = connect(fd, (const sockaddr*)&ss,  sizeof(peer));
    close(fd);

    glooLog.open("Edgify_glooLog" , std::ofstream::out | std::ofstream::app);
    glooLog << "connectionOK key: " << key << " trying to connect: "  << (res == 0) << "\n";
    glooLog.close();

    return res == 0;
}

bool EdgifyStore::check(const std::vector<std::string>& keys) {
    for (const auto& key : keys){
        if (!connectionOK(key)){
            return false;
        }
    }
    return true;
}

void EdgifyStore::wait(
    const std::vector<std::string>& keys,
    const std::chrono::milliseconds& timeout) {


    glooLog.open("Edgify_glooLog" , std::ofstream::out | std::ofstream::app);
    glooLog << "wait was called with key(s): " << ::gloo::MakeString(keys) << "\n";
    glooLog.close();

    //check if keys are available until timeout is reached
  const auto start = std::chrono::steady_clock::now();
  while (!check(keys)) {
    const auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now() - start);
    if (timeout != kNoTimeout && elapsed > timeout) {
      GLOO_THROW_IO_EXCEPTION(GLOO_ERROR_MSG(
          "Wait timeout for key(s): ", ::gloo::MakeString(keys)));
    }
    /* sleep override */
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }
}

} // namespace rendezvous
} // namespace gloo
