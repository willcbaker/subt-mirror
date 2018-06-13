#!/usr/bin/env python

# Copyright (C) 2018 Open Source Robotics Foundation
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

from __future__ import print_function

import argparse
import copy
import math
import os
import random
import subprocess
import sys

import em
import rospkg
import yaml

rospack = rospkg.RosPack()
world_dir = os.path.join(rospack.get_path('subt_gazebo'), 'worlds')
launch_dir = os.path.join(rospack.get_path('subt_gazebo'), 'launch')
template_files = [
#    os.path.join(world_dir, 'subt_gazebo.world.template'),
    os.path.join(launch_dir, 'subt_gazebo.launch.template'),
#    os.path.join(launch_dir, 'subt_gazebo.urdf.xacro.template'),
]

# Prepares a parser for arguments.
# You can see the acceptable options to this python script.
def prepare_arguments(parser):
    add = parser.add_argument
    add('-n', '--dry-run', action='store_true', default=False,
        help='print generated files to stdout, but do not write them to disk')
    add('-v', '--verbose', action='store_true', default=False,
        help='output additional logging to console')
    add('-o', '--output', default='/tmp/subt_gazebo/',
        help='directory in which to output the generated files')
    add('--development-mode', '-d', action='store_true', default=False,
        help='if true the competition mode environment variable will not be set (default false)')
    add('--no-gui', action='store_true', default=False,
        help="don't run the gazebo client gui")
    add('-w', '--world', nargs=1, help='a path to a specific world file')

    add('-l', '--state-logging', action='store', type=str2bool, nargs='?',
        help='generate gazebo state logs (will override config file option)')
    add('--visualize-sensor-views', action='store_true', default=False,
        help='visualize the views of sensors in gazebo')
    mex_group = parser.add_mutually_exclusive_group(required=False)
    add = mex_group.add_argument
    add('config', nargs='?', metavar='CONFIG',
        help='yaml string that is the configuration')
    add('-f', '--file', nargs='+', help='list of paths to yaml files that contain the '
        'configuration (contents will be concatenated)')

# Default settings
configurable_options = {
    'gazebo_state_logging': False,
    'visualize_sensor_views': False,
}
default_time_limit = 500 # seconds
default_world_file_path = os.path.join(world_dir, 'subt_gazebo.world')
default_template_data = {
    'options': {},
    'time_limit': default_time_limit,
    'world_dir': world_dir,
}

# Make data to generate files based on the yaml and arguments.
def prepare_template_data(config_dict, args):
    template_data = copy.copy(default_template_data)
    # Process the options first as they may affect the processing of the rest
    options_dict = get_field_with_default(config_dict, 'options', {})
    template_data['options'].update(create_options_info(options_dict))
    if args.state_logging is not None:
        template_data['options']['gazebo_state_logging'] = args.state_logging
    if args.visualize_sensor_views:
        template_data['options']['visualize_sensor_views'] = True

    for key, value in config_dict.items():
        if key == 'options':
            pass
        elif key == 'time_limit':
            template_data['time_limit'] = value
        else:
            print("Error: unknown top level entry '{0}'".format(key), file=sys.stderr)
            sys.exit(1)
    return template_data

# Helper for converting strings to booleans; copied from https://stackoverflow.com/a/43357954
def str2bool(v):
    if v.lower() in ('yes', 'true', 't', 'y', '1'):
        return True
    elif v.lower() in ('no', 'false', 'f', 'n', '0'):
        return False
    else:
        raise argparse.ArgumentTypeError('Boolean value expected.')

def create_options_info(options_dict):
    options = configurable_options
    for option, val in options_dict.items():
        options[option] = val
    return options

def get_field_with_default(data_dict, entry, default_value):
    if entry in data_dict:
        return data_dict[entry]
    else:
        return default_value

def generate_files(template_data):
    files = {}
    for template_file in template_files:
        with open(template_file, 'r') as f:
            data = f.read()
        files[template_file] = em.expand(data, template_data)
    return files

def expand_to_float(val):
    if isinstance(val, str):
        return float(eval(val, {}, eval_local_vars))
    return float(val)


def expand_yaml_substitutions(yaml_dict):
    for k, v in yaml_dict.items():
        if isinstance(v, dict):
            yaml_dict[k] = expand_yaml_substitutions(v)
        if k in ['xyz', 'rpy']:
            yaml_dict[k] = [expand_to_float(x) for x in v]
        if k in ['initial_joint_states']:
            yaml_dict[k] = {kp: expand_to_float(vp) for kp, vp in v.items()}
    return yaml_dict

def main(sysargv=None):
    parser = argparse.ArgumentParser(
        description='Prepares and then executes a gazebo simulation based on configurations.')
    prepare_arguments(parser)
    args = parser.parse_args(sysargv)
    print ('done?')

    config_data = args.config or ''
    if args.file is not None:
        for file in args.file:
            with open(file, 'r') as f:
                comp_config_data = f.read()
                config_data += comp_config_data
    dict_config = yaml.load(config_data) or {}

    expanded_dict_config = expand_yaml_substitutions(dict_config)

    if args.verbose:
        print(yaml.dump({'Using configuration': expanded_dict_config}))

    random_seed = expanded_dict_config.pop('random_seed', None)

    # === generate a world file here? === #

    if args.world:
        world_file_path = args.world[0]
    else:
        world_file_path = default_world_file_path

    # Generate files based on the given yaml files and arguments.
    template_data = prepare_template_data(expanded_dict_config, args)
    files = generate_files(template_data)

    # Write the files into a temporary directory.
    if not args.dry_run and not os.path.isdir(args.output):
        if os.path.exists(args.output) and not os.path.isdir(args.output):
            print('Error, given output directory exists but is not a directory.', file=sys.stderr)
            sys.exit(1)
        print('creating directory: ' + args.output)
        os.makedirs(args.output)
    for name, content in files.items():
        if name.endswith('.template'):
            name = name[:-len('.template')]
        name = os.path.basename(name)
        if args.dry_run:
            print('# file: ' + name)
            print(content)
        else:
            file_path = os.path.join(args.output, name)
            print('writing file ' + file_path)
            with open(file_path, 'w+') as f:
                f.write(content)
        if name.endswith('.world'):
            world_file_path = os.path.join(args.output, name)

    # Make a command to run Gazebo
    cmd = [
        'roslaunch',
        os.path.join(args.output, 'subt_gazebo.launch'),
        'world_path:=' + world_file_path,
#        'gear_urdf_xacro:=' + os.path.join(args.output, 'subt_gazebo.urdf.xacro'),
    ]
    # additional arguments for the launch file.
    if args.verbose:
        cmd += ['verbose:=true']
    if args.no_gui:
        cmd += ['gui:=false']

    if not args.development_mode:
        os.environ['SUBT_COMPETITION'] = '1'

    # Run the simulator.
    print('Running command: ' + ' '.join(cmd))
    if not args.dry_run:
        try:
            p = subprocess.Popen(cmd)
            p.wait()
        except KeyboardInterrupt:
            pass
        finally:
            p.wait()
        return p.returncode

if __name__ == '__main__':
    # Filter out any special ROS remapping arguments.
    # This is necessary if the script is being run from a ROS launch file.
    import rospy
    filtered_argv = rospy.myargv(sys.argv)

    sys.exit(main(filtered_argv[1:]))
