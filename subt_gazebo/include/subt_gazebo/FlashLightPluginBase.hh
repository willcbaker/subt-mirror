/*
 * Copyright (C) 2018 Open Source Robotics Foundation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
*/

#ifndef SUBT_GAZEBO_FLASHLIGHTPLUGINBASE_HH_
#define SUBT_GAZEBO_FLASHLIGHTPLUGINBASE_HH_

#include <string>
#include <vector>
#include "gazebo/msgs/msgs.hh"
#include "gazebo/transport/transport.hh"
#include "gazebo/common/Plugin.hh"
#include "gazebo/physics/physics.hh"
#include "ignition/math/Color.hh"

namespace gazebo
{
  // forward declaration
  class FlashLightSettings;

  /// \brief A plugin that turns on/off a light component in the model.
  // This plugin accesses <light> components in the model specified
  // by <flash_light> as a parameter.
  //
  // <flash_light>
  //   <link_name>link_light</link_name>
  //   <light_name>light_source1</light_name>
  //   <duration>0.1</duration>
  //   <interval>0.4</interval>
  //   <color>
  //     <R>0.0</R>
  //     <G>1.0</G>
  //     <B>1.0</B>
  //   </color>
  //   <angle>60</angle>
  //   <range>30</range>
  // </flash_light>
  //
  // More than one <flash_light> can exist.
  //
  // Settings for each flash light is separately stored in a
  // FlashLightSettings class, which takes care of light specifications
  // such as color, range, blinking interval, and so on.
  //
  // This base class provides basic functions to turn the lights on/off.
  // Users can create their own flash light plugin by inheriting this base
  // model.
  //
  class FlashLightPluginBase : public ModelPlugin
  {
    /// \brief list of light settings to control
    private: std::vector<std::shared_ptr<FlashLightSettings>> list_flash_light;

    /// \brief pointer to the model
    private: physics::ModelPtr model;

    /// \brief pointer to the world
    private: physics::WorldPtr world;

    /// \brief pointer to the update even connection
    private: event::ConnectionPtr updateConnection;

    /// \brief Called when the plugin is loaded
    public: void Load(physics::ModelPtr _parent, sdf::ElementPtr _sdf);

    /// \brief Called by the world update start event
    public: virtual void OnUpdate();

    /// \brief Turn on a flash light specified by the light name
    /// If more than one link have lights with the identical name,
    /// the first appearing light in the list will be updated.
    /// \param[in] _light_name The name of flash light
    public: virtual bool TurnOn(const std::string &_light_name) final;

    /// \brief Turn on a flash light specified by the name and its link
    /// \param[in] _light_name The name of flash light
    /// \param[in] _link_name The name of the link holding the light
    public: virtual bool TurnOn(
        const std::string &_light_name, const std::string &_link_name) final;

    /// \brief Turn on all flash lights
    public: virtual bool TurnOnAll() final;

    /// \brief Turn off a flash light specified by the name
    /// If more than one link have lights with the identical name,
    /// the first appearing light in the list will be updated.
    /// \param[in] _light_name The name of flash light
    public: virtual bool TurnOff(const std::string &_light_name) final;

    /// \brief Turn off a flash light specified by the name
    /// \param[in] _light_name The name of flash light
    /// \param[in] _link_name The name of the link holding the light
    public: virtual bool TurnOff(const std::string &_light_name,
        const std::string &_link_name) final;

    /// \brief Turn off all flash lights
    public: virtual bool TurnOffAll() final;
  };

  /// \brief Internal data class to hold individual flash light settings
  class FlashLightSettings
  {
    /// \brief The name of flash light
    private: std::string name;

    /// \brief Link which holds this flash light
    private: physics::LinkPtr link;

    /// \brief The time at which the current phase started
    private: common::Time start_time;

    /// \brief The current switch state (the light itself on or off)
    private: bool f_switch_on;

    /// \brief The current flasshing state (flashing or not)
    private: bool f_flashing;

    /// \brief The duration time to flash (in seconds)
    private: double duration;

    /// \brief The interval time between flashing (in seconds)
    //  When it is zero, the light is constant.
    private: double interval;

    /// \brief The color of light
    private: ignition::math::Color color;

    /// \brief The length of the ray (in meters)
    private: double range;

    /// \brief The angle of the ray (in degrees)
    private: double angle;

    /// \brief The pointer to node for communication
    private: static transport::NodePtr node;

    /// \brief The pointer to publisher to send a command to a light
    private: static transport::PublisherPtr pubLight;

    /// \brief Flash the light
    /// This function is internally used to update the light in the environment
    private: void Flash();

    /// \brief Dim the light
    /// This function is internally used to update the light in the environment
    private: void Dim();

    /// \brief Constructor
    public: FlashLightSettings(
      const physics::ModelPtr _model,
      const sdf::ElementPtr _sdfFlashLight,
      const common::Time _current_time);

    /// \brief Update the light based on the given time
    public: void UpdateLightInEnv(const common::Time _current_time);

    /// \brief Assignment operator
    public: FlashLightSettings& operator=(const FlashLightSettings &_settings);

    /// \brief Getter of name
    public: const std::string GetName() const;

    /// \brief Getter of link
    public: const physics::LinkPtr GetLink() const;

    /// \brief Switch on
    public: void SwitchOn();

    /// \brief Switch off
    public: void SwitchOff();
  };
}
#endif
