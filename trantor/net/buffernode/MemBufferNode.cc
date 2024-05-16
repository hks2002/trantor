#include "BufferNode.h"
namespace trantor {
class MemBufferNode : public BufferNode {
public:
  MemBufferNode() = default;

  /**
   * @brief Append data to the buffer.
   * @param data
   * @param len
   */
  void append(const char *data, size_t len) override {
    buffer_.append(data, len);
  }

  /**
   * @brief Get data from the buffer.
   * @param data
   * @param len
   */
  void getData(const char *&data, size_t &len) override {
    data = buffer_.peek();
    len  = buffer_.readableBytes();
  }

  /**
   * @brief Retrieve data from the buffer by len.
   * @param len
   */
  void retrieve(size_t len) override {
    buffer_.retrieve(len);
  }

  /**
   * @brief Remaining bytes in the buffer.
   * @return
   */
  long long remainingBytes() const override {
    if (isDone_) return 0;
    return static_cast<long long>(buffer_.readableBytes());
  }

private:
  trantor::MsgBuffer buffer_;
};

BufferNodePtr BufferNode::newMemBufferNode() {
  return std::make_shared<MemBufferNode>();
}
}  // namespace trantor
