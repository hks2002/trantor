#include "BufferNode.h"
namespace trantor {
static const size_t kMaxSendFileBufferSize = 16 * 1024;
class StreamBufferNode : public BufferNode {
public:
  StreamBufferNode(std::function<std::size_t(char *, std::size_t)> &&callback) : streamCallback_(std::move(callback)) {}

  ~StreamBufferNode() override {
    if (streamCallback_) streamCallback_(nullptr, 0);  // cleanup callback internals
  }

  /**
   * @brief Indicate this buffer is a stream.
   * @return
   */
  bool isStream() const override {
    return true;
  }

  /**
   * @brief Get data from the buffer.
   * @param data
   * @param len
   */
  void getData(const char *&data, size_t &len) override {
    if (msgBuffer_.readableBytes() == 0) {
      msgBuffer_.ensureWritableBytes(kMaxSendFileBufferSize);
      auto n = streamCallback_(msgBuffer_.beginWrite(), msgBuffer_.writableBytes());
      if (n > 0) {
        msgBuffer_.hasWritten(n);
      } else {
        isDone_ = true;
      }
    }
    data = msgBuffer_.peek();
    len  = msgBuffer_.readableBytes();
  }

  /**
   * @brief Retrieve data from the buffer by len.
   * @param len
   */
  void retrieve(size_t len) override {
    msgBuffer_.retrieve(len);
#ifndef NDEBUG
    dataWritten_ += len;
    LOG_TRACE << "send stream in loop: bytes written: " << dataWritten_ << " / total bytes written: " << dataWritten_;
#endif
  }

  /**
   * @brief Remaining bytes in the buffer.
   * @return
   */
  long long remainingBytes() const override {
    if (isDone_) return 0;
    return 1;
  }

private:
  std::function<std::size_t(char *, std::size_t)> streamCallback_;
#ifndef NDEBUG  // defined by CMake for release build
  std::size_t dataWritten_{0};
#endif
  MsgBuffer msgBuffer_;
};

BufferNodePtr BufferNode::newStreamBufferNode(StreamCallback &&callback) {
  return std::make_shared<StreamBufferNode>(std::move(callback));
}
}  // namespace trantor