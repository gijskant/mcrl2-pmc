// Author(s): Rimco Boudewijns and Sjoerd Cranen
// Copyright: see the accompanying file COPYING or copy at
// https://svn.win.tue.nl/trac/MCRL2/browser/trunk/COPYING
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

/**

  @file graph.h
  @author S. Cranen, R. Boudewijns

  This file contains an interface to the graph data structure used by LTSGraph.

*/

#ifndef GRAPH_H
#define GRAPH_H

#include "mcrl2/lts/lts.h"

#include <cmath>

namespace Graph
{

  struct Coord3D
  {
      GLfloat x;
      GLfloat y;
      GLfloat z;
      Coord3D(GLfloat x, GLfloat y, GLfloat z)
        : x(x), y(y), z(z) {}
      Coord3D() : x(0), y(0), z(0) {}
      Coord3D& operator+=(const Coord3D& a)
      {
        x += a.x;
        y += a.y;
        z += a.z;
        return *this;
      }
      Coord3D& operator-=(const Coord3D& a)
      {
        x -= a.x;
        y -= a.y;
        z -= a.z;
        return *this;
      }

      Coord3D& operator*=(float f)
      {
        x *= f;
        y *= f;
        z *= f;
        return *this;
      }

      Coord3D& operator/=(float f)
      {
        x /= f;
        y /= f;
        z /= f;
        return *this;
      }


      Coord3D operator+(const Coord3D& a) const
      {
        return Coord3D(a.x + x, a.y + y, a.z + z);
      }
      Coord3D operator-(const Coord3D& a) const
      {
        return Coord3D(x - a.x, y - a.y, z - a.z);
      }
      Coord3D operator*(GLfloat c) const
      {
        return Coord3D(c * x, c * y, c * z);
      }
      Coord3D operator-() const
      {
        return Coord3D(-x, -y, -z);
      }
      Coord3D operator/(GLfloat c) const
      {
        return Coord3D(x / c, y / c, z / c);
      }
      float size() const
      {
        return sqrt(x * x + y * y + z * z);
      }
      float dot(const Coord3D& a) const
      {
        return x * a.x + y * a.y + z * a.z;
      }
      Coord3D cross(const Coord3D& a) const
      {
        return Coord3D(
              y * a.z - z * a.y,
              z * a.x - x * a.z,
              x * a.y - y * a.x
              );
      }
      void clip(const Coord3D& min, const Coord3D& max)
      {
        if (x < min.x) x = min.x;
        if (x > max.x) x = max.x;
        if (y < min.y) y = min.y;
        if (y > max.y) y = max.y;
        if (z < min.z) z = min.z;
        if (z > max.z) z = max.z;
      }
      operator const GLfloat*() const { return &x; }
      bool operator==(const Coord3D& other) const
      {
        return x == other.x && y == other.y && z == other.z;
      }
      bool operator!=(const Coord3D& other) const
      {
        return !operator==(other);
      }
  };


  /**
   * @brief A class which contains the information of a single edge.
   */
  class Edge
  {
    protected:
      size_t m_from;      ///< The originating node.
      size_t m_to;        ///< The node pointed at.

    public:
      /// \brief Default constructor
      Edge()=default;

      /// \brief Constructor
      Edge(const size_t from, const size_t to)
       : m_from(from), m_to(to)
      {}
     
      /// \brief Obtain the value of from.
      size_t from() const
      {
        return m_from;
      }
     
      /// \brief Obtain a reference to the value of from.
      size_t& from() 
      {
        return m_from;
      }
      
      /// \brief Obtain the value of to.
      size_t to() const
      {
        return m_to;
      }
     
      /// \brief Obtain a reference to the value of to.
      size_t& to() 
      {
        return m_to;
      }
  };


  /**
   * @brief A class which contains the information of a single node (as in movable object).
   */
  class Node
  {
    protected:
      Coord3D m_pos;            ///< The position of the node.
      bool m_anchored;          ///< Indicates wether this node cannot be moved.
      bool m_locked;            ///< Indicates if anchored should be left true at all times.
      float m_selected;         ///< Indicates that this node is selected (pointed at). Range 0.0f .. 1.0f.

    public:

      /// \brief Default constructor. 
      Node()=default;

      /// \brief Constructor
      Node(const Coord3D& pos)
       : m_pos(pos),
         m_anchored(false),
         m_locked(false),
         m_selected(0.0f)
      {}

      /// \brief Constructor
      Node(const Coord3D& pos, 
           const bool anchored, 
           const bool locked, 
           const float& selected) 
       : m_pos(pos),
         m_anchored(anchored),
         m_locked(locked),
         m_selected(selected)
      {}

      /// \brief Get the position of a node.
      const Coord3D& pos() const
      {
        return m_pos;
      }
      
      /// \brief Get a reference to the position of a node.
      Coord3D& pos()
      {
        return m_pos;
      }
      
      /// \brief Get the value of anchored.
      bool anchored() const
      {
        return m_anchored;
      }
      
      /// \brief Get a reference to anchored.
      void set_anchored(bool b)
      {
        m_anchored=b;
      }
      
      /// \brief Get the value of locked.
      bool locked() const
      {
        return m_locked;
      }
      
      /// \brief Get a reference to the value of locked.
      void set_locked(bool b)
      {
        m_locked=b;
      }
      
      /// \brief Get whether this node is selected.
      const float& selected() const
      {
        return m_selected;
      }
      
      /// \brief Get a reference to whether this node is selected.
      float& selected()
      {
        return m_selected;
      }
  };

  /**
   * @brief A class which contains a Node with additional color information.
   */
  class NodeWithColor : public Node
  {
    protected:
      GLfloat m_color[3];       ///< The (painted) color of the node.

    public:

      /// \brief Default constructor. 
      NodeWithColor()=default;

      /// \brief Constructor
      NodeWithColor(const Coord3D& pos)
       : Node(pos)
      {
        m_color[0]=0.0f;
        m_color[1]=0.0f;
        m_color[2]=0.0f;
      }

      /// \brief Constructor
      NodeWithColor(
           const Coord3D& pos, 
           const bool anchored, 
           const bool locked, 
           const float& selected, 
           const GLfloat& color0,
           const GLfloat& color1,
           const GLfloat& color2)
       : Node(pos, anchored, locked, selected)
      {
        m_color[0]=color0;
        m_color[1]=color1;
        m_color[2]=color2;
      }

      /// \brief Get the color.
      GLfloat* color() 
      {
        return m_color;
      }

      /// \brief Get the color.
      const GLfloat& color(size_t i) const
      {
        return m_color[i];
      }

      /// \brief Get a reference to the color.
      GLfloat& color(size_t i) 
      {
        return m_color[i];
      }

  };

  

  /**
   * @brief A structure which contains the information of a single label (with tau indicator).
   */
  struct LabelString
  {
    protected:
      bool m_isTau;             ///< Indicates that the label is tau.
      QString m_label;          ///< The string representation of the label.

    public:
      /// \brief Default constructor.
      LabelString() = default;

      /// \brief Constructor. If isTau is true, set the label to the the greek symbol tau.
      LabelString(bool isTau, const QString& label)
       : m_isTau(isTau), m_label(isTau?QChar(0x03C4):label)
      {}

      /// \brief Get whether this label is equal to tau.
      bool is_tau() const
      {
        return m_isTau;
      }
      
      /// \brief Get the label in this string.
      const QString& label() const
      {
        return m_label;
      }
  };


  /**
   * @brief A structure which contains the information of a single edge.
   */
  class LabelNode : public NodeWithColor
  {
    protected:
      size_t m_labelindex;      ///< The index of the label (string).

    public:
      /// \default constructor
      LabelNode() = default;

      /// \brief Constructor
      LabelNode(const Coord3D& p, const size_t labelindex)
       : NodeWithColor(p), m_labelindex(labelindex)
      {}

      /// \brief Constructor
      LabelNode(const Coord3D& pos, 
                bool anchored, 
                bool locked, 
                const float& selected,
                const GLfloat& color0,
                const GLfloat& color1,
                const GLfloat& color2,
                const size_t labelindex)
       : NodeWithColor(pos,anchored,locked,selected,color0,color1,color2), m_labelindex(labelindex)
      {
      }

      /// \brief Get the value of labelindex.
      size_t labelindex() const
      { 
        return m_labelindex;
      }

      /// \brief Get a reference to the value of labelindex.
      size_t& labelindex()
      { 
        return m_labelindex;
      }
  };


  /**
   * @brief A structure which contains the information of a single graph node.
   */
  class NodeNode : public NodeWithColor
  {
    protected:
      bool m_is_probabilistic;  ///< Indicates that this is a probabilistic state.

    public:
      /// \brief Default constructor.
      NodeNode()=default;

      /// \brief Constructor.
      NodeNode(const Coord3D& p)
        : NodeWithColor(p)
      {}

      /// \brief Constructor
      NodeNode(const Coord3D& pos, 
               bool anchored, 
               bool locked, 
               const float& selected,
               const GLfloat& color0,
               const GLfloat& color1,
               const GLfloat& color2,
               bool is_probabilistic)
       : NodeWithColor(pos,anchored,locked,selected,color0,color1,color2), m_is_probabilistic(is_probabilistic)
      {
      }

      /// \brief Get whether the node is probabilistic.
      bool is_probabilistic() const
      {
        return m_is_probabilistic;
      }

      /// \brief Get a reference to the boolean indicating that this NodeNode is probabilistic.
      bool& is_probabilistic()
      {
        return m_is_probabilistic;
      }
  };

  namespace detail
  {
    class GraphImplBase;
  }

  /**
  @brief: This is the internal data structure that LTSGraph operates on.

    In its implementation it uses the mCRL2 lts classes to represent the graphs,
    and augments it with further information. In particular, positions of labels
    and edge handles are stored as if they were nodes.
*/
  class Graph{
    private:
      detail::GraphImplBase* m_impl;  ///< The internal implementation of the graph used.
      mcrl2::lts::lts_type m_type;    ///< The type of the current graph.
      QString m_empty;                ///< Empty string that is returned as label if none present.


      /**
       * @brief Initialises a graph implementation of the desired type.
       */
      void createImpl(mcrl2::lts::lts_type itype);
    public:

      /**
       * @brief Constructor which initialises a empty graph.
       */
      Graph();

      /**
       * @brief Destructor.
       */
      ~Graph();


      /**
       * @brief Loads a graph with random positioning for the nodes.
       * @param filename The file which contains the graph.
       * @param min The minimum coordinates for any node.
       * @param max The maximum coordinates for any node.
       */
      void load(const QString& filename, const Coord3D& min, const Coord3D& max);

      /**
       * @brief Loads a graph with from a XML file exported using @fn saveXML.
       * @param filename The file which contains the graph.
       */
      void loadXML(const QString& filename);

      /**
       * @brief Saves the current graph with all neccesary information.
       * @param filename The file to which the graph is saved.
       */
      void saveXML(const QString& filename);

      /**
       * @brief Returns the string representation of the transition label with index @e labelindex.
       *        If the index is not valid, an empty string is returned.
       * @param labelindex The index of the label.
       */
      const QString& transitionLabelstring(size_t labelindex) const;

      /**
       * @brief Returns the string representation of the state label with index @e labelindex.
       *        If the index is not valid, an empty string is returned.
       * @param labelindex The index of the label.
       */
      const QString& stateLabelstring(size_t labelindex) const;

      /**
       * @brief Restrains all nodes of the graph between @e min and @e max.
       * @param min The minimum coordinates for any node.
       * @param max The maximum coordinates for any node.
       */
      void clip(const Coord3D& min, const Coord3D& max);

      // Getters and setters
      Edge edge(size_t index) const;
      NodeNode& node(size_t index) const;
      Node& handle(size_t edge) const;
      LabelNode& transitionLabel(size_t edge) const;
      LabelNode& stateLabel(size_t edge) const;
      bool isTau(size_t labelindex) const;

      size_t edgeCount() const;
      size_t nodeCount() const;
      size_t transitionLabelCount() const;
      size_t stateLabelCount() const;
      size_t initialState() const;
  };

}

#endif // GRAPH_H
