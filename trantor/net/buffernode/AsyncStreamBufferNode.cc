#include "BufferNode.h"

namespace trantor {
class AsyncBufferNode : public BufferNode {
public:
  AsyncBufferNode()           = default;
  ~AsyncBufferNode() override = default;

  /**
   * @brief Indicate whether the buffer is asynchronous.
   * @return
   */
  bool isAsync() const override {
    return true;
  }

  /**
   * @brief Indicate this buffer is a stream.
   * @return
   */
  bool isStream() const override {
    return true;
  }

  /**
   * @brief Indicate whether the buffer is available.
   * @return
   */
  bool available() const override {
    return !isDone_;
  }

  /**
   * @brief Append data to the buffer.
   * @param data
   * @param len
   */
  void append(const char *data, size_t len) override {
    if (!msgBufferPtr_) {
      msgBufferPtr_ = std::make_unique<MsgBuffer>(len);
    }
    msgBufferPtr_->append(data, len);
  }

  /**
   * @brief Get data from the buffer.
   * @param data
   * @param len
   */
  void getData(const char *&data, size_t &len) override {
    if (msgBufferPtr_) {
      data = msgBufferPtr_->peek();
      len  = msgBufferPtr_->readableBytes();
    } else {
      data = nullptr;
      len  = 0;
    }
  }

  /**
   * @brief Retrieve data from the buffer by len.
   * @param len
   */
  void retrieve(size_t len) override {
    assert(msgBufferPtr_);
    if (msgBufferPtr_) {
      msgBufferPtr_->retrieve(len);
    }
  }

  /**
   * @brief Remaining bytes in the buffer.
   * @return
   */
  long long remainingBytes() const override {
    if (msgBufferPtr_) return static_cast<long long>(msgBufferPtr_->readableBytes());
    return 0;
  }

private:
  std::unique_ptr<MsgBuffer> msgBufferPtr_;
};

BufferNodePtr BufferNode::newAsyncStreamBufferNode() {
  return std::make_shared<AsyncBufferNode>();
}
}  // namespace trantor
