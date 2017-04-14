/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.01 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_01.txt                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
*/

#include "hphp/runtime/ext/extension.h"
#include "hphp/util/lock.h"
#include "hphp/util/mutex.h"
#include <unordered_map>
#include <queue>
#include <string>

namespace HPHP {


class SharedFifo {
  ReadWriteMutex fifo_queue_mutex;
  const String& name;
  std::queue<std::string> fifo_queue;
public:
  SharedFifo(const String& _name) : name(_name) { }

  void push(const String& value) {
    WriteLock write_lock(fifo_queue_mutex);
    fifo_queue.push(value.toCppString());
  }

  Variant pop() {
    WriteLock write_lock(fifo_queue_mutex);

    if (fifo_queue.size() == 0) {
      return Variant(Variant::NullInit{});
    }

    const std::string value = fifo_queue.front();
  fifo_queue.pop();
  return Variant(String(value));
   }
};


static ReadWriteMutex shared_fifo_mutex;
static std::unordered_map<std::string, SharedFifo*> shared_fifos;

static bool HHVM_FUNCTION(shfifo_init, const String& name) {
  const std::string cpp_name = name.toCppString();

  {
    ReadLock read_lock(shared_fifo_mutex);
    std::unordered_map<std::string, SharedFifo*>::const_iterator shared_fifo = shared_fifos.find(cpp_name);
    if (shared_fifo != shared_fifos.end()) return false;
  }

  {
  WriteLock write_lock(shared_fifo_mutex);

    // Have to gate against a race condition by checking if it was just created here!
    std::unordered_map<std::string, SharedFifo *>::const_iterator shared_fifo = shared_fifos.find(cpp_name);
    if (shared_fifo != shared_fifos.end()) return false;

    shared_fifos[cpp_name] = new SharedFifo(name);
  }

  return true;
}

static bool HHVM_FUNCTION(shfifo_push, const String& queue_name, const String& value) {
  ReadLock read_lock(shared_fifo_mutex);
  std::unordered_map<std::string, SharedFifo *>::const_iterator shared_fifo = shared_fifos.find(queue_name.toCppString());
  if (shared_fifo == shared_fifos.end()) return false;

  shared_fifo->second->push(value);

  return true;
}

static Variant HHVM_FUNCTION(shfifo_pop, const String& queue_name, const String& value) {
  ReadLock read_lock(shared_fifo_mutex);
  std::unordered_map<std::string, SharedFifo *>::const_iterator shared_fifo = shared_fifos.find(queue_name.toCppString());
  if (shared_fifo == shared_fifos.end()) return Variant(false);

  return shared_fifo->second->pop();
}

class SharedFifoExtension : public Extension {
 public:
  SharedFifoExtension() : Extension("shared_fifo") {}
  virtual void moduleInit() {
    HHVM_FE(shfifo_init);
    HHVM_FE(shfifo_push);
    HHVM_FE(shfifo_pop);
    loadSystemlib();
  }
} s_shared_fifo_extension;

HHVM_GET_MODULE(shared_fifo);

} // namespace HPHP