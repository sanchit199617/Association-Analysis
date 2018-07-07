// FPGrowth.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <iostream>
#include <vector>
#include <fstream>
#include <string>
#include <algorithm>

using namespace std;

int node_identity = 0;

struct Node
{
	int number;
	int item;
	int count;
	struct Node *parent;
	vector<struct Node *> next;
};

struct Item
{
	int identity;
	int support;
};

struct lNode
{
	Node *data;
	struct lNode *next;
};

void resize(vector<vector<float>>& data, ifstream& datafile);
void preprocess(vector<vector<float>>&data, ifstream &datafile, string datatxt);
int find_pos(int c, float t);
float maxOf(int c);
float minOf(int c);
int nparts(int c);
void printData(vector<vector<float>>&data);
int totalItems();
bool sort_by_support(Item &lhs, Item &rhs);

void printtree(Node *root);
Node * newNode(int i);
Node * formFPTree(vector<vector<int>> newdata, vector<Item>& items, vector<lNode *>& heads);
void formSubTree(Node * node, vector<int>& transaction, int j, vector<Item>& items, vector<lNode *>& heads);
void formTree(Node * node, vector<int>& transaction, int j, vector<Item>& items, vector<lNode *>& heads);
lNode * newlNode(Node * d);
void printLinkedList(lNode *node);
Node * formPrefixTree(vector<Item>& items, vector<lNode *>& heads, vector<lNode *>& item_heads, int item);
Node * searchTree(Node * proot, int k);
int updatesupport(Node *root);
void setsupport(Node *root);
void updateSupports(Node *root);
void updateLinkedList(vector<lNode *>& heads, int identity, int index);
Node * formCondFPTree(Node *proot, vector<Item>& items, vector<lNode *>& heads, int item, int minsup);
void countOfLeafNodes(Node *root, int *count);
int searchItems(vector<Item>& items, int item);
void FPGrowth(vector<Item>& items, vector<lNode *>& heads, vector<vector<int>>& freqSets, int minsup);
void checkFrequency(vector<Item>& items, vector<lNode *>& heads, vector<lNode *>& item_heads, vector<vector<int>>& freqSets, vector<int>& item_freqSet, int minsup, int item);
int itemExistsInLinkedList(vector<lNode *>& heads, int item);

int main()
{
	string datatxt = "data1.txt";
	ifstream datafile(datatxt);
	vector<vector<float>> data;
	
	//data preprocessing - discretizing and binarizing
	preprocess(data, datafile, datatxt);
	int minsup = 0.4*data.size(), minconf;
	int n = totalItems();
	vector<Item> items(n);
	for (int i = 0; i < n; i++)
	{
		items[i].identity = i;
	}
	for (int i = 0; i < data.size(); i++)
	{
		for (int j = 0; j < n; j++)
		{
			items[j].support += data[i][j];
		}
	}
	for (int i = 0; i < data.size(); i++)
	{
		for (int j = 0; j < n; j++)
		{
			if (items[j].support <= minsup)
			{
				data[i][j] = 0;
			}
		}
	}
	sort(items.begin(), items.end(), sort_by_support);
	//pop infrequent itemsets from items vector
	for (int i = items.size() - 1; i >= 0; i--)
	{
		if (items[i].support <= minsup)
		{
			items.pop_back();
		}
	}
	//form newdata with items in sorted order
	vector<vector<int>> newdata(data.size());
	for (int i = 0; i < newdata.size(); i++)
	{
		for (int j = 0; j < items.size(); j++)
		{
			if (data[i][items[j].identity] == 1)
			{
				newdata[i].push_back(items[j].identity);
			}
		}
	}
	for (int i = 0; i < newdata.size(); i++)
	{
		for (int j = 0; j < newdata[i].size(); j++)
		{
			cout << newdata[i][j] << ", ";
		}
		cout << "\n";
	}
	vector<lNode *>heads(items.size());
	Node *root = formFPTree(newdata, items, heads);
	cout << "\n\n";
	printtree(root);
	cout << "\n\n";
	for (int i = 0; i < heads.size(); i++)
	{
		printLinkedList(heads[i]);
		cout << "\n";
	}
	cout << "\n";

	vector<lNode *>item_heads(items.size());
	Node *proot = formPrefixTree(items, heads,item_heads, 5);
	printtree(proot);
	cout << "\n\n";
	for (int i = 0; i < item_heads.size(); i++)
	{
		printLinkedList(item_heads[i]);
		cout << "\n";
	}
	vector<vector<int>> freqSets;
	FPGrowth(items, heads, freqSets, minsup);
	for (int i = 0; i < freqSets.size(); i++)
	{
		for (int j = 0; j < freqSets[i].size(); j++)
		{
			cout << freqSets[i][j] << ", ";
		}
		cout << "\n";
	}
	return 0;
}

void removeLeafs(Node *root, vector<Item>& items, vector<lNode *>& heads, int it)
{
	if (root->next.size() == 0 && root->item == it)
	{
		Node *temp;
		for (int j = 0; j < root->parent->next.size(); j++)
		{
			if (root->parent->next[j]->item == it)
			{
				temp = root->parent->next[j];
			}
		}
		root->parent->next.erase(remove(root->parent->next.begin(), root->parent->next.end(), temp), root->parent->next.end());
		int index = searchItems(items, temp->item);
		updateLinkedList(heads, temp->number, index);
	}
	for (int i = 0; i < root->next.size(); i++)
	{
		removeLeafs(root->next[i],items, heads, it);
	}
}

void removeInfrequentNodes(Node *root, vector<Item>& items,  vector<lNode *>& heads, int minsup)
{
	if (root->count < minsup)
	{
		Node *p = root->parent;
		Node *temp;
		for (int j = 0; j < p->next.size(); j++)
		{
			if (p->next[j]->item == root->item)
			{
				temp = p->next[j];
			}
		}
		p->next.erase(remove(p->next.begin(), p->next.end(), temp), p->next.end());
		int index = searchItems(items, temp->item);
		updateLinkedList(heads, temp->number, index);
		for (int j = 0; j < root->next.size(); j++)
		{
			p->next.push_back(root->next[j]);
			root->next[j]->parent = p;
		}
		root = p;
	}
	for (int i = 0; i < root->next.size(); i++)
	{
		removeInfrequentNodes(root->next[i], items, heads, minsup);
	}
}

int count(Node *root)
{
	int count = 0;
	countOfLeafNodes(root, &count);
	return count;
}

void countOfLeafNodes(Node *root, int *count)
{
	if (root->next.size() == 0)
	{
		*count+=root->count;
	}
	for (int i = 0; i < root->next.size(); i++)
	{
		countOfLeafNodes(root->next[i], count);
	}
}


void FPGrowth(vector<Item>& items, vector<lNode *>& heads, vector<vector<int>>& freqSets, int minsup)
{
	for (int i = items.size() - 1; i >= 0; i--)
	{
		vector<lNode *>item_heads(items.size());
		Node *proot = formPrefixTree(items, heads, item_heads, items[i].identity);
		printtree(proot);
		cout << "\nPrefix LList\n";
		for (int i = 0; i < item_heads.size(); i++)
		{
			printLinkedList(item_heads[i]);
			cout << "\n";
		}
		cout << "\n"<< items[i].identity << " : \n COnd Tree\n";
		vector<int> item_freqSet;
		if (count(proot) > minsup)
		{
			item_freqSet.push_back(items[i].identity);
			/*for (int i = 0; i < item_freqSet.size(); i++)
			{
				cout << item_freqSet[i] << "  ";
			}
			cout << "\n";*/
			freqSets.push_back(item_freqSet);
			Node * croot = formCondFPTree(proot,items, item_heads, items[i].identity, minsup);
			printtree(croot);
			cout << "\n LList \n";
			for (int i = 0; i < item_heads.size(); i++)
			{
				printLinkedList(item_heads[i]);
				cout << "\n";
			}
			cout << "\n";
			for (int j = i - 1; j >= 0; j--)
			{
				cout << items[j].identity << ", ";
				if (itemExistsInLinkedList(item_heads, items[j].identity))
				{
					checkFrequency(items, heads, item_heads, freqSets, item_freqSet, minsup, items[j].identity);
				}
			}
		}
	}
}

int itemExistsInLinkedList(vector<lNode *>& heads, int item)
{
	for (int i = 0; i < heads.size(); i++)
	{
		lNode *curr = heads[i];
		while (curr != NULL)
		{
			if (curr->data->item == item)
			{
				return 1;
			}
			curr = curr->next;
		}
	}
	return 0;
}

//checks recursively for each item set
void checkFrequency(vector<Item>& items, vector<lNode *>& heads, vector<lNode *>& item_heads, vector<vector<int>>& freqSets, vector<int>& item_freqSet, int minsup, int item)
{
	vector<lNode *>new_heads(items.size());
	cout << "inside check frequency  " << item << "\n";
	Node *proot = formPrefixTree(items, item_heads, new_heads, item);
	printtree(proot);
	cout << "\n\n";
	if (count(proot) > minsup)
	{
		item_freqSet.push_back(item);
		freqSets.push_back(item_freqSet);
		Node * croot = formCondFPTree(proot,items, new_heads, item, minsup);
		int index = searchItems(items, item);
		for (int j = index - 1; j >= 0; j--)
		{
			if (itemExistsInLinkedList(new_heads, items[j].identity))
			{
				vector<lNode *> nheads(items.size());
				checkFrequency(items, new_heads, nheads, freqSets, item_freqSet, minsup, items[j].identity);
			}
		}
	}
}


int itemExistsinTree(Node *croot, int item)
{
	if (croot->item == item)
	{
		return 1;
	}
	for (int i = 0; i < croot->next.size(); i++)
	{
		return itemExistsinTree(croot->next[i], item);
	}
}

void reUpdateLinkedList(Node *croot, vector<lNode *>& heads)
{
	for (int i = 0; i < heads.size(); i++)
	{
		lNode *curr = heads[i];
		int c = itemExistsinTree(croot, curr->data->item);
		if (c == 0)
		{
			heads[i] = heads[i]->next;
			curr = heads[i];
		}
		if (curr != NULL)
		{

		}
		lNode *prev = curr;
		while (curr != NULL)
		{
			int check = 0;
			check = itemExistsinTree(croot, curr->data->item);
			if (!check)
			{

			}
			curr = curr->next;
		}
	}
}


void updateLinkedList(vector<lNode *>& heads, int identity, int index)
{
	lNode * curr = heads[index];
	if (curr->data->number == identity)
	{
		heads[index] = heads[index]->next;
	}
	else
	{
		lNode *prev = curr;
		curr = curr->next;
		while (curr != NULL)
		{
			if (curr->data->number == identity)
			{
				prev->next = curr->next;
				break;
			}
			curr = curr->next;
			prev = prev->next;
		}
	}
}

Node * formCondFPTree(Node *proot, vector<Item>& items, vector<lNode *>& heads, int item, int minsup)
{
	Node *croot = new Node();
	croot = proot;
	updateSupports(croot);
	removeLeafs(croot,items, heads, item);
	removeInfrequentNodes(croot, items, heads, minsup);
	//printtree(croot);
	return croot;
}

void updateSupports(Node *root)
{
	setsupport(root);
	updatesupport(root);
}

void setsupport(Node *root)
{
	if (root->next.size() == 0)
	{
		root->count = 1;
	}
	else
	{
		root->count = 0;
		for (int i = 0; i < root->next.size(); i++)
		{
			setsupport(root->next[i]);
		}
	}
}

void printtree(Node *root)
{
	cout << root->number << " : " << root->item << "[" << root->count << "]\n";
	for (int i = 0; i < root->next.size(); i++)
	{
		printtree(root->next[i]);
	}
}

int updatesupport(Node *root)
{
	if (root->next.size() == 0)
	{
		root->count = 1;
		return 1;
	}
	else
	{
		for (int i = 0; i < root->next.size(); i++)
		{
			root->count += updatesupport(root->next[i]);
		}
		return root->count;
	}
}

Node * formFPTree(vector<vector<int>> newdata, vector<Item>& items, vector<lNode *>& heads)
{
	Node *root = newNode(-1);
	node_identity++;
	root->count = 0;
	for (int i = 0; i < newdata.size(); i++)
	{
		formTree(root, newdata[i], 0, items, heads);
	}
	return root;
}

void printLinkedList(lNode *node)
{
	while (node != NULL)
	{
		cout << node->data->item << " [" << node->data->count << "] ";
		node = node->next;
	}
}

//return index at which a particular item is present
int searchItems(vector<Item>& items, int item)
{
	for (int i = 0; i < items.size(); i++)
	{
		if (items[i].identity == item)
		{
			return i;
		}
	}
}

Node * searchTree(Node * proot, int k)
{
	if (proot->number == k)
	{
		return proot;
	}
	else
	{
		for (int i = 0; i < proot->next.size(); i++)
		{
			return searchTree(proot->next[i], k);
		}
	}
}

void formPrefixLinkedList(vector<Item>& items, vector<lNode *>& heads, vector<lNode *>& item_heads, Node *node)
{
	lNode *t = newlNode(node);
	int index = searchItems(items, node->item);
	if (!item_heads[index])
	{
		item_heads[index] = t;
	}
	else
	{
		lNode *curr = item_heads[index];
		while (curr->next != NULL)
		{
			curr = curr->next;
		}
		curr->next = t;
	}
}

Node * formPrefixTree(vector<Item>& items, vector<lNode *>& heads, vector<lNode *>& item_heads, int item)
{
	Node * proot = newNode(-1);
	proot->number = 0;
	int index = searchItems(items, item);
	lNode *curr = heads[index];
	int i = 0;
	while (curr != NULL)
	{
		Node *n = new Node();
		n = curr->data;
		n->next.resize(0);
		formPrefixLinkedList(items, heads, item_heads, n);
		Node *p = n->parent;
		Node *c = n;
		if (i == 0)
		{
			while (p->item != -1)
			{
				Node *t = new Node();
				t = p;
				t->next.resize(0);
				t->next.push_back(c);
				formPrefixLinkedList(items, heads, item_heads, t);
				p = p->parent;
				c = t;
			}
			proot->next.push_back(c);
		}
		else
		{
			while (p->parent->item != 1)
			{
				Node *f = NULL;
				f = searchTree(proot, p->number);
				if (!f)
				{
					Node *t = new Node();
					t = p;
					t->next.resize(0);
					t->next.push_back(c);
					formPrefixLinkedList(items, heads, item_heads, t);
					p = p->parent;
					c = t;
				}
				else
				{
					f->next.push_back(c);
					break;
				}
			}
			
		}
		curr = curr->next;
		i++;
	}
	return proot;
}


void formSubTree(Node * node, vector<int>& transaction, int j, vector<Item>& items, vector<lNode *>& heads)
{
	Node *temp = newNode(transaction[j]);
	node_identity++;
	temp->parent = node;
	node->next.push_back(temp);
	lNode *t = newlNode(temp);
	int index = searchItems(items, transaction[j]);
	if (!heads[index])
	{
		heads[index] = t;
	}
	else
	{
		lNode *curr = heads[index];
		while (curr->next != NULL)
		{
			curr = curr->next;
		}
		curr->next = t;
	}
	formTree(node->next[node->next.size() - 1], transaction, j + 1, items, heads);
}

void formTree(Node * node, vector<int>& transaction, int j, vector<Item>& items, vector<lNode *>& heads)
{
	if (j < transaction.size())
	{
		if (node->item == -1)
		{
			int c = 0;
			for (int k = 0; k < node->next.size(); k++)
			{
				if (transaction[j] == node->next[k]->item)
				{
					c = 1;
					formTree(node->next[k], transaction, j, items, heads);
				}
			}
			if (c == 0)
			{
				formSubTree(node, transaction, j, items, heads);
			}
		}
		else if (transaction[j] == node->item)
		{
			node->count++;
			j++;
			if (node->next.size() == 0 && j < transaction.size())
			{
				formSubTree(node, transaction, j, items, heads);
			}
			else if(j < transaction.size())
			{
				int c = 0;
				for (int k = 0; k < node->next.size(); k++)
				{
					if (transaction[j] == node->next[k]->item)
					{
						c = 1;
						formTree(node->next[k], transaction, j, items, heads);
					}
				}
				if (c == 0)
				{
					formSubTree(node, transaction, j, items, heads);
				}
			}
		}
		else
		{
			formSubTree(node, transaction, j, items, heads);
		}
	}
}

lNode * newlNode(Node * d)
{
	lNode *temp = new lNode();
	temp->data = d;
	temp->next = NULL;
	return temp;
}

Node * newNode(int i)
{
	Node *node = new Node();
	node->item = i;
	node->count = 1;
	node->parent = NULL;
	node->number = node_identity;
	node->next.resize(0);
	return node;
}

bool sort_by_support(Item &lhs, Item &rhs)
{
	return lhs.support > rhs.support;
}

int totalItems()
{
	int total_c = 0;
	for (int c = 0; c < 9; c++)
	{
		total_c += nparts(c);
	}
	return total_c;
}

void printData(vector<vector<float>>&data)
{
	int total_items = totalItems();
	
	for (int i = 0; i < data.size(); i++)
	{
		for (int j = 0; j < total_items; j++)
		{
			cout << data[i][j] << ",";
		}
		cout << "\n";
	}
}

void preprocess(vector<vector<float>>&data, ifstream &datafile, string datatxt)
{
	string temp, line;
	resize(data, datafile);
	datafile.open(datatxt);
	int i = 0;
	while (getline(datafile, line))
	{
		for (int j = 0, k = 0, c = 0; j < line.length() && c<9; j++)
		{
			while (line[j] != ',' && j < line.length())
			{
				temp.push_back(line[j]);
				j++;
			}
			float t = stof(temp);
			int p = find_pos(c, t);
			int l;
			for (l = k; l < k + nparts(c); l++)
			{
				if (l == k + p)
				{
					data[i][l] = 1;
				}
				else
				{
					data[i][l] = 0;
				}
			}
			k = l;
			c++;
			temp.clear();
		}
		i++;
	}
	datafile.close();
}

int find_pos(int c, float t)
{
	int n = nparts(c);
	float max = maxOf(c);
	float min = minOf(c);
	float r = (max - min) / n;
	for (int i = 0; i < n; i++)
	{
		float l = min + i*r;
		float u = l + r;
		if (l <= t && t < u)
		{
			return i;
		}
		if (i == n-1 && t == u)
		{
			return i;
		}
	}
}

float maxOf(int c)
{
	switch (c)
	{
	case 0: return 17;
	case 1: return 199;
	case 2: return 122;
	case 3: return 99;
	case 4: return 846;
	case 5: return 67.1;
	case 6: return 2.42;
	case 7: return 81;
	case 8: return 1;
	}
}

float minOf(int c)
{
	switch (c)
	{
	case 0: return 0;
	case 1: return 0;
	case 2: return 0;
	case 3: return 0;
	case 4: return 0;
	case 5: return 0;
	case 6: return 0.078;
	case 7: return 21;
	case 8: return 0;
	}
}

int nparts(int c)
{
	if (c == 8)
		return 2;
	return 2;
}

void resize(vector<vector<float>>& data, ifstream& datafile)
{
	string line;
	int size_data = 0;						//size of data (number of instances)
	if (datafile.is_open())
	{
		while (getline(datafile, line))
		{
			size_data++;
		}
		datafile.close();
	}
	else
	{
		cout << "Unable to open data file!" << "\n";
	}
	data.resize(size_data);
	int total_c = 0;
	for (int c = 0; c < 9; c++)
	{
		total_c += nparts(c);
	}
	for (int i = 0;i < data.size();i++)
	{
		data[i].resize(total_c);
	}
}