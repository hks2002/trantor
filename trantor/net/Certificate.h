
#ifndef TRANTOR_CERTIFICATE_H
#define TRANTOR_CERTIFICATE_H

#include <memory>
#include <string>

namespace trantor {
struct Certificate {
  virtual ~Certificate()                        = default;
  virtual std::string sha1Fingerprint() const   = 0;
  virtual std::string sha256Fingerprint() const = 0;
  virtual std::string pem() const               = 0;
};
using CertificatePtr = std::shared_ptr<Certificate>;

}  // namespace trantor

#endif  // TRANTOR_CERTIFICATE_H
