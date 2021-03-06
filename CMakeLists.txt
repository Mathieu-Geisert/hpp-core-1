#
# Copyright (c) 2010, 2011 CNRS
# Authors: Florent Lamiraux
#
#
# This file is part of hpp-core
# hpp-core is free software: you can redistribute it
# and/or modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation, either version
# 3 of the License, or (at your option) any later version.
#
# hpp-core is distributed in the hope that it will be
# useful, but WITHOUT ANY WARRANTY; without even the implied warranty
# of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# General Lesser Public License for more details.  You should have
# received a copy of the GNU Lesser General Public License along with
# hpp-core  If not, see
# <http://www.gnu.org/licenses/>.

CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

SET(CXX_DISABLE_WERROR TRUE)
INCLUDE(cmake/base.cmake)
INCLUDE(cmake/cpack.cmake)

SET(PROJECT_NAME hpp-core)
SET(PROJECT_DESCRIPTION
  "Implement basic classes for canonical path planning for kinematic chains."
  )
SET(PROJECT_URL "")

# Where to compile shared objects
SET(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)

SETUP_PROJECT()

# Activate hpp-util logging if requested
SET (HPP_DEBUG FALSE CACHE BOOL "trigger hpp-util debug output")
IF (HPP_DEBUG)
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DHPP_DEBUG")
ENDIF()

# Declare Headers
SET(${PROJECT_NAME}_HEADERS
  include/hpp/core/basic-configuration-shooter.hh
  include/hpp/core/configuration-shooter.hh
  include/hpp/core/config-projector.hh
  include/hpp/core/connected-component.hh
  include/hpp/core/constraint.hh
  include/hpp/core/constraint-set.hh
  include/hpp/core/differentiable-function.hh
  include/hpp/core/diffusing-planner.hh
  include/hpp/core/discretized-collision-checking.hh
  include/hpp/core/distance.hh
  include/hpp/core/edge.hh
  include/hpp/core/fwd.hh
  include/hpp/core/locked-dof.hh
  include/hpp/core/node.hh
  include/hpp/core/path.hh
  include/hpp/core/path-optimizer.hh
  include/hpp/core/path-planner.hh
  include/hpp/core/path-validation.hh
  include/hpp/core/path-vector.hh
  include/hpp/core/plan-and-optimize.hh
  include/hpp/core/problem.hh
  include/hpp/core/problem-solver.hh
  include/hpp/core/random-shortcut.hh
  include/hpp/core/roadmap.hh
  include/hpp/core/steering-method.hh
  include/hpp/core/steering-method-straight.hh
  include/hpp/core/straight-path.hh
  include/hpp/core/weighed-distance.hh
  include/hpp/core/k-d-tree.hh
)

ADD_REQUIRED_DEPENDENCY("hpp-util >= 3")
ADD_REQUIRED_DEPENDENCY("hpp-model >= 3")
ADD_REQUIRED_DEPENDENCY("roboptim-trajectory >= 1.0")

# Add dependency toward hpp-model library in pkg-config file.
PKG_CONFIG_APPEND_LIBS("hpp-core")

ADD_SUBDIRECTORY(src)
ADD_SUBDIRECTORY(tests)

SETUP_PROJECT_FINALIZE()
SETUP_PROJECT_CPACK()
