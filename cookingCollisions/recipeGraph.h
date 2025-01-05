#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <iostream>

class RecipeGraph
{
public:
	struct Edge {
		std::string destination;
		std::string input;
	};

	static RecipeGraph& GetInstance();

	void AddNode(const std::string& node) { graph[node] = {}; }
	void AddEdge(const std::string& from, const std::string& to, const std::string& input = "none") { graph[from].push_back({ to, input }); }
	const std::vector<Edge>& GetNeighbors(const std::string& node) const;
	void PrintGraph() const;
	
	std::string FindCommonNode(const std::string& node1, const std::string& node2) const;
	std::string GetInputForEdge(const std::string& from, const std::string& to) const;
	std::string ApplyInputToNode(const std::string& from, const std::string& input) const;

private:
	RecipeGraph() = default;

	std::unordered_map<std::string, std::vector<Edge>> graph;
};