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

#include <memory>
#include <string>
#include <vector>

#include "rclcpp/rclcpp.hpp"
#include "topic_tools/demux_node.hpp"

int main(int argc, char * argv[])
{
  auto args = rclcpp::init_and_remove_ros_arguments(argc, argv);
  auto options = rclcpp::NodeOptions{};

  if (args.size() < 3) {
    RCLCPP_ERROR(
        rclcpp::get_logger("demux"),
        "Incorect number of arguments. "
        "Usage: "
        "ros2 run topic_tools demux <intopic> <outtopic1> [outtopic2...]");
    return 1;
  }

  options.append_parameter_override("input_topic", args.at(1));
  options.append_parameter_override(
      "output_topics", std::vector<std::string>{args.begin() + 2, args.end()});

  auto node = std::make_shared<topic_tools::DemuxNode>(options);

  rclcpp::spin(node);
  rclcpp::shutdown();
  return 0;
}
