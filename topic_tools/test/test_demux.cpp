// Copyright 2024 Rufus Wong
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <chrono>
#include <memory>
#include <string>

#include "gtest/gtest.h"
#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/string.hpp"
#include "test_topic_tool.hpp"
#include "topic_tools/demux_node.hpp"

class DemuxTest : public TestTopicToolMultiPub
{
public:
  void SetUp()
  {
    TestTopicToolMultiPub::SetUp();

    auto options = rclcpp::NodeOptions{};
    options.append_parameter_override("initial_topic", get_target_output_topics()[0]);
    options.append_parameter_override("input_topic", get_target_input_topic());
    options.append_parameter_override("output_topics", get_target_output_topics());
    target_node_ = std::make_shared<topic_tools::DemuxNode>(options);

    srv_client_ =
      test_node_->create_client<topic_tools_interfaces::srv::DemuxSelect>("/demux/select");

    std::function<void(std_msgs::msg::String::ConstSharedPtr)> validator =
      [](std_msgs::msg::String::ConstSharedPtr msg) {ASSERT_EQ(msg->data, "not dropped");};
    set_msg_validator(validator);
  }

  void change_topic(int topic_index)
  {
    using namespace std::chrono_literals;

    auto request = std::make_shared<topic_tools_interfaces::srv::DemuxSelect::Request>();
    request->topic = get_target_output_topics()[topic_index];

    while (!srv_client_->wait_for_service(1s)) {
      if (!rclcpp::ok()) {
        return;
      }
    }

    auto result = srv_client_->async_send_request(request);
    rclcpp::spin_some(target_node_);
  }

  void publish_and_check(std::string msg_content)
  {
    TestTopicToolMultiPub::publish_and_check(msg_content, target_node_);
  }

private:
  rclcpp::Client<topic_tools_interfaces::srv::DemuxSelect>::SharedPtr srv_client_;
  std::shared_ptr<rclcpp::Node> target_node_;
};

TEST_F(DemuxTest, MessagesToTheSelectedTopicArrive)
{
  publish_and_check("not dropped");

  ASSERT_EQ(get_received_msgs(0), 1);
}

TEST_F(DemuxTest, MessagesToNonSelectedTopicDontArrive)
{
  publish_and_check("not dropped");

  ASSERT_EQ(get_received_msgs(1), 0);
}

TEST_F(DemuxTest, SwitchingTopicsWorks)
{
  publish_and_check("not dropped");
  ASSERT_EQ(get_received_msgs(1), 0);
  change_topic(1);
  publish_and_check("not dropped");
  ASSERT_EQ(get_received_msgs(1), 1);
}
