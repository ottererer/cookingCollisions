#include "recipeGraph.h"

RecipeGraph& RecipeGraph::GetInstance()
{
	static RecipeGraph instance;
	return instance;
}

const std::vector<RecipeGraph::Edge>& RecipeGraph::GetNeighbors(const std::string& node) const
{
	auto it = graph.find(node);
	if (it != graph.end()) return it->second;
	else {
		static const std::vector<Edge> emptyVector;
		return emptyVector;
	}
}

void RecipeGraph::PrintGraph() const {
	for (const auto& pair : graph) {
		std::cout << pair.first << " -> ";
		for (const auto& neighbor : pair.second) {
			std::cout << neighbor.destination << " ";
		}
		std::cout << std::endl;
	}
}

std::string RecipeGraph::FindCommonNode(const std::string& node1, const std::string& node2) const
{
	const auto& neighbors1 = GetNeighbors(node1);
	const auto& neighbors2 = GetNeighbors(node2);

	std::unordered_set<std::string> set1;

	for (const auto& neighbor : neighbors1) {
		set1.insert(neighbor.destination);
	}

	std::string commonNode = "NULL";
	for (const auto& neighbor : neighbors2) {
		if (set1.count(neighbor.destination)) commonNode = neighbor.destination;
	}

	return commonNode;
}

std::string RecipeGraph::GetInputForEdge(const std::string& from, const std::string& to) const
{
	const auto& neighbors = GetNeighbors(from);
	for (const auto& neighbor : neighbors) {
		if (neighbor.destination == to) {
			return neighbor.input;
		}
	}
	return "NULL";
}

std::string RecipeGraph::ApplyInputToNode(const std::string& from, const std::string& input) const
{
	const auto& neighbors = GetNeighbors(from);

	for (const auto& neighbor : neighbors) {
		if (neighbor.input == input) {
			return neighbor.destination;
		}
	}

	return "NULL";
}