
#ifndef TRANTOR_ASYNC_STREAM_H
#define TRANTOR_ASYNC_STREAM_H

#include "trantor/NonCopyable.h"
#include <memory>
#include <string>

namespace trantor {
/**
 * @brief This class represents a data stream that can be sent asynchronously.
 * The data is sent in chunks, and the chunks are sent in order, and all the
 * chunks are sent continuously.
 */
class TRANTOR_EXPORT AsyncStream : public NonCopyable {
public:
  virtual ~AsyncStream() = default;

  /**
   * @brief Send data asynchronously.
   *
   * @param data The data to be sent
   * @param len The length of the data
   * @return true if the data is sent successfully or at least is put in the
   * send buffer.
   * @return false if the connection is closed.
   */
  virtual bool send(const char *data, size_t len) = 0;

  /**
   * @brief Send data asynchronously.
   *
   * @param data The data to be sent
   * @return true if the data is sent successfully or at least is put in the
   * send buffer.
   * @return false if the connection is closed.
   */
  bool send(const std::string &data) {
    return send(data.data(), data.length());
  }

  /**
   * @brief Terminate the stream.
   */
  virtual void close() = 0;
};
using AsyncStreamPtr = std::unique_ptr<AsyncStream>;

}  // namespace trantor

#endif  // TRANTOR_ASYNC_STREAM_H
