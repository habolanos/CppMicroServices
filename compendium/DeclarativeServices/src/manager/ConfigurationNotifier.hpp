/*=============================================================================

 Library: CppMicroServices

 Copyright (c) The CppMicroServices developers. See the COPYRIGHT
 file at the top-level directory of this distribution and at
 https://github.com/CppMicroServices/CppMicroServices/COPYRIGHT .

 Licensed under the Apache License, Version 2.0 (the "License");
 you may not use this file except in compliance with the License.
 You may obtain a copy of the License at

 http://www.apache.org/licenses/LICENSE-2.0

 Unless required by applicable law or agreed to in writing, software
 distributed under the License is distributed on an "AS IS" BASIS,
 WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 See the License for the specific language governing permissions and
 limitations under the License.

 =============================================================================*/

#ifndef __CPPMICROSERVICES_SCRIMPL_CONFIGURATIONNOTIFIER_HPP__
#define __CPPMICROSERVICES_SCRIMPL_CONFIGURATIONNOTIFIER_HPP__

#include "../SCRLogger.hpp"
#include "ConcurrencyUtil.hpp"
#include "cppmicroservices/asyncworkservice/AsyncWorkService.hpp"
#include "cppmicroservices/cm/ConfigurationListener.hpp"

namespace cppmicroservices {
namespace scrimpl {
class ComponentConfigurationImpl;

/** ConfigChangeNotification
     * This class is used by ConfigurationListener to notify ComponentConfigurationImpl
     * about changes to Configuration Objects.
     */
struct ConfigChangeNotification final
{
  ConfigChangeNotification(
    std::string pid,
    std::shared_ptr<cppmicroservices::AnyMap> properties,
    cppmicroservices::service::cm::ConfigurationEventType evt)
    : pid(std::move(pid))
    , event(std::move(evt))
    , newProperties(properties)
  {}

  std::string pid;
  cppmicroservices::service::cm::ConfigurationEventType event;
  std::shared_ptr<cppmicroservices::AnyMap> newProperties;
};

struct Listener final
{
  Listener(std::function<void(const ConfigChangeNotification&)> notify,
           std::shared_ptr<ComponentConfigurationImpl> mgr)
    : notify(std::move(notify))
    , mgr(std::move(mgr))
  {}

  std::function<void(const ConfigChangeNotification&)> notify;
  std::shared_ptr<ComponentConfigurationImpl> mgr;
};

class ConfigurationNotifier final
{

public:
  /**
   * @throws std::invalid_argument exception if any of the params is a nullptr 
   */
  ConfigurationNotifier(
    const cppmicroservices::BundleContext& context,
    std::shared_ptr<cppmicroservices::logservice::LogService> logger,
    std::shared_ptr<cppmicroservices::async::AsyncWorkService>
      asyncWorkService_);

  ConfigurationNotifier(const ConfigurationNotifier&) = delete;
  ConfigurationNotifier(ConfigurationNotifier&&) = delete;
  ConfigurationNotifier& operator=(const ConfigurationNotifier&) = delete;
  ConfigurationNotifier& operator=(ConfigurationNotifier&&) = delete;
  ~ConfigurationNotifier() = default;

  /**
   * @throws std::bad_alloc exception if memory cannot be allocated
   */
  cppmicroservices::ListenerTokenId RegisterListener(
    const std::string& pid,
    std::function<void(const ConfigChangeNotification&)> notify,
    std::shared_ptr<ComponentConfigurationImpl> mgr);

  void UnregisterListener(
    const std::string& pid,
    const cppmicroservices::ListenerTokenId token) noexcept;

  bool AnyListenersForPid(const std::string& pid) noexcept;

  void NotifyAllListeners(
    const std::string& pid,
    cppmicroservices::service::cm::ConfigurationEventType type,
    std::shared_ptr<cppmicroservices::AnyMap> properties);

  void CreateFactoryComponent(const std::string& pid,
                              std::shared_ptr<ComponentConfigurationImpl>& mgr);
private:

  using TokenMap = std::unordered_map<ListenerTokenId, Listener>;

  cppmicroservices::scrimpl::Guarded<
    std::unordered_map<std::string, std::shared_ptr<TokenMap>>>
    listenersMap;

  std::atomic<cppmicroservices::ListenerTokenId> tokenCounter; ///< used to
    ///generate unique
    ///tokens for
    ///listeners

  cppmicroservices::BundleContext bundleContext;
  std::shared_ptr<cppmicroservices::logservice::LogService> logger;
  std::shared_ptr<cppmicroservices::async::AsyncWorkService> asyncWorkService;
};

} // namespace scrimpl
} // namespace cppmicroservices
#endif //__CPPMICROSERVICES_SCRIMPL_CONFIGURATIONNOTIFIER_HPP__
