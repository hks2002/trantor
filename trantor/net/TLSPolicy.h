/**
 *
 *  @file TLSPolicy.h
 *  @author An Tao
 *
 *  Copyright 2018, An Tao.  All rights reserved.
 *  https://github.com/an-tao/trantor
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the License file.
 *
 *  Trantor
 *
 */

#ifndef TRANTOR_TLS_POLICY_H
#define TRANTOR_TLS_POLICY_H

#include "trantor/exports.h"
#include <memory>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace trantor {
struct TRANTOR_EXPORT TLSPolicy final {
  /**
   * @brief set the ssl configuration commands. The commands will be passed
   * to the ssl library. The commands are in the form of {{key, value}}.
   * for example, {"SSL_OP_NO_SSLv2", "1"}. Not all TLS providers support
   * this feature AND the meaning of the commands may vary between TLS
   * providers.
   *
   * As of 2023-03 Only OpenSSL supports this feature. LibreSSL does not
   * nor Botan.
   */
  TLSPolicy &setConfCmds(const std::vector<std::pair<std::string, std::string>> &sslConfCmds) {
    sslConfCmds_ = sslConfCmds;
    return *this;
  }

  /**
   * @brief set the hostname to be used for SNI and certificate validation.
   */
  TLSPolicy &setHostname(std::string_view hostname) {
    hostname_ = hostname;
    return *this;
  }

  /**
   * @brief set the path to the certificate file. The file must be in PEM
   * format.
   */
  TLSPolicy &setCertPath(std::string_view certPath) {
    certPath_ = certPath;
    return *this;
  }

  /**
   * @brief set the path to the private key file. The file must be in PEM
   * format.
   */
  TLSPolicy &setKeyPath(std::string_view keyPath) {
    keyPath_ = keyPath;
    return *this;
  }

  /**
   * @brief set the path to the CA file or directory. The file must be in
   * PEM format.
   */
  TLSPolicy &setCaPath(std::string_view caPath) {
    caPath_ = caPath;
    return *this;
  }

  /**
   * @brief enables the use of the old TLS protocol (old meaning < TLS 1.2).
   * TLS providers may not support old protocols even if this option is set
   */
  TLSPolicy &setUseOldTLS(bool useOldTLS) {
    useOldTLS_ = useOldTLS;
    return *this;
  }

  /**
   * @brief set the list of protocols to be used for ALPN.
   *
   * @note for servers, it selects matching protocol against the client's
   * list. And the first matching protocol supplied in the parameter will be
   * selected. If no matching protocol is found, the connection will be
   * closed.
   *
   * @note for clients, it sends the list of protocols to the server.
   */
  TLSPolicy &setAlpnProtocols(const std::vector<std::string> &alpnProtocols) {
    alpnProtocols_ = alpnProtocols;
    return *this;
  }
  /**
   * @brief set the list of protocols to be used for ALPN.
   *
   * @note for servers, it selects matching protocol against the client's
   * list. And the first matching protocol supplied in the parameter will be
   * selected. If no matching protocol is found, the connection will be
   * closed.
   *
   * @note for clients, it sends the list of protocols to the server.
   */
  TLSPolicy &setAlpnProtocols(std::vector<std::string> &&alpnProtocols) {
    alpnProtocols_ = std::move(alpnProtocols);
    return *this;
  }

  /**
   * @brief Weather to use the system's certificate store.
   *
   * @note setting both not to use the system's certificate store and to
   * supply a CA path WILL LEAD TO NO CERTIFICATE VALIDATION AT ALL.
   */
  TLSPolicy &setUseSystemCertStore(bool useSystemCertStore) {
    useSystemCertStore_ = useSystemCertStore;
    return *this;
  }

  /**
   * @brief Enable certificate validation.
   */
  TLSPolicy &setValidate(bool enable) {
    validate_ = enable;
    return *this;
  }

  /**
   * @brief Allow broken chain (self-signed certificate, root CA not in
   * allowed list, etc..) but still validate the domain name and date. This
   * option has no effect if validate is false.
   *
   * @note IMPORTANT: This option makes more then self signed certificates
   * valid. It also allows certificates that are not signed by a trusted CA,
   * the CA gets revoked. But the underlying implementation may still check
   * for the type of certificate, date and hostname, etc.. To disable all
   * certificate validation, use setValidate(false).
   */
  TLSPolicy &setAllowBrokenChain(bool allow) {
    allowBrokenChain_ = allow;
    return *this;
  }

  // The getters
  /**
   * @brief Get the Configuration Commands
   *
   * @return const std::vector<std::pair<std::string, std::string>>&
   */
  const std::vector<std::pair<std::string, std::string>> &getConfCmds() const {
    return sslConfCmds_;
  }

  /**
   * @brief Get the Hostname
   */
  const std::string &getHostname() const {
    return hostname_;
  }

  /**
   * @brief Get the Cert Path
   */
  const std::string &getCertPath() const {
    return certPath_;
  }

  /**
   * @brief Get the Key Path
   */
  const std::string &getKeyPath() const {
    return keyPath_;
  }

  /**
   * @brief Get the CA Path
   */
  const std::string &getCaPath() const {
    return caPath_;
  }

  /**
   * @brief If use old TLS
   */
  bool getUseOldTLS() const {
    return useOldTLS_;
  }

  /**
   * @brief If validate
   */
  bool getValidate() const {
    return validate_;
  }

  /**
   * @brief If allow broken chain
   */
  bool getAllowBrokenChain() const {
    return allowBrokenChain_;
  }

  /**
   * @brief Get the list of protocols
   */
  const std::vector<std::string> &getAlpnProtocols() const {
    return alpnProtocols_;
  }

  /**
   * @brief Get the list of protocols
   */
  const std::vector<std::string> &getAlpnProtocols() {
    return alpnProtocols_;
  }

  /**
   * @brief If use system cert store
   */
  bool getUseSystemCertStore() const {
    return useSystemCertStore_;
  }

  /**
   * @brief Get the default server policy.
   * @param certPath
   * @param keyPath
   */
  static std::shared_ptr<TLSPolicy> defaultServerPolicy(std::string_view certPath, std::string_view keyPath) {
    auto policy = std::make_shared<TLSPolicy>();
    policy->setValidate(false).setUseOldTLS(false).setUseSystemCertStore(false).setCertPath(certPath).setKeyPath(
      keyPath);
    return policy;
  }

  /**
   * @brief Get the default server policy.
   * @param hostname
   */
  static std::shared_ptr<TLSPolicy> defaultClientPolicy(std::string_view hostname = "") {
    auto policy = std::make_shared<TLSPolicy>();
    policy->setValidate(true).setUseOldTLS(false).setUseSystemCertStore(true).setHostname(hostname);
    return policy;
  }

protected:
  std::vector<std::pair<std::string, std::string>> sslConfCmds_        = {};
  std::string                                      hostname_           = "";
  std::string                                      certPath_           = "";
  std::string                                      keyPath_            = "";
  std::string                                      caPath_             = "";
  std::vector<std::string>                         alpnProtocols_      = {};
  bool                                             useOldTLS_          = false;  // turn into specific version
  bool                                             validate_           = true;
  bool                                             allowBrokenChain_   = false;
  bool                                             useSystemCertStore_ = true;
};
using TLSPolicyPtr = std::shared_ptr<TLSPolicy>;
}  // namespace trantor

#endif  // TRANTOR_TLS_POLICY_H