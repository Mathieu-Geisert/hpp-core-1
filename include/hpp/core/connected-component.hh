//
// Copyright (c) 2014 CNRS
// Authors: Florent Lamiraux
//
// This file is part of hpp-core
// hpp-core is free software: you can redistribute it
// and/or modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation, either version
// 3 of the License, or (at your option) any later version.
//
// hpp-core is distributed in the hope that it will be
// useful, but WITHOUT ANY WARRANTY; without even the implied warranty
// of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// General Lesser Public License for more details.  You should have
// received a copy of the GNU Lesser General Public License along with
// hpp-core  If not, see
// <http://www.gnu.org/licenses/>.

#ifndef HPP_CORE_CONNECTED_COMPONENT_HH
# define HPP_CORE_CONNECTED_COMPONENT_HH

# include <hpp/core/fwd.hh>
# include <hpp/core/config.hh>
# include <hpp/core/node.hh>

namespace hpp {
  namespace core {
    /// Connected component
    ///
    /// Set of nodes reachable from one another.
    class HPP_CORE_DLLAPI ConnectedComponent {
    public:
      // List of nodes within the connected component
      typedef std::list <NodePtr_t> Nodes_t;

      /// Create connected component and return shared pointer
      static ConnectedComponentPtr_t create ()
      {
	ConnectedComponent* ptr = new ConnectedComponent ();
	ConnectedComponentPtr_t shPtr (ptr);
	ptr->init (shPtr);
	return shPtr;
      }

      /// Get list of connected component reachable from this one
      const ConnectedComponents_t& reachableTo () const
      {
	return reachableTo_;
      }
      /// Get list of connected components that can reach this one
      const ConnectedComponents_t& reachableFrom () const
      {
	return reachableFrom_;
      }
      /// Merge two connected components.
      ///
      /// \param other connected component to merge into this one.
      /// \note other will be empty after calling this method.
      void merge (const ConnectedComponentPtr_t& other)
      {
	for (Nodes_t::iterator itNode = other->nodes_.begin ();
	     itNode != other->nodes_.end (); itNode++) {
	  (*itNode)->connectedComponent (weak_.lock ());
	}

	nodes_.splice (nodes_.end (), other->nodes_);
      }
      /// Add node in connected component
      /// \param node node to add.
      void addNode (const NodePtr_t& node)
      {
	nodes_.push_back (node);
      }
      /// Access to the nodes
      const Nodes_t& nodes () const
      {
	return nodes_;
      }
    protected:
      /// Constructor
      ConnectedComponent () : nodes_ (), weak_ ()
      {
      }
      void init (const ConnectedComponentPtr_t& shPtr){
	weak_ = shPtr;
      }
    private:
      // List of nodes
       Nodes_t nodes_;
      // List of connected components that can be reached from this connected
      // component
      ConnectedComponents_t reachableTo_;
      // List of connected components from which this connected component can
      // be reached
      ConnectedComponents_t reachableFrom_;
      ConnectedComponentWkPtr_t weak_;
      friend class Roadmap;
    }; // class ConnectedComponent
  } //   namespace core
} // namespace hpp
#endif // HPP_CORE_CONNECTED_COMPONENT_HH
