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

#include "subt_gazebo/CommonTypes.hh"
#include "subt_gazebo/LightControlPlugin.hh"
#include "subt_gazebo/protobuf/lightcommand.pb.h"

using namespace gazebo;
using namespace subt;

//////////////////////////////////////////////////
void LightControlPlugin::Load(physics::ModelPtr _parent, sdf::ElementPtr _sdf)
{
  // === must call this ===
  FlashLightPlugin::Load(_parent, _sdf);

  std::string serviceName;

  if (_sdf->HasElement("main_switch_srvs"))
  {
    serviceName = _sdf->Get<std::string>("main_switch_srvs");
  }
  else
  {
    serviceName = "light_control";
  }

  // Make sure the ROS node for Gazebo has already been initialized
  if (!ros::isInitialized())
  {
    ROS_FATAL_STREAM("A ROS node for Gazebo has not been initialized, unable to load plugin. "
      << "Load the Gazebo system plugin 'libgazebo_ros_api_plugin.so' in the gazebo_ros package)");
    return;
  }

  printf("Plugin Loaded: ROSFlashLightPlugin\n");

  // ROS service to receive a command to control the light
  ros::NodeHandle n;
  this->service
    = n.advertiseService(serviceName, &LightControlPlugin::Control, this);
}

//////////////////////////////////////////////////
bool LightControlPlugin::Control(
  std_srvs::SetBool::Request &req,
  std_srvs::SetBool::Response &res)
{
  if(req.data)
  {
    res.success = this->TurnOnAll();
  }
  else
  {
    res.success = this->TurnOffAll();
  }

  return res.success;
}

// Register this plugin with the simulator
GZ_REGISTER_MODEL_PLUGIN(LightControlPlugin)
