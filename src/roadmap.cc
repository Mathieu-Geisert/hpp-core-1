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

#include <algorithm>
#include <hpp/util/debug.hh>
#include <hpp/core/connected-component.hh>
#include <hpp/core/edge.hh>
#include <hpp/core/node.hh>
#include <hpp/core/path.hh>
#include <hpp/core/roadmap.hh>
#include "nearest-neighbor.hh"
#include <hpp/core/k-d-tree.hh>

namespace hpp {
  namespace core {

    std::string displayConfig (ConfigurationIn_t q)
    {
      std::ostringstream oss;
      for (size_type i=0; i < q.size (); ++i) {
	oss << q [i] << ",";
      }
      return oss.str ();
    }

    RoadmapPtr_t Roadmap::create (const DistancePtr_t& distance, const DevicePtr_t& robot)
    {
      Roadmap* ptr = new Roadmap (distance, robot);
      return RoadmapPtr_t (ptr);
    }

    Roadmap::Roadmap (const DistancePtr_t& distance, const DevicePtr_t& robot) :
      distance_ (distance), connectedComponents_ (), nodes_ (), edges_ (),
      initNode_ (), goalNodes_ (),
      //nearestNeighbor_ (),
      kdTree_(robot, distance, 30)
    {
    }

    Roadmap::~Roadmap ()
    {
      clear ();
    }

    void Roadmap::clear ()
    {
      connectedComponents_.clear ();

      for (Nodes_t::iterator it = nodes_.begin (); it != nodes_.end (); it++) {
	delete *it;
      }
      nodes_.clear ();

      for (Edges_t::iterator it = edges_.begin (); it != edges_.end (); it++) {
	delete *it;
      }
      edges_.clear ();

      goalNodes_.clear ();
      initNode_ = 0x0;
      //nearestNeighbor_.clear ();
      kdTree_.clear();
    }

    NodePtr_t Roadmap::addNode (const ConfigurationPtr_t& configuration)
    {
      value_type distance;
      if (nodes_.size () != 0) {
	NodePtr_t nearest = nearestNode (configuration, distance);
	if (*(nearest->configuration ()) == *configuration) {
	  return nearest;
	}
	if (distance < 1e-4) {
	  throw std::runtime_error ("distance to nearest node too small");
	}
      }
      NodePtr_t node = new Node (configuration);
      hppDout (info, "Added node: " << displayConfig (*configuration));
      nodes_.push_back (node);
      // Node constructor creates a new connected component. This new
      // connected component needs to be added in the roadmap and the
      // new node needs to be registered in the connected component.
      addConnectedComponent (node);
      return node;
    }

    NodePtr_t Roadmap::addNode (const ConfigurationPtr_t& configuration,
				ConnectedComponentPtr_t connectedComponent)
    {
      assert (connectedComponent);
      value_type distance;
      if (nodes_.size () != 0) {
	NodePtr_t nearest = nearestNode (configuration, connectedComponent,
					 distance);
	if (*(nearest->configuration ()) == *configuration) {
	  return nearest;
	}
	if (distance < 1e-4) {
	  throw std::runtime_error ("distance to nearest node too small");
	}
      }
      NodePtr_t node = new Node (configuration, connectedComponent);
      hppDout (info, "Added node: " << displayConfig (*configuration));
      nodes_.push_back (node);
      // The new node needs to be registered in the connected
      // component.
      connectedComponent->addNode (node);
	//nearestNeighbor_ [connectedComponent]->add (node);
	kdTree_.addNode(node);
      return node;
    }

    NodePtr_t Roadmap::addNodeAndEdge (const NodePtr_t from,
				       const ConfigurationPtr_t& to,
				       const PathPtr_t path)
    {
      NodePtr_t nodeTo = addNode (to, from->connectedComponent ());
      addEdge (from, nodeTo, path);
      addEdge (nodeTo, from, path->reverse ());
      return nodeTo;
    }

    NodePtr_t
    Roadmap::nearestNode (const ConfigurationPtr_t& configuration,
			  value_type& minDistance)
    {
	NodePtr_t closest;
	minDistance = std::numeric_limits<value_type>::infinity ();
	for (ConnectedComponents_t::const_iterator itcc =
	       connectedComponents_.begin ();
	     itcc != connectedComponents_.end (); itcc++) {
	  value_type distance;
	  NodePtr_t node;
	  //node = nearestNeighbor_ [*itcc]->nearest (configuration, distance);
	  node = kdTree_.search(configuration, *itcc, distance);
	  if (distance < minDistance) {
	    minDistance = distance;
	    closest = node;
	  }

	}
      return closest;
    }

    NodePtr_t
    Roadmap::nearestNode (const ConfigurationPtr_t& configuration,
			  const ConnectedComponentPtr_t& connectedComponent,
			  value_type& minDistance)
    {
      assert (connectedComponent);
      //return nearestNeighbor_ [connectedComponent]->nearest (configuration,
	//						     minDistance);
      return kdTree_.search(configuration, connectedComponent, minDistance);
    }

    void Roadmap::addGoalNode (const ConfigurationPtr_t& config)
    {
      NodePtr_t node = addNode (config);
      goalNodes_.push_back (node);
    }

    const DistancePtr_t& Roadmap::distance () const
    {
      return distance_;
    }

    EdgePtr_t Roadmap::addEdge (const NodePtr_t& n1, const NodePtr_t& n2,
				const PathPtr_t& path)
    {
      EdgePtr_t edge = new Edge (n1, n2, path);
      n1->addOutEdge (edge);
      n2->addInEdge (edge);
      edges_.push_back (edge);
      hppDout (info, "Added edge between: " <<
	       displayConfig (*(n1->configuration ())));
      hppDout (info, "               and: " <<
	       displayConfig (*(n2->configuration ())));
      // If node connected components are different, merge them
      ConnectedComponentPtr_t cc1 = n1->connectedComponent ();
      ConnectedComponentPtr_t cc2 = n2->connectedComponent ();
      
      if (cc1 != cc2) {
          //Check and update reachability of the connected components
          updateCCReachability(cc1,cc2);
      }
      return edge;
    }

    void Roadmap::addConnectedComponent (const NodePtr_t& node)
    {
      connectedComponents_.push_back (node->connectedComponent ());
      //nearestNeighbor_ [node->connectedComponent ()] =
	//NearestNeighborPtr_t (new NearestNeighbor (distance_));
      node->connectedComponent ()->addNode (node);
      //nearestNeighbor_ [node->connectedComponent ()]->add (node);
      kdTree_.addNode(node);
    }
    void Roadmap::updateCCReachability(const ConnectedComponentPtr_t& cc1,
            const ConnectedComponentPtr_t& cc2)
    {
        ConnectedComponents_t::iterator itcc1 = 
            std::find (cc1->reachableTo_.begin(),cc1->reachableTo_.end(),
                    cc2);
        ConnectedComponents_t::iterator itcc2 = 
            std::find (cc2->reachableTo_.begin(),cc2->reachableTo_.end(),
                    cc1);
        bool cc1Tocc2 = (itcc1 != cc1->reachableTo_.end())?
            true:false;
        bool cc2Tocc1 = (itcc2 != cc2->reachableTo_.end())?
            true:false;
        //If both components are reachable to each other, merge them
        if (cc1Tocc2 && cc2Tocc1) {
            cc1->merge (cc2);
            //Merge the connected components in kdTree
            kdTree_.merge(cc1, cc2);
            // Remove cc2 from list of connected components
            ConnectedComponents_t::iterator itcc =
                std::find (connectedComponents_.begin (), connectedComponents_.end (),
                        cc2);
            assert (itcc != connectedComponents_.end ());
            connectedComponents_.erase (itcc);
        }
        else {
            ConnectedComponents_t::iterator itcc; 
            if (cc1Tocc2) {
                itcc = cc1->reachableTo_.end();
                cc1->reachableTo_.splice (itcc, cc2->reachableTo_);
                itcc = cc2->reachableFrom_.end();
                cc2->reachableFrom_.splice (itcc, cc1->reachableFrom_);
            }
            if (cc2Tocc1) {
                itcc = cc2->reachableTo_.end();
                cc2->reachableTo_.splice (itcc, cc1->reachableTo_);
                itcc = cc1->reachableFrom_.end();
                cc1->reachableFrom_.splice (itcc, cc2->reachableFrom_);
            }
        }
    }
    bool Roadmap::pathExists () const
    {
      const ConnectedComponents_t reachableFromInit =
	initNode ()->connectedComponent ()->reachableTo ();
      for (Nodes_t::const_iterator itGoal = goalNodes ().begin ();
	   itGoal != goalNodes ().end (); itGoal++) {
	if (std::find (reachableFromInit.begin (), reachableFromInit.end (),
		       (*itGoal)->connectedComponent ()) !=
	    reachableFromInit.end ()) {
	  return true;
	}
      }
      return false;
    }
  } //   namespace core
} // namespace hpp

std::ostream& operator<< (std::ostream& os, const hpp::core::Roadmap& r)
{
  using hpp::core::Nodes_t;
  using hpp::core::NodePtr_t;
  using hpp::core::Edges_t;
  using hpp::core::EdgePtr_t;
  using hpp::core::ConnectedComponents_t;
  using hpp::core::ConnectedComponentPtr_t;
  using hpp::core::size_type;

  // Enumerate nodes and connected components
  std::map <NodePtr_t, size_type> nodeId;
  std::map <ConnectedComponentPtr_t, size_type> ccId;

  size_type count = 0;
  for (Nodes_t::const_iterator it = r.nodes ().begin ();
       it != r.nodes ().end (); ++it) {
    nodeId [*it] = count; ++count;
  }

  count = 0;
  for (ConnectedComponents_t::const_iterator it =
	 r.connectedComponents ().begin ();
       it != r.connectedComponents ().end (); ++it) {
    ccId [*it] = count; ++count;
  }

  // Display list of nodes
  os << "----------------------------------------------------------------------"
     << std::endl;
  os << "Roadmap" << std::endl;
  os << "----------------------------------------------------------------------"
     << std::endl;
  os << "----------------------------------------------------------------------"
     << std::endl;
  os << "Nodes" << std::endl;
  os << "----------------------------------------------------------------------"
     << std::endl;
  for (Nodes_t::const_iterator it = r.nodes ().begin ();
       it != r.nodes ().end (); ++it) {
    const NodePtr_t node = *it;
    os << "Node " << nodeId [node] << ": " << *node << std::endl;
  }
  os << "----------------------------------------------------------------------"
     << std::endl;
  os << "Edges" << std::endl;
  os << "----------------------------------------------------------------------"
     << std::endl;
  for (Edges_t::const_iterator it = r.edges ().begin ();
       it != r.edges ().end (); ++it) {
    const EdgePtr_t edge = *it;
    os << "Edge: " << nodeId [edge->from ()] << " -> "
       << nodeId [edge->to ()] << std::endl;
  }
  os << "----------------------------------------------------------------------"
     << std::endl;
  os << "Connected components" << std::endl;
  os << "----------------------------------------------------------------------"
     << std::endl;
  for (ConnectedComponents_t::const_iterator it =
	 r.connectedComponents ().begin ();
       it != r.connectedComponents ().end (); ++it) {
    const ConnectedComponentPtr_t cc = *it;
    os << "Connected component " << ccId [cc] << std::endl;
    os << "Nodes : ";
    for (Nodes_t::const_iterator itNode = cc->nodes ().begin ();
	 itNode != cc->nodes ().end (); ++itNode) {
      os << nodeId [*itNode] << ", ";
    }
    os << std::endl;
    os << "Reachable to :";
    for (ConnectedComponents_t::const_iterator itTo =
	   cc->reachableTo ().begin (); itTo != cc->reachableTo ().end ();
	 ++itTo) {
      os << ccId [*itTo] << ", ";
    }
    os << std::endl;
    os << "Reachable from :";
    for (ConnectedComponents_t::const_iterator itFrom =
	   cc->reachableFrom ().begin (); itFrom != cc->reachableFrom ().end ();
	 ++itFrom) {
      os << ccId [*itFrom] << ", ";
    }
    os << std::endl;
  }
  return os;
}

