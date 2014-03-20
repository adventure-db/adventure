#include "graph.h"

#include <stdio.h>

/*
void generateGraph()
{
	graph_t g;
	g.n = 3;
	g.nodes = (node_t *) malloc( sizeof(node_t)*g.n );

	for(int i = 0; i < g.n; i++) {
		g.nodes[i].id = i;
		g.nodes[i].edges = (edge_t *) malloc( sizeof(edge_t)*1 );
		g.nodes[i].edges[0].id = 0;
		g.nodes[i].edges[0].start = &g.nodes[i];
		g.nodes[i].edges[0].end = i == g.n? &g.nodes[0] : &g.nodes[i+1];
		g.nodes[i].e = 1;
	}

	debugPrint(g);

	// Just say no to memory leakss
	for(int i = 0; i < g.n; i++) {
		free(g.nodes[i].edges);
	}
	free(g.nodes);
}

void debugPrint(graph_t g)
{
	printf("Nodes: \t");
	for(int i = 0; i < g.n; i++) {
		printf("%lu ", g.nodes[i].id);
	}
	printf("\n");

	printf("Edges: \n");
	for(int i = 0; i < g.n; i++) {
		edge_t *e = g.nodes[i].edges;
		printf("%lu:\t", e->start->id);
		for(int j = 0; j < g.nodes[i].e; j++) {
			printf("(%lu, %lu) ", e[j].start->id, e[j].end->id);
		}
		printf("\n");
	}
	printf("\n");
}
*/