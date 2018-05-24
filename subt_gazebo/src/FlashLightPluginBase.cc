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

#include "FlashLightPluginBase.hh"

using namespace gazebo;

// static members
transport::NodePtr FlashLightSettings::node;
transport::PublisherPtr FlashLightSettings::pubLight;

//////////////////////////////////////////////////
void FlashLightSettings::Flash()
{
  // Make a message
  msgs::Light msg;
  msg.set_name(this->link->GetScopedName() + "::" + this->name);
  msg.set_range(this->range);
  // Send the message
  FlashLightSettings::pubLight->Publish(msg);
  // Update the state
  this->f_flashing = true;
}

//////////////////////////////////////////////////
void FlashLightSettings::Dim()
{
  // Make a message
  msgs::Light msg;
  msg.set_name(this->link->GetScopedName() + "::" + this->name);
  msg.set_range(0.0);
  // Send the message
  FlashLightSettings::pubLight->Publish(msg);
  // Update the state
  this->f_flashing = false;
}

//////////////////////////////////////////////////
FlashLightSettings::FlashLightSettings(
  const physics::ModelPtr _model,
  const sdf::ElementPtr _sdfFlashLight,
  const common::Time _current_time)
{
  if (!FlashLightSettings::node)
  {
    // Create a node
    FlashLightSettings::node = transport::NodePtr(new transport::Node());
    FlashLightSettings::node->Init();

    // advertise the topic to update lights
    FlashLightSettings::pubLight =
      FlashLightSettings::node
      ->Advertise<gazebo::msgs::Light>("~/light/modify");

    FlashLightSettings::pubLight->WaitForConnection();
  }

  // name
  this->name = _sdfFlashLight->Get<std::string>("light_name");
  // link which holds this light
  this->link = _model->GetLink(_sdfFlashLight->Get<std::string>("link_name"));
  // start time
  this->start_time = _current_time;
  // current states
  this->f_switch_on = true;
  this->f_flashing = true;
  // duration
  if (_sdfFlashLight->HasElement("duration"))
  {
    this->duration = _sdfFlashLight->Get<double>("duration");
  }
  else
  {
    this->interval = 0.5;
  }
  // interval
  if (_sdfFlashLight->HasElement("interval"))
  {
    this->interval = _sdfFlashLight->Get<double>("interval");
  }
  else
  {
    this->interval = 0.5;
  }
  // color
  if (_sdfFlashLight->HasElement("color"))
  {
    sdf::ElementPtr sdfColor = _sdfFlashLight->GetElement("color");
    if (sdfColor->HasElement("R"))
    {
      this->color.R(sdfColor->Get<double>("R"));
    }
    else
    {
      gzerr << "flash_light <" + this->name + "> does not have"
        + "R value in its color field." << std::endl;
    }
    if (sdfColor->HasElement("G"))
    {
      this->color.G(sdfColor->Get<double>("G"));
    }
    else
    {
      gzerr << "flash_light <" + this->name + "> does not have"
        + "G value in its color field." << std::endl;
    }
    if (sdfColor->HasElement("B"))
    {
      this->color.B(sdfColor->Get<double>("B"));
    }
    else
    {
      gzerr << "flash_light <" + this->name + "> does not have"
        + "B value in its color field." << std::endl;
    }
    this->color.A(1.0);
  }
  else
  {
    this->color = ignition::math::Color(1.0, 1.0, 1.0, 1.0);
  }
  // range
  if (_sdfFlashLight->HasElement("range"))
  {
    this->range = _sdfFlashLight->Get<double>("range");
  }
  else
  {
    this->range = 30.0;
  }
  // angle
  if (_sdfFlashLight->HasElement("angle"))
  {
    this->angle = _sdfFlashLight->Get<double>("angle");
  }
  else
  {
    this->angle = 60;
  }

  // Initialize the light in the environment
  // Make a message
  msgs::Light msg;
  msg.set_name(this->link->GetScopedName() + "::" + this->name);
  // type is automatically set to spot
  msg.set_type(msgs::Light::SPOT);
  msgs::Set(msg.mutable_diffuse(), this->color);
  // specular is set to black automatically
  msgs::Set(msg.mutable_specular(), ignition::math::Color(0.0, 0.0, 0.0, 1.0));
  msg.set_range(this->range);
  ignition::math::Angle outer_rad, inner_rad;
  double outer_deg = this->angle;
  double inner_deg = outer_deg - 5.0;
  if (inner_deg < 0)
    inner_deg = 0;
  outer_rad.Degree(outer_deg);
  inner_rad.Degree(inner_deg);
  msg.set_spot_outer_angle(outer_rad.Radian());
  msg.set_spot_inner_angle(inner_rad.Radian());
  // cast_shadow is set to true
  msg.set_cast_shadows(true);
  // fall_off is set to 1.0
  msg.set_spot_falloff(1.0);

  // Send the message to initialize the light
  FlashLightSettings::pubLight->Publish(msg);
}

//////////////////////////////////////////////////
void FlashLightSettings::UpdateLightInEnv(const common::Time _current_time)
{
  // reset the time at the and of each phase
  if (_current_time.Double() - this->start_time.Double()
        > this->duration + this->interval)
  {
    this->start_time = _current_time;
  }

  if (this->f_switch_on)
  {
    // time to dim
    if (_current_time.Double() - this->start_time.Double()
          > this->duration)
    {
      if (this->f_flashing == true)
      {
        this->Dim();
      }
    }
    // time to flash
    else
    {
      if (this->f_flashing == false)
      {
        this->Flash();
      }
    }
  }
  else if (this->f_flashing)
  {
    this->Dim();
  }
}

//////////////////////////////////////////////////
FlashLightSettings& FlashLightSettings::operator=(
  const FlashLightSettings &_settings)
{
  this->name = _settings.name;
  this->link = _settings.link;
  this->start_time = _settings.start_time;
  this->f_switch_on = _settings.f_switch_on;
  this->f_flashing = _settings.f_flashing;
  this->duration = _settings.duration;
  this->interval = _settings.interval;
  this->color = _settings.color;
  this->range = _settings.range;
  this->angle = _settings.angle;

  return *this;
}

//////////////////////////////////////////////////
const std::string FlashLightSettings::GetName() const
{
  return this->name;
}

//////////////////////////////////////////////////
const physics::LinkPtr FlashLightSettings::GetLink() const
{
  return this->link;
}

//////////////////////////////////////////////////
void FlashLightSettings::SwitchOn()
{
  this->f_switch_on = true;
}

//////////////////////////////////////////////////
void FlashLightSettings::SwitchOff()
{
  this->f_switch_on = false;
}

//////////////////////////////////////////////////
void FlashLightPluginBase::Load(physics::ModelPtr _parent, sdf::ElementPtr _sdf)
{
  // Store the pointers to the model and world
  this->model = _parent;
  this->world = _parent->GetWorld();

  // get the current time
  common::Time current_time = this->world->SimTime();

  sdf::ElementPtr sdfFlashLight = _sdf->GetElement("flash_light");
  while (sdfFlashLight)
  {
    // link_name and light_name are required
    if (sdfFlashLight->HasElement("link_name")
      && sdfFlashLight->HasElement("light_name"))
    {
      // Get the setting information from sdf
      std::shared_ptr<FlashLightSettings> settings
        = std::make_shared<FlashLightSettings>(this->model,
                                               sdfFlashLight,
                                               current_time);

      // Store the settings to the list
      this->list_flash_light.push_back(settings);
    }
    else
    {
      // display an error message
      gzerr << "no name field exists in <flash_light>" << std::endl;
    }

    sdfFlashLight = sdfFlashLight->GetNextElement("flash_light");
  }

  // listen to the update event by the World
  this->updateConnection = event::Events::ConnectWorldUpdateBegin(
    std::bind(&FlashLightPluginBase::OnUpdate, this));
}

//////////////////////////////////////////////////
void FlashLightPluginBase::OnUpdate()
{
  common::Time current_time = this->world->SimTime();

  std::vector<std::shared_ptr<FlashLightSettings>>::iterator it
    = this->list_flash_light.begin();

  while (it != this->list_flash_light.end())
  {
    std::shared_ptr<FlashLightSettings> set
      = (std::shared_ptr<FlashLightSettings>)*it;

    /// update the light
    set->UpdateLightInEnv(current_time);

    ++it;
  }
}

//////////////////////////////////////////////////
bool FlashLightPluginBase::TurnOn(const std::string _light_name)
{
  return this->TurnOn(_light_name, "");
}

//////////////////////////////////////////////////
bool FlashLightPluginBase::TurnOn(
  const std::string _light_name, const std::string _link_name)
{
  bool f_found = false;

  std::vector<std::shared_ptr<FlashLightSettings>>::iterator it;
  for (it = this->list_flash_light.begin();
    it != this->list_flash_light.end(); ++it)
  {
    std::shared_ptr<FlashLightSettings> set
      = (std::shared_ptr<FlashLightSettings>)*it;

    if (set->GetName().compare(_light_name) == 0)
    {
      if (_link_name.length() == 0
        || set->GetLink()->GetName().compare(_link_name) == 0)
      {
        set->SwitchOn();
        f_found = true;
      }
    }
  }

  if (f_found == false)
    gzerr << "light <" + _light_name + "> does not exist." << std::endl;

  return f_found;
}

//////////////////////////////////////////////////
bool FlashLightPluginBase::TurnOnAll()
{
  bool f_found = false;

  std::vector<std::shared_ptr<FlashLightSettings>>::iterator it;
  for (it = this->list_flash_light.begin();
    it != this->list_flash_light.end(); ++it)
  {
    std::shared_ptr<FlashLightSettings> set
      = (std::shared_ptr<FlashLightSettings>)*it;

    set->SwitchOn();
    f_found = true;
  }

  if (f_found == false)
    gzerr << "no flash lights exist to turn on." << std::endl;

  return f_found;
}

//////////////////////////////////////////////////
bool FlashLightPluginBase::TurnOff(const std::string _light_name)
{
  return this->TurnOff(_light_name, "");
}

//////////////////////////////////////////////////
bool FlashLightPluginBase::TurnOff(const std::string _light_name,
  const std::string _link_name)
{
  bool f_found = false;

  std::vector<std::shared_ptr<FlashLightSettings>>::iterator it;
  for (it = this->list_flash_light.begin();
    it != this->list_flash_light.end(); ++it)
  {
    std::shared_ptr<FlashLightSettings> set
      = (std::shared_ptr<FlashLightSettings>)*it;

    if (set->GetName().compare(_light_name) == 0)
    {
      if (_link_name.length() == 0
        || set->GetLink()->GetName().compare(_link_name))
      {
        set->SwitchOff();
        f_found = true;
      }
    }
  }

  if (f_found == false)
    gzerr << "light <" + _light_name + "> does not exist." << std::endl;

  return f_found;
}

//////////////////////////////////////////////////
bool FlashLightPluginBase::TurnOffAll()
{
  bool f_found = false;

  std::vector<std::shared_ptr<FlashLightSettings>>::iterator it;
  for (it = this->list_flash_light.begin();
    it != this->list_flash_light.end(); ++it)
  {
    std::shared_ptr<FlashLightSettings> set
      = (std::shared_ptr<FlashLightSettings>)*it;

    set->SwitchOff();
    f_found = true;
  }

  if (f_found == false)
    gzerr << "no flash lights exist to turn off." << std::endl;

  return f_found;
}

