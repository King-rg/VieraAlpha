#include <iostream>
#include <cmath>
#include <vector>

#include "encoder.h"

using namespace std;

float get_largest(float input[], float resolution, float scaling)
{
	float largest_element = 0;
	for (int i = 0; i < resolution; i++)
	{
		float element = input[i] * scaling;

		if (element > largest_element)
		{
			largest_element = element;
		}

	}
	return largest_element;
}

float get_smallest(float input[], float resolution, float scaling)
{
	float smallest_element = 0;
	for (int i = 0; i < resolution; i++)
	{
		if (i == 0)
		{
			smallest_element = input[i] * scaling;
		}

		float element = input[i] * scaling;

		if (element < smallest_element)
		{
			smallest_element = element;
		}

	}
	return smallest_element;
}

SDR encoder::TimeSeries(float data[50])
{

	float mapper = (get_largest(data, resolution, scaling) - get_smallest(data, resolution, scaling)) / resolution;
	vector<int> locations;
	vector<vector<int>> column_map;
	vector<vector<int>> row_map;

	for (int i = 0; i < resolution; i++)
	{
		int location = 0;
		for (float b = get_smallest(data, resolution, scaling); b < data[i] * scaling; b = b + mapper)
		{
			location++;
		}

		locations.push_back(location);
	}

	//Create a column map

	for (int x = 0; x < resolution; x++)
	{
		vector<int> column;
		for (int y = 0; y < resolution; y++)
		{
			if (y == locations[x])
			{
				column.push_back(1);
			}
			else
			{
				column.push_back(0);
			}
		}

		column_map.push_back(column);
	}

	//Convert to row map

	for (int x = 0; x < resolution; x++)
	{
		vector<int> row;
		for (int y = 0; y < resolution; y++)
		{
			row.push_back(column_map[y][x]);
		}

		/*
		for (int i : row)
		{
			cout << i;
		}
		cout << endl;
		*/

		row_map.push_back(row);
	}

	SDR ret;
	ret.resolution = resolution;
	ret.scaling = scaling;
	ret.content = row_map;

	return ret;
}
