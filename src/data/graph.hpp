#pragma once

namespace xen {
template <typename NodeT>
class Graph;

/// GraphNode class, representing a base node in a Graph. Must be inherited to be used by a Graph.
/// \tparam T Graph node's specific type. Must be the derived class itself.
template <typename T>
class GraphNode {
    friend Graph<T>;

public:
    GraphNode(GraphNode const&) = delete;
    GraphNode(GraphNode&&) noexcept = default;

    GraphNode& operator=(GraphNode const&) = delete;
    GraphNode& operator=(GraphNode&&) noexcept = default;

    virtual ~GraphNode() = default;

    std::vector<T*> const& get_parents() const { return parents; }

    size_t get_parent_count() const { return parents.size(); }

    T const& get_parent(size_t index) const;

    T& get_parent(size_t index);

    std::vector<T*> const& get_children() const { return children; }

    size_t get_child_count() const { return children.size(); }

    T const& get_child(size_t index) const;

    T& get_child(size_t index);

    /// Checks if the current node is a root, that is, a node without any parent.
    /// \return True if it is a root node, false otherwise.
    bool is_root() const { return parents.empty(); }

    /// Checks if the current node is a leaf, that is, a node without any child.
    /// \return True if it is a leaf node, false otherwise.
    bool is_leaf() const { return children.empty(); }

    /// Checks if the current node is isolated, that is, a node which is both a root & a leaf (without any parent or
    /// child).
    /// \return True if it is an isolated node, false otherwise.
    bool is_isolated() const { return is_root() && is_leaf(); }

    /// Links the given nodes as parents of the current one.
    /// \tparam OtherNodesTs Types of the other nodes to link.
    /// \param node First node to be linked.
    /// \param other_nodes Other nodes to be linked.
    template <typename... OtherNodesTs>
    void add_parents(GraphNode& node, OtherNodesTs&&... other_nodes);

    /// Unlinks the given nodes as parents of the current one; this also removes the current one from the nodes'
    /// children.
    /// \tparam OtherNodesTs Types of the other nodes to unlink.
    /// \param node First node to be unlinked.
    /// \param other_nodes Other nodes to be unlinked.
    template <typename... OtherNodesTs>
    void remove_parents(GraphNode& node, OtherNodesTs&&... other_nodes);

    /// Links the given nodes as children of the current one.
    /// \tparam OtherNodesTs Types of the other nodes to link.
    /// \param node First node to be linked.
    /// \param other_nodes Other nodes to be linked.
    template <typename... OtherNodesTs>
    void add_children(GraphNode& node, OtherNodesTs&&... other_nodes);

    /// Unlinks the given nodes as children of the current one; this also removes the current one from the nodes'
    /// parents.
    /// \tparam OtherNodesTs Types of the other nodes to unlink.
    /// \param node First node to be unlinked.
    /// \param other_nodes Other nodes to be unlinked.
    template <typename... OtherNodesTs>
    void remove_children(GraphNode& node, OtherNodesTs&&... other_nodes);

protected:
    std::vector<T*> parents{};
    std::vector<T*> children{};

protected:
    GraphNode() = default;

    /// Unlinks the given node only as a parent of the current one; the given node's children are left untouched.
    /// \param node Node to be unlinked.
    void unlink_parent(GraphNode const& node);

    /// Unlinks the given node only as a child of the current one; the given node's parents are left untouched.
    /// \param node Node to be unlinked.
    void unlink_child(GraphNode const& node);
};

/// Graph class, representing a [directed graph](https://en.wikipedia.org/wiki/Directed_graph).
/// This class is implemented as an [adjacency list](https://en.wikipedia.org/wiki/Adjacency_list).
/// \tparam NodeT Type of the graph's nodes. Must be a type derived from GraphNode.
template <typename NodeT>
class Graph {
    static_assert(
        std::is_base_of_v<GraphNode<NodeT>, NodeT>, "Error: The graph node type must be derived from GraphNode."
    );

    using NodePtr = std::unique_ptr<NodeT>;

public:
    /// Default constructor.
    Graph() = default;

    /// Creates a Graph while preallocating the given amount of nodes.
    /// \param node_count Amount of nodes to reserve.
    explicit Graph(size_t node_count) { nodes.reserve(node_count); }

    Graph(Graph const&) = delete;
    Graph(Graph&&) noexcept = default;
    Graph& operator=(Graph const&) = delete;
    Graph& operator=(Graph&&) noexcept = default;

    size_t get_node_count() const { return nodes.size(); }

    NodeT const& get_node(size_t index) const;

    NodeT& get_node(size_t index);

    /// Adds a node into the graph.
    /// \tparam Args Types of the arguments to be forwarded to the node's constructor.
    /// \param args Arguments to be forwarded to the node's constructor.
    /// \return Reference to the newly added node.
    template <typename... Args>
    NodeT& add_node(Args&&... args);

    /// Removes a node from the graph, after unlinking it from all its parents & children.
    /// \param node Node to be removed.
    void remove_node(NodeT& node);

protected:
    std::vector<NodePtr> nodes{};
};
}

#include "graph.inl"