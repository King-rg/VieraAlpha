#include <iostream>
#include <vector>
#include <sstream>
#include <random>
#include <climits>
#include <algorithm>
#include <functional>
#include <string>
#include <Windows.h>
#include <ctime>

#include "lib.h"
#include "encoder.h"

//CONSTRUCTORS

using namespace std;

struct util
{
	string gen_uuid()
	{
		int length = 20;
		static const char alphanum[] =
			"0123456789"
			"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
			"abcdefghijklmnopqrstuvwxyz";
		std::string tmp_s;
		tmp_s.reserve(length);

		for (int i = 0; i < length; ++i) {
			tmp_s += alphanum[rand() % (sizeof(alphanum) - 1)];
		}

		return tmp_s;

	}
};

// CONSTRUCTORS

procunit::procunit(SDR input)
{
	util utilities;
	id = utilities.gen_uuid();

	for (int x = 0; x < childamount; x++)
	{
		proccol n(input);
		columns.push_back(n);
	}
}

proccol::proccol(SDR input)
{
	util utilities;
	id = utilities.gen_uuid();

	for (int x = 0; x < childamount; x++)
	{
		procnode n(input);
		nodes.push_back(n);
	}

	for (int x = 0; x < connamount; x++)
	{
		conn n("proximal", input);
		proxconns.push_back(n);
	}
}

procnode::procnode(SDR input)
{
	util utilities;
	id = utilities.gen_uuid();
}

conn::conn(string type, SDR input)
{
	util utilities;
	id = utilities.gen_uuid();

	if (type.compare("proximal") == 0)
	{
		coord.push_back(rand() % int(input.resolution));
		coord.push_back(rand() % int(input.resolution));
	}

	strength = 1 + (rand() % 100);
}

vector<bool> proccol::process_proximal(SDR input)
{

	int activationAmount = 0;
	bool predFlag = false;
	vector<bool> mapCol;

	//Calculate activationAmount
	for (int x = 0; x < size(proxconns); x++)
	{	
		if (input.content[proxconns[x].coord[0]][proxconns[x].coord[1]] == 1)
		{
			proxconns[x].strength = proxconns[x].strength + 30 - (sqrt(10 * proxconns[x].strength));
			activationAmount++;
		}
		else
		{
			proxconns[x].strength = proxconns[x].strength - (sqrt(10 * proxconns[x].strength));
		}
	}

	//Prune all low strength connections
	for (int x = 0; x < size(proxconns); x++)
	{
		if (proxconns[x].strength <= conf.proxTerminationThreshold)
		{
			proxconns.erase(proxconns.begin()+x);
			// To prevent vector errors, the loop needs to repeat until there are no more low strength connections
			x = 0;
		}
	}


	//Calculate which nodes need to be activated
	if (activationAmount >= conf.proxActivationThreshold)
	{
		//Check if any nodes are in a predictive state
		for (int y = 0; y < size(nodes); y++)
		{
			// If there are any, activate only those nodes
			if (nodes[y].predictive == true)
			{
				predFlag = true;
			}
		}

		if (predFlag == true)
		{
			for (int y = 0; y < size(nodes); y++)
			{
				if (nodes[y].predictive == true)
				{
					nodes[y].active = true;
					mapCol.push_back(true);
				}
				else
				{
					nodes[y].active = false;
					mapCol.push_back(false);
				}
			}
		}

		// If there are none, activate all nodes.
		if (predFlag == false)
		{
			for (int y = 0; y < size(nodes); y++)
			{
				nodes[y].active = true;
				mapCol.push_back(true);
			}
		}
	}
	else
	{
		for (int y = 0; y < size(nodes); y++)
		{
			mapCol.push_back(false);
		}
	}


	return mapCol;
}

void proccol::process_distal(vector<vector<bool>> map, SDR input)
{

	//Adjust strengths of previous predictive states.
	for (int x = 0; x < size(nodes); x++)
	{
		if (nodes[x].active == true && nodes[x].predictive == true)
		{
			nodes[x].predictive = false;

			for (int y = 0; x < size(nodes[y].distconns); y++)
			{
				nodes[x].distconns[y].strength = nodes[x].distconns[y].strength + 30 - (sqrt(10 * nodes[x].distconns[y].strength));
			}
		}
		else if (nodes[x].active == false && nodes[x].predictive == true)
		{
			nodes[x].predictive = false;

			for (int y = 0; x < size(nodes[y].distconns); y++)
			{
				nodes[x].distconns[y].strength = nodes[x].distconns[y].strength - (sqrt(10 * nodes[x].distconns[y].strength));
			}
		}
	}

	//Generate new connections
	bool predFlag = false;
	for (int x = 0; x < size(nodes); x++)
	{
		if (nodes[x].predictive == true)
		{
			predFlag = true;
		}
	}

	//If no nodes are predictive
	if (predFlag == false)
	{
		int smallest = 0;
		for (int x = 0; x < size(nodes); x++)
		{
			if (size(nodes[x].distconns) < size(nodes[smallest].distconns))
			{
				smallest = x;
			}
		}
		procnode& selectedNode = nodes[smallest];
		
		for (int x = 0; x < size(map); x++)
		{
			for (int y = 0; y < size(map[x]); y++)
			{
				if (map[x][y] == true)
				{
					conn n("distal", input);
					selectedNode.distconns.push_back(n);
					selectedNode.distconns[size(selectedNode.distconns) - 1].coord.push_back(x);
					selectedNode.distconns[size(selectedNode.distconns) - 1].coord.push_back(y);
				}
			}
		}
	}			

	//Calculate new predictive states
	for (int x = 0; x < size(nodes); x++)
	{
		int activationAmount = 0;
		for (int y = 0; y < size(nodes[x].distconns); y++)
		{
			if (map[nodes[x].distconns[y].coord[0]][nodes[x].distconns[y].coord[1]] == true)
			{
				activationAmount++;
			}
		}

		if (activationAmount > conf.distalActivationThreshold)
		{
			nodes[x].predictive = true;
		}
	}

}

void procunit::cycle(SDR input)
{
	//Run proximal processing cycle on every column
	vector<vector<bool>> map;
	for (int x = 0; x < size(columns); x++)
	{
		map.push_back(columns[x].process_proximal(input));
	}

	for (int x = 0; x < size(columns); x++)
	{
		columns[x].process_distal(map, input);
	}

	//Print activation map

	for (int x = 0; x < size(map); x++)
	{
		for (int y = 0; y < size(map[x]); y++)
		{
			if (map[x][y] == true)
			{
				cout << "1 ";
			}
			else
			{
				cout << "0 ";
			}
		}
		cout << endl;
	}

	cout << "========" << endl;
}
