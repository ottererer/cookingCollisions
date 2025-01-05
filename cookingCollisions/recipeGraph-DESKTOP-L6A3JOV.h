#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <iostream>

class RecipeGraph
{
public:
	void AddNode(const std::string& node) { graph[node] = {}; }
	void AddEdge(const std::string& from, const std::string& to) { graph[from].push_back(to); }
	const std::vector<std::string>& GetNeighbors(const std::string& node) const { return graph.at(node); }
	void PrintGraph() const;
	
	std::string FindCommonNode(const std::string& node1, const std::string& node2) const;

private:
	std::unordered_map<std::string, std::vector<std::string>> graph;
};