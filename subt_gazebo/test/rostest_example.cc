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

#include <gazebo/gazebo.hh>
#include <gazebo/gazebo_client.hh>
#include <gazebo/msgs/msgs.hh>
#include <gazebo/transport/transport.hh>
#include <gtest/gtest.h>
#include "ros/ros.h"
#include "test/test_config.h"

using namespace gazebo;
using namespace subt;

/////////////////////////////////////////////////
/// \brief A fixture class for testing.
class ROSTestExample : public testing::Test
{
  // Constructor.
  public: ROSTestExample(): testing::Test()
  {
  }

  // Destructor.
  public: virtual ~ROSTestExample()
  {
  }
};

/////////////////////////////////////////////////
TEST_F(ROSTestExample, switchOffAndOn)
{
  // ROS spinning
  std::shared_ptr<ros::AsyncSpinner> async_ros_spin_;
  async_ros_spin_.reset(new ros::AsyncSpinner(0));
  async_ros_spin_->start();

  // Initialize the transport node
  transport::NodePtr node = transport::NodePtr(new transport::Node());
  node->Init();

  node->Fini();
}

/////////////////////////////////////////////////
int main(int argc, char **argv)
{
  initGazeboEnv();

  ::testing::InitGoogleTest(&argc, argv);

  // Start ROS
  ros::init(argc,argv,"rostest_example");

  // Start Gazebo client
  gazebo::client::setup(argc, argv);

  return RUN_ALL_TESTS();
}
