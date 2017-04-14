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
#include <unordered_map>

namespace HPHP {

static Mutex shared_fifo_mutex;
static std::unordered_map<std::string, SharedFifo> shared_fifos;

static bool HHVM_FUNCTION(shfifo_init, const String& name) {
  {
    ReadLock(shared_fifo_mutex);
    std::unordered_map<std::string, SharedFifo>::const_iterator shared_fifo = shared_fifos.find(name);
    if (shared_fifo != shared_fifos.end()) return false;
  }

  SharedFifo shared_fifo = new SharedFifo(name);
  
  {
    WriteLock(shared_fifo_mutex);

    // Have to gate against a race condition by checking if it was just created here!
    std::unordered_map<std::string, SharedFifo>::const_iterator shared_fifo = shared_fifos.find(name);
    if (shared_fifo != shared_fifos.end()) return false;

    shared_fifos.insert(std::pair<std::string, SharedFifo>{name, shared_fifo});
  }

  return true;
}

static bool HHVM_FUNCTION(shfifo_push, const String& pool_name, const String& value) {
  {
    ReadLock(shared_fifo_mutex);
    std::unordered_map<std::string, SharedFifo>::const_iterator shared_fifo = shared_fifos.find(name);
    if (shared_fifo == shared_fifos.end()) return false;
  }

  return shared_fifo.push(value);
}

static Variant HHVM_FUNCTION(shfifo_pop, const String& pool_name, const String& value) {
  {
    ReadLock(shared_fifo_mutex);
    std::unordered_map<std::string, SharedFifo>::const_iterator shared_fifo = shared_fifos.find(name);
    if (shared_fifo == shared_fifos.end()) return false;
  }

  return shared_fifo.pop();
}

static class SharedFifoExtension : public Extension {
 public:
  SharedFifoExtension() : Extension("shared_fifo") {}
  virtual void moduleInit() {
    HHVM_FE(shfifo_init);
    HHVM_FE(shfifo_push);
    HHVM_FE(shfifo_pop);
    loadSystemlib();
  }
} s_shared_fifo;

static class SharedFifo {
  static Mutex fifo_queue_mutex;
  const String& name;
  const std::queue<std::string> fifo_queue;
public:
  SharedFifo(const String& _name) {
    name = _name;
  }

  bool push(const String& value) {
    WriteLock(fifo_queue_mutex);
    fifo_queue.push(value);

    return true;
  }

  Variant pop() {
    WriteLock(fifo_queue_mutex);

    if (fifo_queue_mutex.size() === 0) {
      return Variant(Variant::NullInit{});
    }

    return fifo_queue.pop();
  }
}

HHVM_GET_MODULE(shared_fifo)

} // namespace HPHP