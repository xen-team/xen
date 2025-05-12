namespace xen {
template <typename T>
T const& GraphNode<T>::get_parent(size_t index) const
{
    Log::rt_assert(index < parents.size(), "Error: The requested parent node is out of bounds.");
    return *parents[index];
}

template <typename T>
T& GraphNode<T>::get_parent(size_t index)
{
    Log::rt_assert(index < parents.size(), "Error: The requested parent node is out of bounds.");
    return *parents[index];
}

template <typename T>
T const& GraphNode<T>::get_child(size_t index) const
{
    Log::rt_assert(index < children.size(), "Error: The requested child node is out of bounds.");
    return *children[index];
}

template <typename T>
T& GraphNode<T>::get_child(size_t index)
{
    Log::rt_assert(index < children.size(), "Error: The requested child node is out of bounds.");
    return *children[index];
}

template <typename T>
template <typename... OtherNodesTs>
void GraphNode<T>::add_parents(GraphNode& node, OtherNodesTs&&... other_nodes)
{
    if (&node == this) {
        throw std::invalid_argument("Error: A graph node cannot be a parent of itself");
    }

    if (std::find(parents.cbegin(), parents.cend(), &node) == parents.cend()) {
        parents.emplace_back(static_cast<T*>(&node));
    }

    if (std::find(node.children.cbegin(), node.children.cend(), this) == node.children.cend()) {
        node.children.emplace_back(static_cast<T*>(this));
    }

    // Stop the recursive unpacking if no more nodes are to be added as parents
    if constexpr (sizeof...(other_nodes) > 0) {
        add_parents(std::forward<OtherNodesTs>(other_nodes)...);
    }
}

template <typename T>
template <typename... OtherNodesTs>
void GraphNode<T>::remove_parents(GraphNode& node, OtherNodesTs&&... other_nodes)
{
    unlink_parent(node);
    node.unlink_child(*this);

    // Stop the recursive unpacking if no more nodes are to be removed as parents
    if constexpr (sizeof...(other_nodes) > 0) {
        remove_parents(std::forward<OtherNodesTs>(other_nodes)...);
    }
}

template <typename T>
template <typename... OtherNodesTs>
void GraphNode<T>::add_children(GraphNode& node, OtherNodesTs&&... other_nodes)
{
    if (&node == this) {
        throw std::invalid_argument("Error: A graph node cannot be a child of itself");
    }

    if (std::find(children.cbegin(), children.cend(), &node) == children.cend()) {
        children.emplace_back(static_cast<T*>(&node));
    }

    if (std::find(node.parents.cbegin(), node.parents.cend(), this) == node.parents.cend()) {
        node.parents.emplace_back(static_cast<T*>(this));
    }

    // Stop the recursive unpacking if no more nodes are to be added as children
    if constexpr (sizeof...(other_nodes) > 0) {
        add_children(std::forward<OtherNodesTs>(other_nodes)...);
    }
}

template <typename T>
template <typename... OtherNodesTs>
void GraphNode<T>::remove_children(GraphNode& node, OtherNodesTs&&... other_nodes)
{
    unlink_child(node);
    node.unlink_parent(*this);

    // Stop the recursive unpacking if no more nodes are to be removed as children
    if constexpr (sizeof...(other_nodes) > 0) {
        remove_children(std::forward<OtherNodesTs>(other_nodes)...);
    }
}

template <typename T>
void GraphNode<T>::unlink_parent(GraphNode const& node)
{
    if (&node == this) {
        throw std::invalid_argument("Error: A graph node cannot be unlinked from itself");
    }

    auto const parent_it = std::find(parents.cbegin(), parents.cend(), &node);
    if (parent_it != parents.cend()) {
        parents.erase(parent_it);
    }
}

template <typename T>
void GraphNode<T>::unlink_child(GraphNode const& node)
{
    if (&node == this) {
        throw std::invalid_argument("Error: A graph node cannot be unlinked from itself");
    }

    auto const child_it = std::find(children.cbegin(), children.cend(), &node);
    if (child_it != children.cend()) {
        children.erase(child_it);
    }
}

template <typename NodeT>
NodeT const& Graph<NodeT>::get_node(size_t index) const
{
    Log::rt_assert(index < nodes.size(), "Error: The requested node is out of bounds.");
    return *nodes[index];
}

template <typename NodeT>
NodeT& Graph<NodeT>::get_node(size_t index)
{
    Log::rt_assert(index < nodes.size(), "Error: The requested node is out of bounds.");
    return *nodes[index];
}

template <typename NodeT>
template <typename... Args>
NodeT& Graph<NodeT>::add_node(Args&&... args)
{
    nodes.emplace_back(std::make_unique<NodeT>(std::forward<Args>(args)...));
    return *nodes.back();
}

template <typename NodeT>
void Graph<NodeT>::remove_node(NodeT& node)
{
    auto const node_it = std::find_if(nodes.cbegin(), nodes.cend(), [&node](NodePtr const& nodePtr) {
        return (nodePtr.get() == &node);
    });

    if (node_it == nodes.cend()) {
        throw std::invalid_argument("Error: The graph node to be removed does not exist");
    }

    for (NodeT* parent : node.parents) {
        parent->unlink_child(node);
    }

    for (NodeT* child : node.children) {
        child->unlink_parent(node);
    }

    nodes.erase(node_it);
}
}